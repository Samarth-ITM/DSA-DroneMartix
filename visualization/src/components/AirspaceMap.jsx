import { useEffect, useRef, useState } from 'react';
import { NODES, EDGES } from '../data/simulationStates.js';

// Color theme representing Google Maps (Daylight Logistics style)
const THEME = {
  land: '#f4f3f0',          // Google Maps sand/light gray land
  water: '#aad3df',         // soft water blue
  park: '#d0ebd4',          // soft green grass for parks
  roadMajor: '#ffffff',     // wide white major highways
  roadBorder: '#e4e2de',    // highway grey borders
  roadLocal: '#ffffff',     // thin white local roads
  textDark: '#1e293b',
  textMuted: '#64748b'
};

// Map original nodes to visual customer locations (faded indicators showing map density)
const ADDED_CUSTOMERS = [
  { id: 'C_F', name: 'Customer_Zone_F', x: 280, y: 100 },
  { id: 'C_G', name: 'Customer_Zone_G', x: 580, y: 200 },
  { id: 'C_H', name: 'Customer_Zone_H', x: 450, y: 450 },
  { id: 'C_I', name: 'Customer_Zone_I', x: 150, y: 340 },
  { id: 'C_J', name: 'Customer_Zone_J', x: 580, y: 320 },
];

// Draw storm cloud helper
const drawCloud = (ctx, x, y, scale) => {
  ctx.beginPath();
  ctx.arc(x, y, 15 * scale, 0, Math.PI * 2);
  ctx.arc(x + 12 * scale, y - 4 * scale, 12 * scale, 0, Math.PI * 2);
  ctx.arc(x + 22 * scale, y, 10 * scale, 0, Math.PI * 2);
  ctx.arc(x - 10 * scale, y + 2 * scale, 10 * scale, 0, Math.PI * 2);
  ctx.closePath();
  ctx.fill();
};

