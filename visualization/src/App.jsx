import { useState, useEffect, useCallback } from 'react';
import simulationStates from './data/simulationStates.js';
import AirspaceMap    from './components/AirspaceMap.jsx';
import FleetDashboard from './components/FleetDashboard.jsx';
import PackageQueue   from './components/PackageQueue.jsx';
import MissionPanel   from './components/MissionPanel.jsx';
import EventLog       from './components/EventLog.jsx';
import Controls       from './components/Controls.jsx';
import './App.css';

const TOTAL_STATES = simulationStates.length;
const AUTO_ADVANCE_MS = 2800; // ms between auto-ticks in Run Demo mode

const DSA_CONCEPTS = [
  {
    title: "Graph Topology & Node Mapping",
    details: "Constructing the 3D Airspace city map as an Adjacency List Graph containing Warehouse, Charging Pads, Corridor Hubs, Delivery Zones, and restricted No-Fly boundaries."
  },
  {
    title: "FIFO Queue & Hash Map Address Lookup",
    details: "FIFO queue buffer (std::queue) guarantees orderly package loading. Hash Map (std::unordered_map) converts delivery street addresses to GPS locations in O(1) time."
  },
  {
    title: "Min-Heap Drone Selection & Dijkstra's pathfinding",
    details: "Min-Heap Priority Queue (std::priority_queue) extracts drones with maximum battery margin. Dijkstra's Algorithm computes the shortest collision-free 3D airspace route."
  },
  {
    title: "Real-time Airspace Edge Reservations",
    details: "Simulating path ticks. Multi-capacity airspace corridors enforce traffic edge limits to prevent drone bottlenecks."
  },
  {
    title: "Delivery Status Verification & Return Routing",
    details: "Checking delivery destination reachability, updating package status to DELIVERED, and computing the optimal return path to Warehouse."
  },
  {
    title: "Stack-based Flight State Rollback (Flight Undo)",
    details: "Sudden thunderstorm alert. Using Stack backtracking (std::stack) to pop historical safe coordinates, reverting the drone's position to its last safe checkpoint."
  },
  {
    title: "Airspace Congestion Detection",
    details: "Corridor capacity constraint triggered (2/2 drones). Drone 103 waits at Node 10, incrementing block ticks before recalculating route."
  },
  {
    title: "Dijkstra Rerouting for Congestion Recovery",
    details: "Wait threshold exceeded (3/3). Dijkstra's algorithm dynamically adjusts edge weight to calculate detours bypassing congested corridors."
  },
  {
    title: "Resource Allocation (Charging Pad Manager)",
    details: "Assigning critical-battery drone to nearest rooftop charging pad. Ignores busy pads (Pads 1 & 2) and locks Pad 3 to prevent landing collisions."
  },
  {
    title: "Concurrent Mission Execution & Pad Locking",
    details: "Managing multiple active drone flights simultaneously. Enforcing single-occupancy locks on charging pads to prevent landing collisions."
  },
  {
    title: "Depot Return Routing & Charge Rates",
    details: "Drones returning to Warehouse Alpha release corridor capacity. Charging Pad Manager updates status and executes battery replenishment rates."
  },
  {
    title: "Iterative FIFO Dispatching & Route Generation",
    details: "FIFO queue pops the next package. Min-heap priorities select candidate Drone 104, and Dijkstra computes a safe route to Customer Zone D."
  },
  {
    title: "Simultaneous Transit & Resource Reclamation",
    details: "Simulating ticks for multiple drones in different modes. Drone 105 finishes charging, releases Pad 3, and transitions back to Warehouse."
  },
  {
    title: "Archiving Completed Missions & Fleet Reclamation",
    details: "Package 1004 is successfully delivered. The finished mission is archived, and Drone 105 returns to the idle pool at Warehouse."
  },
  {
    title: "Curved Airspace Geometry Routing",
    details: "Dijkstra computes final route. The rendering engine activates quadratic Bezier curved paths to steer the drone around Restricted Hospital airspace boundaries."
  },
  {
    title: "Final Phase Delivery & Return Flow",
    details: "The final package reaches Customer Zone E (Node 13). Drone 105 begins its return leg along the curved, safe corridor."
  },
  {
    title: "System Verification & Invariant Proofs",
    details: "Final audit verifies 100% path safety compliance. 5 deliveries completed, 0 battery depletions, 0 restricted air space violations. System status: PASSED."
  }
];

export default function App() {
  const [stateIndex, setStateIndex] = useState(0);
  const [isRunning, setIsRunning]   = useState(false);

  const currentState = simulationStates[stateIndex];
  const currentDsa = DSA_CONCEPTS[stateIndex] || { title: "Simulation Step", details: "" };

  const handleNext = useCallback(() => {
    setStateIndex(i => Math.min(i + 1, TOTAL_STATES - 1));
  }, []);

  const handleReset = useCallback(() => {
    setIsRunning(false);
    setStateIndex(0);
  }, []);

  const handleRunDemo = useCallback(() => {
    setStateIndex(0);
    setIsRunning(true);
  }, []);

  // Auto-advance timer
  useEffect(() => {
    if (!isRunning) return;
    if (stateIndex >= TOTAL_STATES - 1) {
      setIsRunning(false);
      return;
    }
    const timer = setTimeout(() => {
      setStateIndex(i => i + 1);
    }, AUTO_ADVANCE_MS);
    return () => clearTimeout(timer);
  }, [isRunning, stateIndex]);

  return (
    <div className="app-shell">
      {/* ── Top Bar ─────────────────────────────────────────── */}
      <header className="top-bar">
        <div className="top-bar-left">
          <div className="logo-mark">◈</div>
          <div className="top-bar-title">
            <h1>Courier Dispatch Airspace Drone Matrix</h1>
            <p className="top-bar-sub">DSA Project — Simulation Visualization Layer</p>
          </div>
        </div>
        <div className="top-bar-right">
          <span className="stage-chip">{currentState.label}</span>
        </div>
      </header>

      {/* ── DSA Concept Banner ───────────────────────────────── */}
      <div className="dsa-concept-banner">
        <div className="dsa-concept-header">
          <span className="dsa-badge">CORE DSA CONCEPT</span>
          <h3 className="dsa-title">{currentDsa.title}</h3>
        </div>
        <p className="dsa-details">{currentDsa.details}</p>
      </div>

      {/* ── Stage Description ────────────────────────────────── */}
      <div className="stage-banner">
        <span className="stage-desc"><strong>Action:</strong> {currentState.description}</span>
      </div>

      {/* ── Main Layout ─────────────────────────────────────── */}
      <main className="main-layout">

        {/* Left Column — Map (dominates) */}
        <section className="col-left">
          <AirspaceMap state={currentState} />
        </section>

        {/* Right Column — Panels */}
        <section className="col-right">
          <Controls
            currentIndex={stateIndex}
            total={TOTAL_STATES}
            onNext={handleNext}
            onReset={handleReset}
            onRunDemo={handleRunDemo}
            isRunning={isRunning}
          />
          <FleetDashboard state={currentState} />
          <PackageQueue   state={currentState} />
          <MissionPanel   state={currentState} />
          <EventLog       state={currentState} />
        </section>

      </main>

      {/* ── Footer ──────────────────────────────────────────── */}
      <footer className="footer">
        <span>C++ Backend: Dijkstra · FIFO Queue · Min-Heap · Stack Checkpoints · Graph · HashMap · Congestion Recovery</span>
        <span className="footer-sep">|</span>
        <span>Visualization Layer — React + SVG</span>
      </footer>
    </div>
  );
}