export default function AirspaceMap({ state }) {
  const canvasRef = useRef(null);
  
  // Camera parameters
  const [zoom, setZoom] = useState(1.3);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);

  // Drag states
  const dragRef = useRef({ isDragging: false, startX: 0, startY: 0 });

  // Animation states for drones
  // Stores positions in dataset coordinate space (x: 100-700, y: 20-500)
  const droneAnimRef = useRef({});
  const lastStateIndexRef = useRef(-1);

  // Animation frame loop handles the continuous update of drone positions
  const requestRef = useRef();

  const handleMouseDown = (e) => {
    dragRef.current = {
      isDragging: true,
      startX: e.clientX - panX,
      startY: e.clientY - panY
    };
  };

  const handleMouseMove = (e) => {
    if (!dragRef.current.isDragging) return;
    setPanX(e.clientX - dragRef.current.startX);
    setPanY(e.clientY - dragRef.current.startY);
  };

  const handleMouseUp = () => {
    dragRef.current.isDragging = false;
  };

  const handleWheel = (e) => {
    e.preventDefault();
    const zoomFactor = 1.1;
    setZoom(z => e.deltaY < 0 ? Math.min(3.0, z * zoomFactor) : Math.max(0.6, z / zoomFactor));
  };

  // Detect state changes and trigger drone animations from previous positions
  useEffect(() => {
    state.drones.forEach(d => {
      const targetNode = NODES.find(n => n.id === d.node);
      if (!targetNode) return;

      const currentAnim = droneAnimRef.current[d.id];
      if (!currentAnim) {
        // First initialization
        droneAnimRef.current[d.id] = {
          x: targetNode.x,
          y: targetNode.y,
          startX: targetNode.x,
          startY: targetNode.y,
          targetX: targetNode.x,
          targetY: targetNode.y,
          t: 1.0,
          status: d.status,
          battery: d.battery
        };
      } else {
        // If the drone has moved to a new target node
        if (currentAnim.targetX !== targetNode.x || currentAnim.targetY !== targetNode.y) {
          droneAnimRef.current[d.id] = {
            ...currentAnim,
            startX: currentAnim.x, // start from its current animated location
            startY: currentAnim.y,
            targetX: targetNode.x,
            targetY: targetNode.y,
            t: 0.0, // reset animation timer
            status: d.status,
            battery: d.battery
          };
        } else {
          // just update stats
          currentAnim.status = d.status;
          currentAnim.battery = d.battery;
        }
      }
    });
  }, [state]);

  // Main daylight drawing loop with smooth drone transitions & weather alerts
  useEffect(() => {
    const draw = () => {
      const canvas = canvasRef.current;
      if (!canvas) return;
      const ctx = canvas.getContext('2d');

      // Make canvas responsive with High-DPI (Retina) support
      const dpr = window.devicePixelRatio || 1;
      const rect = canvas.parentElement.getBoundingClientRect();
      const targetWidth = Math.floor(rect.width * dpr);
      const targetHeight = Math.floor(rect.height * dpr);
      
      if (canvas.width !== targetWidth || canvas.height !== targetHeight) {
        canvas.width = targetWidth;
        canvas.height = targetHeight;
      }
      
      // Reset transform and apply Retina scaling
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

      const cx = rect.width / 2;
      const cy = rect.height / 2;

      // Project original coordinates to screen space
      const project = (x, y) => {
        const sx = cx + (x - 380) * zoom + panX;
        const sy = cy + (y - 260) * zoom + panY;
        return { x: sx, y: sy };
      };

      // Curved paths to route safely around Restricted Hospital (Node 11 at 490, 50)
      const isCurvedSegment = (n1, n2) => {
        return (n1 === 10 && n2 === 13) || (n1 === 13 && n2 === 10) ||
               (n1 === 12 && n2 === 13) || (n1 === 13 && n2 === 12);
      };

      const getControlPoint = (n1, n2) => {
        if ((n1 === 10 && n2 === 13) || (n1 === 13 && n2 === 10)) {
          // Bend edge 10 -> 13 upwards at (490, -70) to avoid Hospital cylinder
          return { x: 490, y: -70 };
        }
        if ((n1 === 12 && n2 === 13) || (n1 === 13 && n2 === 12)) {
          // Bend edge 12 -> 13 downwards at (415, 180) to avoid Hospital cylinder
          return { x: 415, y: 180 };
        }
        return null;
      };

      // --- 1. UPDATE DRONE INTERPOLATION POSITIONS (SLOW MOVEMENT) ---
      state.drones.forEach(d => {
        const anim = droneAnimRef.current[d.id];
        if (!anim) return;

        if (anim.t < 1.0) {
          // Slow speed: increment by 0.007 per frame (~2.4 seconds to travel)
          anim.t = Math.min(1.0, anim.t + 0.007);
          
          // Determine starting and ending node IDs
          const nodeObj = NODES.find(n => n.x === anim.targetX && n.y === anim.targetY);
          const startNodeObj = NODES.find(n => n.x === anim.startX && n.y === anim.startY);
          
          if (nodeObj && startNodeObj && isCurvedSegment(startNodeObj.id, nodeObj.id)) {
            // Curved routing around Hospital
            const cp = getControlPoint(startNodeObj.id, nodeObj.id);
            const t = anim.t;
            anim.x = (1 - t) * (1 - t) * anim.startX + 2 * (1 - t) * t * cp.x + t * t * anim.targetX;
            anim.y = (1 - t) * (1 - t) * anim.startY + 2 * (1 - t) * t * cp.y + t * t * anim.targetY;
          } else {
            // Straight line routing
            anim.x = anim.startX + (anim.targetX - anim.startX) * anim.t;
            anim.y = anim.startY + (anim.targetY - anim.startY) * anim.t;
          }
        }
      });

      // --- 2. DRAW GOOGLE MAPS STYLE BACKGROUND ---
      ctx.fillStyle = THEME.land;
      ctx.fillRect(0, 0, rect.width, rect.height);

      // Draw Parks
      const drawPark = (x, y, w, h) => {
        const pTopLeft = project(x, y);
        ctx.fillStyle = THEME.park;
        ctx.beginPath();
        ctx.roundRect(pTopLeft.x, pTopLeft.y, w * zoom, h * zoom, 6 * zoom);
        ctx.fill();
      };
      drawPark(150, 60, 100, 80);    // Park North-West
      drawPark(420, 100, 120, 80);   // Park North-East
      drawPark(420, 320, 140, 90);   // Park South-East
      drawPark(180, 380, 110, 85);   // Park South-West

      // Draw River
      ctx.strokeStyle = THEME.water;
      ctx.lineWidth = 35 * zoom;
      ctx.lineCap = 'round';
      ctx.lineJoin = 'round';
      ctx.beginPath();
      const riverStart = project(80, 480);
      const cp1 = project(260, 360);
      const cp2 = project(400, 220);
      const riverEnd = project(680, 60);
      ctx.moveTo(riverStart.x, riverStart.y);
      ctx.bezierCurveTo(cp1.x, cp1.y, cp2.x, cp2.y, riverEnd.x, riverEnd.y);
      ctx.stroke();

      // Draw Urban Block Building Outlines
      const drawUrbanBlock = (x, y, w, h) => {
        const p = project(x, y);
        ctx.fillStyle = 'rgba(224, 222, 218, 0.6)';
        ctx.strokeStyle = 'rgba(200, 198, 194, 0.8)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.roundRect(p.x, p.y, w * zoom, h * zoom, 3 * zoom);
        ctx.fill();
        ctx.stroke();
      };
      drawUrbanBlock(220, 90, 25, 20);
      drawUrbanBlock(260, 90, 20, 20);
      drawUrbanBlock(220, 120, 45, 15);
      drawUrbanBlock(230, 200, 30, 35);
      drawUrbanBlock(450, 130, 30, 25);
      drawUrbanBlock(490, 130, 20, 25);
      drawUrbanBlock(450, 340, 40, 25);
      drawUrbanBlock(500, 340, 30, 25);
      drawUrbanBlock(510, 240, 25, 25);
      drawUrbanBlock(230, 420, 35, 20);
      drawUrbanBlock(270, 420, 20, 20);

      // --- 3. DRAW THE HIGHWAY STREET GRID ---
      const drawHighway = (x1, y1, x2, y2) => {
        const p1 = project(x1, y1);
        const p2 = project(x2, y2);
        ctx.strokeStyle = THEME.roadBorder;
        ctx.lineWidth = 14 * zoom;
        ctx.beginPath();
        ctx.moveTo(p1.x, p1.y);
        ctx.lineTo(p2.x, p2.y);
        ctx.stroke();

        ctx.strokeStyle = THEME.roadMajor;
        ctx.lineWidth = 10 * zoom;
        ctx.beginPath();
        ctx.moveTo(p1.x, p1.y);
        ctx.lineTo(p2.x, p2.y);
        ctx.stroke();
      };
      drawHighway(100, 50, 680, 50);    // y = 50
      drawHighway(100, 160, 680, 160);  // y = 160
      drawHighway(100, 280, 680, 280);  // y = 280
      drawHighway(100, 400, 680, 400);  // y = 400
      drawHighway(200, 20, 200, 500);   // x = 200
      drawHighway(350, 20, 350, 500);   // x = 350
      drawHighway(500, 20, 500, 500);   // x = 500

      // --- 4. DRAW RESTRICTED AIRSPACES ---
      NODES.filter(n => n.type === 'R').forEach(node => {
        const p = project(node.x, node.y);
        const radius = node.id === 4 ? 55 * zoom : 45 * zoom;

        ctx.fillStyle = 'rgba(239, 68, 68, 0.16)';
        ctx.strokeStyle = '#ef4444';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(p.x, p.y, radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();

        ctx.strokeStyle = 'rgba(239, 68, 68, 0.4)';
        ctx.lineWidth = 1.5;
        ctx.setLineDash([4, 6]);
        ctx.beginPath();
        ctx.arc(p.x, p.y, radius - 6 * zoom, 0, Math.PI * 2);
        ctx.stroke();
        ctx.setLineDash([]);

        ctx.fillStyle = '#b91c1c';
        ctx.font = 'bold 9px monospace'; // Constant font size
        ctx.textAlign = 'center';
        const labelText = node.id === 4 ? 'RESTRICTED AIRPORT ZONE' : 'RESTRICTED HOSPITAL HELIPAD';
        ctx.fillText(labelText, p.x, p.y + 3);
      });

      // --- 5. DRAW GRAPH FLIGHT CORRIDORS (EDGES) ---
      EDGES.forEach(edge => {
        const fromNode = NODES.find(n => n.id === edge.from);
        const toNode = NODES.find(n => n.id === edge.to);
        if (!fromNode || !toNode) return;

        const p1 = project(fromNode.x, fromNode.y);
        const p2 = project(toNode.x, toNode.y);

        const isHighlighted = state.highlightedEdges.some(
          he => (he.from === edge.from && he.to === edge.to) || (he.from === edge.to && he.to === edge.from)
        );
        const isCongested = state.highlightedEdges.some(
          he => he.congested && ((he.from === edge.from && he.to === edge.to) || (he.from === edge.to && he.to === edge.from))
        );

        const isCurved = isCurvedSegment(edge.from, edge.to);
        const cp = isCurved ? project(getControlPoint(edge.from, edge.to).x, getControlPoint(edge.from, edge.to).y) : null;

        if (isCongested) {
          ctx.strokeStyle = 'rgba(239, 68, 68, 0.35)';
          ctx.lineWidth = 10 * zoom;
          ctx.beginPath();
          if (isCurved) {
            ctx.moveTo(p1.x, p1.y);
            ctx.quadraticCurveTo(cp.x, cp.y, p2.x, p2.y);
          } else {
            ctx.moveTo(p1.x, p1.y);
            ctx.lineTo(p2.x, p2.y);
          }
          ctx.stroke();

          ctx.strokeStyle = '#ef4444';
          ctx.lineWidth = 5; // Bold solid line
          ctx.setLineDash([5, 5]);
        } else if (isHighlighted) {
          ctx.strokeStyle = 'rgba(245, 158, 11, 0.35)';
          ctx.lineWidth = 10 * zoom;
          ctx.beginPath();
          if (isCurved) {
            ctx.moveTo(p1.x, p1.y);
            ctx.quadraticCurveTo(cp.x, cp.y, p2.x, p2.y);
          } else {
            ctx.moveTo(p1.x, p1.y);
            ctx.lineTo(p2.x, p2.y);
          }
          ctx.stroke();

          ctx.strokeStyle = '#fbbf24';
          ctx.lineWidth = 5; // Bold solid line
        } else {
          ctx.strokeStyle = 'rgba(30, 41, 59, 0.28)';
          ctx.lineWidth = 2.5; // Slightly bolder standard corridors
        }

        ctx.beginPath();
        if (isCurved) {
          ctx.moveTo(p1.x, p1.y);
          ctx.quadraticCurveTo(cp.x, cp.y, p2.x, p2.y);
        } else {
          ctx.moveTo(p1.x, p1.y);
          ctx.lineTo(p2.x, p2.y);
        }
        ctx.stroke();
        ctx.setLineDash([]); // Reset

        // Cost tags
        if (isHighlighted || isCongested) {
          const midX = isCurved ? (p1.x + cp.x * 2 + p2.x) / 4 : (p1.x + p2.x) / 2;
          const midY = isCurved ? (p1.y + cp.y * 2 + p2.y) / 4 - 8 : (p1.y + p2.y) / 2;
          
          ctx.fillStyle = isCongested ? '#ef4444' : '#d97706';
          ctx.font = 'bold 9px monospace'; // Constant size
          ctx.textAlign = 'center';
          
          const txt = `cost:${edge.weight}`;
          const tw = ctx.measureText(txt).width + 6;
          const th = 13;
          ctx.fillStyle = '#ffffff';
          ctx.strokeStyle = isCongested ? '#ef4444' : '#d97706';
          ctx.lineWidth = 1;
          ctx.beginPath();
          ctx.roundRect(midX - tw/2, midY - th/2, tw, th, 3);
          ctx.fill();
          ctx.stroke();
          
          ctx.fillStyle = isCongested ? '#b91c1c' : '#854d0e';
          ctx.fillText(txt, midX, midY + 3.5);
        }
      });

      // --- 6. DRAW GRAPH NODES (MAP PINS - CONSTANT SCREEN SIZES) ---
      NODES.forEach(node => {
        if (node.type === 'R') return; // Handled in restricted zones block
        const p = project(node.x, node.y);

        if (node.type === 'W') {
          ctx.fillStyle = '#1d4ed8';
          ctx.strokeStyle = '#ffffff';
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 11, 0, Math.PI * 2); // Constant 11px
          ctx.fill();
          ctx.stroke();

          ctx.fillStyle = '#ffffff';
          ctx.beginPath();
          ctx.arc(p.x, p.y, 7, 0, Math.PI * 2);
          ctx.fill();

          ctx.fillStyle = '#1d4ed8';
          ctx.font = 'bold 9px sans-serif';
          ctx.textAlign = 'center';
          ctx.fillText('W', p.x, p.y + 3);
        } else if (node.type === 'P') {
          const padState = state.pads.find(p => p.node === node.id);
          const isOccupied = padState ? padState.occupied : false;
          const padColor = isOccupied ? '#f59e0b' : '#10b981';

          ctx.fillStyle = '#ffffff';
          ctx.strokeStyle = padColor;
          ctx.lineWidth = 2.5;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 8.5, 0, Math.PI * 2); // Constant 8.5px
          ctx.fill();
          ctx.stroke();

          ctx.fillStyle = padColor;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 4, 0, Math.PI * 2);
          ctx.fill();

          ctx.fillStyle = padColor;
          ctx.font = 'bold 9px sans-serif';
          ctx.textAlign = 'center';
          ctx.fillText('⚡', p.x, p.y - 11);
        } else if (node.type === 'D') {
          ctx.fillStyle = '#10b981';
          ctx.strokeStyle = '#ffffff';
          ctx.lineWidth = 1.5;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 7, 0, Math.PI * 2); // Constant 7px
          ctx.fill();
          ctx.stroke();

          ctx.fillStyle = '#ffffff';
          ctx.beginPath();
          ctx.arc(p.x, p.y, 2.5, 0, Math.PI * 2);
          ctx.fill();
        } else if (node.type === 'C') {
          ctx.fillStyle = '#ffffff';
          ctx.strokeStyle = '#2563eb';
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.arc(p.x, p.y, 5, 0, Math.PI * 2); // Constant 5px
          ctx.fill();
          ctx.stroke();
        }

        ctx.fillStyle = '#0f172a';
        ctx.font = 'bold 9px Inter, sans-serif'; // Constant size
        ctx.textAlign = 'center';
        
        let labelOffset = 13;
        if (node.type === 'W') labelOffset = 16;
        ctx.fillText(node.name.replace(/_/g, ' '), p.x, p.y - labelOffset);
      });

      // --- 7. DRAW EXTRA PASSIVE CUSTOMER PINS FOR DENSITY ---
      ADDED_CUSTOMERS.forEach(cust => {
        const p = project(cust.x, cust.y);
        ctx.fillStyle = 'rgba(16, 185, 129, 0.45)';
        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 1.2;
        ctx.beginPath();
        ctx.arc(p.x, p.y, 6, 0, Math.PI * 2); // Constant 6px
        ctx.fill();
        ctx.stroke();

        ctx.fillStyle = 'rgba(15, 23, 42, 0.5)';
        ctx.font = 'bold 8px Inter, sans-serif'; // Constant size
        ctx.textAlign = 'center';
        ctx.fillText(cust.name.replace(/_/g, ' '), p.x, p.y - 9);
      });

      // --- 8. DRAW ACTIVE DRONES (WITH SMOOTH REAL-TIME INTERPOLATION & CARDS) ---
      const dronePositions = {};
      state.drones.forEach(d => {
        const anim = droneAnimRef.current[d.id];
        if (!anim) return;
        
        const isStationary = anim.t >= 1.0 || (anim.startX === anim.targetX && anim.startY === anim.targetY);
        if (isStationary) {
          const key = `${anim.targetX},${anim.targetY}`;
          dronePositions[key] = (dronePositions[key] || 0) + 1;
        }
      });

      const dronePositionIndices = {};

      state.drones.forEach(d => {
        const anim = droneAnimRef.current[d.id];
        if (!anim) return;

        let px = anim.x;
        let py = anim.y;

        const isStationary = anim.t >= 1.0 || (anim.startX === anim.targetX && anim.startY === anim.targetY);
        if (isStationary) {
          const key = `${anim.targetX},${anim.targetY}`;
          const totalAtPos = dronePositions[key] || 1;
          if (totalAtPos > 1) {
            const index = dronePositionIndices[key] || 0;
            dronePositionIndices[key] = index + 1;

            const angle = (index / totalAtPos) * Math.PI * 2;
            const offsetRadius = 14;
            const screenOffset = offsetRadius / zoom;
            px += Math.cos(angle) * screenOffset;
            py += Math.sin(angle) * screenOffset;
          }
        }

        const p = project(px, py);

        const isEmergency = d.status.includes('EMERGENCY') || d.status.includes('LOW');
        const isSelected = d.status.includes('SELECTED');
        const colorDrone = isEmergency ? '#ef4444' : d.status.includes('CHARG') ? '#10b981' : isSelected ? '#f59e0b' : '#2563eb';

        // Draw quadcopter circular base (Constant Size)
        const droneRad = 11; // Constant 11px
        ctx.fillStyle = 'rgba(15, 23, 42, 0.9)';
        ctx.strokeStyle = colorDrone;
        ctx.lineWidth = 2.5;
        
        ctx.beginPath();
        ctx.arc(p.x, p.y, droneRad, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();

        // Draw cross lines for rotors (Constant Size)
        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 1.8;
        ctx.beginPath();
        const crossSize = 7;
        ctx.moveTo(p.x - crossSize, p.y - crossSize);
        ctx.lineTo(p.x + crossSize, p.y + crossSize);
        ctx.moveTo(p.x - crossSize, p.y + crossSize);
        ctx.lineTo(p.x + crossSize, p.y - crossSize);
        ctx.stroke();

        // Rotor nodes (Constant Size)
        ctx.fillStyle = colorDrone;
        ctx.beginPath();
        ctx.arc(p.x - crossSize, p.y - crossSize, 2.5, 0, Math.PI * 2);
        ctx.arc(p.x + crossSize, p.y + crossSize, 2.5, 0, Math.PI * 2);
        ctx.arc(p.x - crossSize, p.y + crossSize, 2.5, 0, Math.PI * 2);
        ctx.arc(p.x + crossSize, p.y - crossSize, 2.5, 0, Math.PI * 2);
        ctx.fill();

        // Blinking LED status dot
        const blink = Date.now() % 600 < 300;
        ctx.fillStyle = blink ? '#38a169' : '#ffffff';
        if (isEmergency) ctx.fillStyle = blink ? '#ef4444' : '#ffffff';
        ctx.beginPath();
        ctx.arc(p.x, p.y, 2, 0, Math.PI * 2);
        ctx.fill();

        // Pulse ring
        ctx.strokeStyle = colorDrone;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        const pulseRad = droneRad + 4 + Math.sin(Date.now() * 0.008) * 3;
        ctx.arc(p.x, p.y, pulseRad, 0, Math.PI * 2);
        ctx.stroke();

        // Only draw high-legibility telemetry bubbles for non-idle or selected drones
        const isIdle = d.status === 'IDLE' || d.status.includes('REJECTED');
        const showBubble = !isIdle || isSelected;

        if (showBubble) {
          let loadIcon = '💤 IDLE';
          if (d.status.includes('DELIVER') || d.status.includes('ACTIVE')) {
            loadIcon = '📦 LOADED';
          } else if (d.status.includes('RETURN')) {
            loadIcon = '📭 EMPTY';
          } else if (d.status.includes('CHARG')) {
            loadIcon = '🔌 CHARGE';
          }

          const cardW = 85; // Constant width in pixels
          const cardH = 28; // Constant height in pixels
          const cardX = p.x - cardW / 2;
          const cardY = p.y - droneRad - cardH - 6; // Constant height offset

          ctx.fillStyle = '#1e293b'; // slate dark background
          ctx.strokeStyle = colorDrone;
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.roundRect(cardX, cardY, cardW, cardH, 5);
          ctx.fill();
          ctx.stroke();

          // Battery gauge bar
          const barW = cardW - 8;
          const barH = 3;
          const barX = cardX + 4;
          const barY = cardY + cardH - 6;
          const fillW = (d.battery / 100) * barW;
          const battColor = d.battery >= 60 ? '#10b981' : d.battery >= 30 ? '#f59e0b' : '#ef4444';
          
          ctx.fillStyle = 'rgba(74, 85, 104, 0.5)';
          ctx.fillRect(barX, barY, barW, barH);
          ctx.fillStyle = battColor;
          ctx.fillRect(barX, barY, fillW, barH);

          // Drone ID & status text
          ctx.fillStyle = '#f8fafc';
          ctx.font = 'bold 8.5px monospace'; // Constant font size
          ctx.textAlign = 'center';
          ctx.fillText(`D${d.id} | ${loadIcon}`, cardX + cardW/2, cardY + 9);
          
          ctx.fillStyle = '#94a3b8';
          ctx.font = 'bold 8px sans-serif'; // Constant font size
          ctx.fillText(`Batt: ${d.battery}%`, cardX + cardW/2, cardY + 17);
        }
      });

      // --- 9. DRAW STORMY WEATHER EFFECT (STATE INDEX 5 SPECIFIC) ---
      const isStormState = state.description.toLowerCase().includes('storm') || state.label.toLowerCase().includes('storm');
      
      if (isStormState) {
        // Flash storm sky colors (dark slate translucent overlay)
        ctx.fillStyle = 'rgba(15, 23, 42, 0.28)';
        ctx.fillRect(0, 0, rect.width, rect.height);

        // Draw Dark storm clouds at top
        ctx.fillStyle = 'rgba(71, 85, 105, 0.82)';
        drawCloud(ctx, rect.width * 0.15, rect.height * 0.1, 1.4);
        drawCloud(ctx, rect.width * 0.5, rect.height * 0.08, 1.6);
        drawCloud(ctx, rect.width * 0.85, rect.height * 0.12, 1.5);

        // Draw diagonal rain lines falling down
        ctx.strokeStyle = 'rgba(186, 230, 253, 0.4)';
        ctx.lineWidth = 1;
        const rainCount = 40;
        for (let i = 0; i < rainCount; i++) {
          const rx = (Math.sin(i * 123.45) * 0.5 + 0.5) * rect.width;
          const ry = ((i * 41 + Date.now() * 0.12) % rect.height);
          ctx.beginPath();
          ctx.moveTo(rx, ry);
          ctx.lineTo(rx - 8, ry + 18);
          ctx.stroke();
        }

        // Random lightning strike triggers every 1.5s
        if (Math.random() < 0.02) {
          ctx.fillStyle = 'rgba(255, 255, 255, 0.85)';
          ctx.fillRect(0, 0, rect.width, rect.height);
          
          // Draw a jagged lightning bolt down the center
          ctx.strokeStyle = '#ffffff';
          ctx.lineWidth = 3;
          ctx.shadowColor = '#e0f2fe';
          ctx.shadowBlur = 15;
          ctx.beginPath();
          const startX = rect.width * (0.3 + Math.random() * 0.4);
          ctx.moveTo(startX, 0);
          ctx.lineTo(startX - 20, rect.height * 0.25);
          ctx.lineTo(startX + 10, rect.height * 0.38);
          ctx.lineTo(startX - 15, rect.height * 0.65);
          ctx.stroke();
          ctx.shadowBlur = 0; // reset
        }

        // Weather banner warning (Constant sizes)
        const bannerW = 260;
        const bannerH = 18;
        const bx = cx - bannerW / 2;
        const by = 8;

        ctx.fillStyle = '#ef4444'; // warning red background
        ctx.beginPath();
        ctx.roundRect(bx, by, bannerW, bannerH, 4);
        ctx.fill();

        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 9px monospace'; // Constant size
        ctx.textAlign = 'center';
        ctx.fillText('⚠ SEVERE WEATHER WARNING: THUNDERSTORM ACTIVE', cx, by + 12);
      }

      requestRef.current = requestAnimationFrame(draw);
    };

    requestRef.current = requestAnimationFrame(draw);

    return () => {
      cancelAnimationFrame(requestRef.current);
    };
  }, [zoom, panX, panY, state]);

  return (
    <div className="panel airspace-panel" style={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
      <div className="panel-header" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <span className="panel-icon">🗺</span>
          <h2>2D Airspace City Map (Google Maps Overlay)</h2>
        </div>
        <div style={{ display: 'flex', gap: '6px' }}>
          <button 
            className="btn btn-outline" 
            style={{ padding: '3px 8px', fontSize: '10px' }} 
            onClick={() => setZoom(z => Math.max(0.6, z - 0.1))}
            title="Zoom Out"
          >
            ➖ Zoom Out
          </button>
          <button 
            className="btn btn-outline" 
            style={{ padding: '3px 8px', fontSize: '10px' }} 
            onClick={() => setZoom(z => Math.min(2.5, z + 0.1))}
            title="Zoom In"
          >
            ➕ Zoom In
          </button>
          <button 
            className="btn btn-outline" 
            style={{ padding: '3px 8px', fontSize: '10px' }} 
            onClick={() => { setZoom(1.3); setPanX(0); setPanY(0); }}
            title="Reset View"
          >
            ⌂ Center Map
          </button>
        </div>
      </div>

      <div style={{ flex: 1, position: 'relative', overflow: 'hidden', borderRadius: '8px', border: '1px solid var(--bg-border)', background: THEME.land }}>
        <canvas
          ref={canvasRef}
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onMouseLeave={handleMouseUp}
          onWheel={handleWheel}
          style={{ position: 'absolute', top: 0, left: 0, width: '100%', height: '100%', display: 'block', cursor: dragRef.current.isDragging ? 'grabbing' : 'grab' }}
        />
        {/* Simple navigation directional pad */}
        <div style={{ position: 'absolute', bottom: '8px', left: '8px', display: 'grid', gridTemplateColumns: 'repeat(3, 22px)', gap: '2px', background: 'rgba(255,255,255,0.85)', padding: '4px', borderRadius: '6px', border: '1px solid #cbd5e1' }}>
          <div />
          <button style={{ border: 'none', background: '#cbd5e1', cursor: 'pointer', fontSize: '10px', padding: '2px', borderRadius: '3px' }} onClick={() => setPanY(py => py - 25)}>▲</button>
          <div />
          <button style={{ border: 'none', background: '#cbd5e1', cursor: 'pointer', fontSize: '10px', padding: '2px', borderRadius: '3px' }} onClick={() => setPanX(px => px - 25)}>◀</button>
          <button style={{ border: 'none', background: '#e2e8f0', cursor: 'pointer', fontSize: '9px', padding: '2px', borderRadius: '3px' }} onClick={() => { setPanX(0); setPanY(0); }}>●</button>
          <button style={{ border: 'none', background: '#cbd5e1', cursor: 'pointer', fontSize: '10px', padding: '2px', borderRadius: '3px' }} onClick={() => setPanX(px => px + 25)}>▶</button>
          <div />
          <button style={{ border: 'none', background: '#cbd5e1', cursor: 'pointer', fontSize: '10px', padding: '2px', borderRadius: '3px' }} onClick={() => setPanY(py => py + 25)}>▼</button>
          <div />
        </div>
        <div style={{ position: 'absolute', bottom: '8px', right: '8px', pointerEvents: 'none', background: 'rgba(255,255,255,0.85)', padding: '4px 8px', borderRadius: '4px', border: '1px solid #cbd5e1', fontSize: '9px', color: '#475569', fontWeight: 'bold' }}>
          Left-Click + Drag: Pan Map | Scroll: Zoom
        </div>
      </div>
    </div>
  );
}
