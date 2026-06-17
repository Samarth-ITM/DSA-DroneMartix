// Simulation states for the Courier Dispatch Airspace Drone Matrix visualization
// Modelled directly from the C++ backend data files (city_graph.txt, drones.txt, pads.txt)

// ─── GRAPH TOPOLOGY ──────────────────────────────────────────────────────────
// Derived from data/city_graph.txt
// Positions are chosen to produce a clean readable layout inside the SVG canvas
export const NODES = [
  { id: 0,  name: 'Warehouse_Alpha',       type: 'W', x: 350, y: 280 },
  { id: 1,  name: 'North_Corridor_1',      type: 'C', x: 350, y: 160 },
  { id: 2,  name: 'East_Corridor_1',       type: 'C', x: 500, y: 280 },
  { id: 3,  name: 'South_Corridor_1',      type: 'C', x: 350, y: 400 },
  { id: 4,  name: 'Restricted_Airport',    type: 'R', x: 190, y: 280 },
  { id: 5,  name: 'Customer_Zone_A',       type: 'D', x: 200, y: 160 },
  { id: 6,  name: 'Customer_Zone_B',       type: 'D', x: 500, y: 160 },
  { id: 7,  name: 'Customer_Zone_C',       type: 'D', x: 500, y: 400 },
  { id: 8,  name: 'Charging_Pad_1',        type: 'P', x: 200, y: 220 },
  { id: 9,  name: 'Charging_Pad_2',        type: 'P', x: 500, y: 220 },
  { id: 10, name: 'Central_Hub',           type: 'C', x: 350, y: 50  },
  { id: 11, name: 'Restricted_Hospital',   type: 'R', x: 490, y: 50  },
  { id: 12, name: 'Customer_Zone_D',       type: 'D', x: 200, y: 50  },
  { id: 13, name: 'Customer_Zone_E',       type: 'D', x: 630, y: 50  },
  { id: 14, name: 'Charging_Pad_3',        type: 'P', x: 350, y: 470 },
];

// Derived from data/city_graph.txt EDGE definitions
export const EDGES = [
  { from: 0,  to: 1,  weight: 5,  capacity: 3 },
  { from: 0,  to: 2,  weight: 6,  capacity: 3 },
  { from: 0,  to: 3,  weight: 7,  capacity: 3 },
  { from: 0,  to: 4,  weight: 4,  capacity: 1 },
  { from: 1,  to: 5,  weight: 8,  capacity: 2 },
  { from: 1,  to: 8,  weight: 3,  capacity: 2 },
  { from: 1,  to: 10, weight: 4,  capacity: 4 },
  { from: 2,  to: 6,  weight: 7,  capacity: 2 },
  { from: 2,  to: 9,  weight: 4,  capacity: 2 },
  { from: 2,  to: 10, weight: 5,  capacity: 4 },
  { from: 3,  to: 7,  weight: 9,  capacity: 2 },
  { from: 3,  to: 14, weight: 5,  capacity: 2 },
  { from: 3,  to: 10, weight: 6,  capacity: 4 },
  { from: 4,  to: 12, weight: 10, capacity: 1 },
  { from: 5,  to: 10, weight: 5,  capacity: 2 },
  { from: 6,  to: 10, weight: 4,  capacity: 2 },
  { from: 7,  to: 10, weight: 6,  capacity: 2 },
  { from: 8,  to: 10, weight: 3,  capacity: 2 },
  { from: 9,  to: 10, weight: 3,  capacity: 2 },
  { from: 10, to: 11, weight: 2,  capacity: 1 },
  { from: 10, to: 12, weight: 6,  capacity: 2 },
  { from: 10, to: 13, weight: 7,  capacity: 2 },
  { from: 10, to: 14, weight: 4,  capacity: 2 },
  { from: 12, to: 13, weight: 5,  capacity: 2 },
];

// ─── SIMULATION STATES ───────────────────────────────────────────────────────

const simulationStates = [

  // ── STATE 0 ─ System Initialization ────────────────────────────────────────
  {
    label: 'Stage 1 — System Initialization',
    description: 'Fleet loaded. 5 Drones at Warehouse (Node 0). 3 Charging Pads online. 2 No-Fly Zones active.',
    drones: [
      { id: 101, battery: 94, node: 0, status: 'IDLE' },
      { id: 102, battery: 85, node: 0, status: 'IDLE' },
      { id: 103, battery: 45, node: 0, status: 'IDLE' },
      { id: 104, battery: 86, node: 2, status: 'IDLE' },
      { id: 105, battery: 30, node: 0, status: 'IDLE' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'PENDING' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING' },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING' },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING' },
    ],
    missions: [],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: ['[Init] System boot complete.', '[Init] 5 drones loaded.', '[Init] 3 charging pads online.', '[Init] 2 restricted zones enforced.'],
  },

  // ── STATE 1 ─ Package Queue & Address Lookup ────────────────────────────────
  {
    label: 'Stage 2 — Package Queue (FIFO) & Address Lookup (HashMap)',
    description: 'FIFO queue holds 5 packages. HashMap resolves OrderID 1001 → GPS → Node 5.',
    drones: [
      { id: 101, battery: 94, node: 0, status: 'IDLE' },
      { id: 102, battery: 85, node: 0, status: 'IDLE' },
      { id: 103, battery: 45, node: 0, status: 'IDLE' },
      { id: 104, battery: 86, node: 2, status: 'IDLE' },
      { id: 105, battery: 30, node: 0, status: 'IDLE' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'RESOLVING' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING'   },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'   },
    ],
    missions: [],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: ['[Queue] 5 packages enqueued in FIFO order.', '[HashMap] OrderID 1001 → lat 19.0760, lng 72.8777 → Node 5.', '[Queue] Package #1001 is next for dispatch.'],
  },

  // ── STATE 2 ─ Drone Selection & Route Generation ────────────────────────────
  {
    label: 'Stage 3–4 — Drone Selection (Heap) + Route Generation (Dijkstra)',
    description: 'Drone 101 wins the selection heap. Dijkstra computes route 0 → 1 → 10 → 13 (cost: 27).',
    drones: [
      { id: 101, battery: 94, node: 0, status: 'SELECTED (WINNER)' },
      { id: 102, battery: 85, node: 0, status: 'IDLE'              },
      { id: 103, battery: 45, node: 0, status: 'REJECTED (LOW MARGIN)' },
      { id: 104, battery: 86, node: 2, status: 'IDLE'              },
      { id: 105, battery: 30, node: 0, status: 'REJECTED (LOW MARGIN)' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_E (Node 13)', status: 'DISPATCHED' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING'    },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING'    },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'    },
      { id: 1005, dest: 'Customer_Zone_A (Node 5)',  status: 'PENDING'    },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 13], progress: 0, dest: 13, status: 'ACTIVE' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 0, to: 1  },
      { from: 1, to: 10 },
      { from: 10, to: 13 },
    ],
    events: ['[Heap] Drone 101 selected (margin: +41). Drone 103 rejected (margin: -8).', '[Dijkstra] Route computed: 0 → 1 → 10 → 13 (Total cost: 27 units).', '[Dispatch] Mission 501 created. Package 1001 assigned to Drone 101.'],
  },

  // ── STATE 3 ─ Tick Progression ──────────────────────────────────────────────
  {
    label: 'Stage 5–6 — Tick Simulation + Fleet Dashboard',
    description: 'Tick 1: Drone 101 moves 0→1. Tick 2: 1→10. Corridors reserved/released per tick.',
    drones: [
      { id: 101, battery: 77, node: 10, status: 'DELIVERING' },
      { id: 102, battery: 85, node: 0,  status: 'IDLE'       },
      { id: 103, battery: 45, node: 0,  status: 'IDLE'       },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'       },
      { id: 105, battery: 30, node: 0,  status: 'IDLE'       },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_E (Node 13)', status: 'IN_TRANSIT' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING'    },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING'    },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'    },
      { id: 1005, dest: 'Customer_Zone_A (Node 5)',  status: 'PENDING'    },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 13], progress: 2, dest: 13, status: 'ACTIVE' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 1,  to: 10 },
      { from: 10, to: 13 },
    ],
    events: ['[Tick 1] Drone 101: 0 → 1. Battery: 94% → 86%. Corridor [0→1] reserved.', '[Tick 2] Drone 101: 1 → 10. Battery: 86% → 77%. Corridor [1→10] reserved. [0→1] released.', '[Tick 3 pending] Drone 101 at Central_Hub. Next: Node 13.'],
  },

  // ── STATE 4 ─ Delivery Complete ─────────────────────────────────────────────
  {
    label: 'Stage 6 — Delivery Complete',
    description: 'Drone 101 delivered Package #1001 to Node 13. Now returning to Warehouse.',
    drones: [
      { id: 101, battery: 68, node: 13, status: 'RETURNING' },
      { id: 102, battery: 85, node: 0,  status: 'IDLE'      },
      { id: 103, battery: 45, node: 0,  status: 'IDLE'      },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'      },
      { id: 105, battery: 30, node: 0,  status: 'IDLE'      },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_E (Node 13)', status: 'DELIVERED' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING'   },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'   },
      { id: 1005, dest: 'Customer_Zone_A (Node 5)',  status: 'PENDING'   },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 13], progress: 3, dest: 13, status: 'RETURNING' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: ['[Tick 3] Drone 101: 10 → 13. Battery: 77% → 68%. Package 1001 DELIVERED!', '[Mission 501] Status: RETURNING. Drone 101 heading back to Warehouse_Alpha.'],
  },

  // ── STATE 5 ─ Storm Event ────────────────────────────────────────────────────
  {
    label: 'Stage 7–8 — Storm Event + Stack Rollback (Undo)',
    description: 'Thunderstorm triggered. Drone 101 emergency halted. Stack pops last checkpoint → Node 10.',
    drones: [
      { id: 101, battery: 77, node: 10, status: 'EMERGENCY' },
      { id: 102, battery: 85, node: 0,  status: 'IDLE'      },
      { id: 103, battery: 45, node: 0,  status: 'IDLE'      },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'      },
      { id: 105, battery: 30, node: 0,  status: 'IDLE'      },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_E (Node 13)', status: 'IN_TRANSIT' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'PENDING'    },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'PENDING'    },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'    },
      { id: 1005, dest: 'Customer_Zone_A (Node 5)',  status: 'PENDING'    },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 13], progress: 2, dest: 13, status: 'EMERGENCY' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: ['[ALERT] Severe thunderstorm warning. Emergency protocol activated!', '[Storm] Delivery halted. Drone 101 emergency state.', '[Stack Undo] Checkpoint popped: Node 13 → Node 10. Battery restored: 68% → 77%.', '[Stack] Flight state successfully rolled back to safe checkpoint.'],
  },

  // ── STATE 6 ─ Congestion Scenario ───────────────────────────────────────────
  {
    label: 'Stage 9 — Congestion Detection',
    description: 'Corridor [10→5] is FULL (2/2). Drone 103 blocked. Wait ticks: 1/3.',
    drones: [
      { id: 101, battery: 77, node: 10, status: 'DELIVERING' },
      { id: 102, battery: 85, node: 10, status: 'DELIVERING' },
      { id: 103, battery: 40, node: 10, status: 'WAITING'    },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'       },
      { id: 105, battery: 30, node: 0,  status: 'IDLE'       },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'IN_TRANSIT' },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'IN_TRANSIT' },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'BLOCKED'    },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'    },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'    },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 5],  progress: 2, dest: 5, status: 'ACTIVE'  },
      { id: 502, droneId: 102, packageId: 1002, route: [0, 1, 10, 5],  progress: 2, dest: 5, status: 'ACTIVE'  },
      { id: 503, droneId: 103, packageId: 1003, route: [0, 1, 10, 5],  progress: 2, dest: 5, status: 'WAITING' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 10, to: 5, congested: true },
    ],
    events: ['[Congestion] Corridor [10→5] FULL (2/2 capacity).', '[Drone 103] Entry to corridor [10→5] rejected. Wait ticks: 1/3.', '[Dijkstra] Searching for congestion-free alternative route...'],
  },

  // ── STATE 7 ─ Congestion Reroute ─────────────────────────────────────────────
  {
    label: 'Stage 9 — Congestion Reroute (Dijkstra Recovery)',
    description: 'Wait threshold exceeded (3/3). Dijkstra reroutes Drone 103: 10→8→5. Recovery successful.',
    drones: [
      { id: 101, battery: 62, node: 5, status: 'DELIVERING' },
      { id: 102, battery: 62, node: 5, status: 'DELIVERING' },
      { id: 103, battery: 32, node: 8, status: 'DELIVERING' },
      { id: 104, battery: 86, node: 2, status: 'IDLE'       },
      { id: 105, battery: 30, node: 0, status: 'IDLE'       },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'  },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'  },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 5],    progress: 3, dest: 5, status: 'ACTIVE' },
      { id: 502, droneId: 102, packageId: 1002, route: [0, 1, 10, 5],    progress: 3, dest: 5, status: 'ACTIVE' },
      { id: 503, droneId: 103, packageId: 1003, route: [10, 8, 5],       progress: 1, dest: 5, status: 'ACTIVE' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 10, to: 8  },
      { from: 8,  to: 5  },
    ],
    events: ['[Deadlock] Wait threshold reached (3/3 ticks). Forced reroute triggered!', '[Dijkstra] New route computed: 10 → 8 → 5 (Congested corridor avoided).', '[Recovery] Drone 103 successfully rerouted. Mission 503 status: ACTIVE.'],
  },

  // ── STATE 8 ─ Charging Pad Selection ────────────────────────────────────────
  {
    label: 'Stage 10 — Charging Pad Selection (Dijkstra + Occupancy Filter)',
    description: 'Drone 105 battery critical. Pads 1 & 2 occupied → ignored. Pad 3 (Node 14) selected.',
    drones: [
      { id: 101, battery: 62, node: 5,  status: 'DELIVERING'   },
      { id: 102, battery: 62, node: 5,  status: 'DELIVERING'   },
      { id: 103, battery: 32, node: 5,  status: 'DELIVERING'   },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'         },
      { id: 105, battery: 12, node: 0,  status: 'LOW BATTERY → CHARGING' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)', status: 'IN_TRANSIT' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'  },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'  },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 5],  progress: 3, dest: 5,  status: 'ACTIVE'      },
      { id: 502, droneId: 102, packageId: 1002, route: [0, 1, 10, 5],  progress: 3, dest: 5,  status: 'ACTIVE'      },
      { id: 503, droneId: 103, packageId: 1003, route: [10, 8, 5],     progress: 2, dest: 5,  status: 'ACTIVE'      },
      { id: 504, droneId: 105, packageId: null, route: [0, 3, 14],     progress: 0, dest: 14, status: 'EMERGENCY' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 0,  to: 3  },
      { from: 3,  to: 14 },
    ],
    events: ['[Battery] Drone 105 battery critical: 12%.', '[Pad Scan] Pad 1 (Node 8): OCCUPIED → Ignored.', '[Pad Scan] Pad 2 (Node 9): OCCUPIED → Ignored.', '[Pad Scan] Pad 3 (Node 14): FREE → Selected (distance: 12 units).', '[Routing] Drone 105 routed to Pad 3 via: 0 → 3 → 14.'],
  },

  // ── STATE 9 ─ Concurrent Deliveries & Pad Landing ─────────────────────────
  {
    label: 'Stage 11 — Concurrent Deliveries & Pad Landing',
    description: 'Drones 101, 102, 103 land at Customer Zone A to complete dispatches. Drone 105 lands at Pad 3 to start charging.',
    drones: [
      { id: 101, battery: 61, node: 5,  status: 'DELIVERING (LANDED)' },
      { id: 102, battery: 61, node: 5,  status: 'DELIVERING (LANDED)' },
      { id: 103, battery: 31, node: 5,  status: 'DELIVERING (LANDED)' },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'                },
      { id: 105, battery: 12, node: 14, status: 'CHARGING (PAD 3)'     },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'   },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 5], progress: 3, dest: 5, status: 'ARCHIVED'  },
      { id: 502, droneId: 102, packageId: 1002, route: [0, 1, 10, 5], progress: 3, dest: 5, status: 'ARCHIVED'  },
      { id: 503, droneId: 103, packageId: 1003, route: [10, 8, 5],    progress: 2, dest: 5, status: 'ARCHIVED'  },
      { id: 504, droneId: 105, packageId: null, route: [0, 3, 14],    progress: 2, dest: 14, status: 'CHARGING' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: true  },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: true  },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: true  },
    ],
    highlightedEdges: [],
    events: [
      '[Delivery] Drones 101, 102, 103 completed package deliveries at Customer Zone A.',
      '[Queue] Package #1001, #1002, and #1003 statuses updated to DELIVERED.',
      '[Charge] Drone 105 safely landed on Charging Pad 3 (Node 14) and started rapid charging (12%).'
    ],
  },

  // ── STATE 10 ─ Return to Depot & Battery Recharge ──────────────────────────
  {
    label: 'Stage 12 — Return to Depot & Battery Recharge',
    description: 'Drones 101, 102, 103 fly back to Warehouse. Drone 105 battery recharges to 65% at Pad 3.',
    drones: [
      { id: 101, battery: 55, node: 1,  status: 'RETURNING'        },
      { id: 102, battery: 55, node: 2,  status: 'RETURNING'        },
      { id: 103, battery: 22, node: 8,  status: 'RETURNING'        },
      { id: 104, battery: 86, node: 2,  status: 'IDLE'             },
      { id: 105, battery: 65, node: 14, status: 'CHARGING (65%)'   },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'PENDING'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'   },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: null, route: [5, 10, 1, 0], progress: 2, dest: 0, status: 'RETURNING' },
      { id: 502, droneId: 102, packageId: null, route: [5, 10, 2, 0], progress: 2, dest: 0, status: 'RETURNING' },
      { id: 503, droneId: 103, packageId: null, route: [5, 8, 0],    progress: 1, dest: 0, status: 'RETURNING' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: true  },
    ],
    highlightedEdges: [
      { from: 10, to: 1  },
      { from: 10, to: 2  },
      { from: 5,  to: 8  },
    ],
    events: [
      '[Return] Drones 101, 102, 103 heading back to base depot.',
      '[Charge] Drone 105 battery level: 12% → 65% (+53% charged).',
      '[Airspace] Corridor reservations released for delivery corridors.'
    ],
  },

  // ── STATE 11 ─ Next Dispatch & Heap Selection ──────────────────────────────
  {
    label: 'Stage 13 — Next Dispatch (Package #1004) & Heap Selection',
    description: 'Package 1004 popped from FIFO. Heap selects Drone 104 (86% battery). Dijkstra routes to Node 12 via 0→1→10→12.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'             },
      { id: 104, battery: 86, node: 0,  status: 'SELECTED (WINNER)' },
      { id: 105, battery: 95, node: 14, status: 'CHARGING (95%)'   },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'DISPATCHED'  },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'     },
    ],
    missions: [
      { id: 505, droneId: 104, packageId: 1004, route: [0, 1, 10, 12], progress: 0, dest: 12, status: 'ACTIVE' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: true  },
    ],
    highlightedEdges: [
      { from: 0,  to: 1  },
      { from: 1,  to: 10 },
      { from: 10, to: 12 },
    ],
    events: [
      '[Queue] FIFO popped Package 1004.',
      '[Heap] Drone 104 wins selection (battery margin: +36%).',
      '[Dijkstra] Shortest safe path: 0 → 1 → 10 → 12 (avoiding Restricted Airport Node 4).'
    ],
  },

  // ── STATE 12 ─ Tick Progression (Drone 104 Transit & Drone 105 Return) ──────
  {
    label: 'Stage 14 — Tick Progression & Pad Departure',
    description: 'Drone 104 advances to Central Hub (Node 10). Fully charged Drone 105 returns from Pad 3.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'             },
      { id: 104, battery: 68, node: 10, status: 'DELIVERING'       },
      { id: 105, battery: 88, node: 3,  status: 'RETURNING'        },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'IN_TRANSIT'  },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'     },
    ],
    missions: [
      { id: 505, droneId: 104, packageId: 1004, route: [0, 1, 10, 12], progress: 2, dest: 12, status: 'ACTIVE' },
      { id: 506, droneId: 105, packageId: null, route: [14, 3, 0],     progress: 1, dest: 0,  status: 'RETURNING' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 10, to: 12 },
      { from: 3,  to: 0  },
    ],
    events: [
      '[Tick] Drone 104: Node 1 → Central Hub (Node 10).',
      '[Charge] Drone 105 charging complete (95%). Pad 3 released.',
      '[Return] Drone 105 returns via: 14 → 3 → 0.'
    ],
  },

  // ── STATE 13 ─ Delivery at Customer Zone D & Drone 105 Home ──────────────────
  {
    label: 'Stage 15 — Delivery at Customer Zone D & Drone 105 Home',
    description: 'Drone 104 reaches Node 12 and completes dispatch. Drone 105 returns safely to Warehouse Alpha.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'             },
      { id: 104, battery: 58, node: 12, status: 'DELIVERING (LANDED)' },
      { id: 105, battery: 88, node: 0,  status: 'IDLE'             },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'DELIVERED'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'PENDING'     },
    ],
    missions: [
      { id: 505, droneId: 104, packageId: 1004, route: [0, 1, 10, 12], progress: 3, dest: 12, status: 'ARCHIVED' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: [
      '[Delivery] Drone 104 arrived at Customer Zone D (Node 12). Package 1004 DELIVERED!',
      '[Return] Drone 105 safely returned to Warehouse Alpha.',
      '[Queue] FIFO popped next package.'
    ],
  },

  // ── STATE 14 ─ Final Dispatch & Curved Path Avoidance (Package #1005) ───────
  {
    label: 'Stage 16 — Final Dispatch & Curved Airspace Avoidance',
    description: 'Final Package 1005 dispatched. Drone 105 selected. Dijkstra routes via curved 10→13 around Restricted Hospital.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'             },
      { id: 104, battery: 48, node: 1,  status: 'RETURNING'        },
      { id: 105, battery: 88, node: 0,  status: 'SELECTED (WINNER)' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'DELIVERED'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'DISPATCHED'  },
    ],
    missions: [
      { id: 505, droneId: 104, packageId: null, route: [12, 10, 1, 0], progress: 2, dest: 0,  status: 'RETURNING' },
      { id: 507, droneId: 105, packageId: 1005, route: [0, 1, 10, 13], progress: 0, dest: 13, status: 'ACTIVE' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 0,  to: 1  },
      { from: 1,  to: 10 },
      { from: 10, to: 13 },
    ],
    events: [
      '[Heap] Drone 105 wins final selection (battery: 88%).',
      '[Dijkstra] Shortest route: 0 → 1 → 10 → 13.',
      '[Safety] Curve active on [10→13] to fly safely around Restricted Hospital.'
    ],
  },

  // ── STATE 15 ─ Final Delivery & Returning Flights ─────────────────────────
  {
    label: 'Stage 17 — Final Delivery & Return Path',
    description: 'Drone 105 reaches Customer Zone E (Node 13) via the curved flight path, delivering Package 1005.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'             },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'             },
      { id: 104, battery: 48, node: 0,  status: 'IDLE'             },
      { id: 105, battery: 61, node: 13, status: 'DELIVERING (LANDED)' },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1002, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1003, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED'   },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'DELIVERED'   },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'DELIVERED'   },
    ],
    missions: [
      { id: 507, droneId: 105, packageId: 1005, route: [0, 1, 10, 13], progress: 3, dest: 13, status: 'ARCHIVED' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [
      { from: 13, to: 10 },
    ],
    events: [
      '[Delivery] Drone 105 landed at Customer Zone E (Node 13). Package 1005 DELIVERED!',
      '[Return] Drone 104 returned to Warehouse depot and is IDLE.',
      '[Return] Drone 105 returning to Warehouse via curved hospital bypass.'
    ],
  },

  // ── STATE 16 ─ Final Summary & Traffic Integrity ───────────────────────────
  {
    label: 'Stage 18 — Final Summary & Traffic Integrity',
    description: 'All packages delivered. 0 violations. 0 battery failures. Traffic integrity PASSED.',
    drones: [
      { id: 101, battery: 55, node: 0,  status: 'IDLE'     },
      { id: 102, battery: 55, node: 0,  status: 'IDLE'     },
      { id: 103, battery: 22, node: 0,  status: 'IDLE'     },
      { id: 104, battery: 48, node: 0,  status: 'IDLE'     },
      { id: 105, battery: 50, node: 0,  status: 'IDLE'     },
    ],
    packages: [
      { id: 1001, dest: 'Customer_Zone_A (Node 5)',  status: 'DELIVERED' },
      { id: 1002, dest: 'Customer_Zone_B (Node 6)',  status: 'DELIVERED' },
      { id: 1003, dest: 'Customer_Zone_C (Node 7)',  status: 'DELIVERED' },
      { id: 1004, dest: 'Customer_Zone_D (Node 12)', status: 'DELIVERED' },
      { id: 1005, dest: 'Customer_Zone_E (Node 13)', status: 'DELIVERED' },
    ],
    missions: [
      { id: 501, droneId: 101, packageId: 1001, route: [0, 1, 10, 5],  progress: 3, dest: 5,  status: 'ARCHIVED' },
      { id: 502, droneId: 102, packageId: 1002, route: [0, 2, 10, 6],  progress: 3, dest: 6,  status: 'ARCHIVED' },
      { id: 503, droneId: 103, packageId: 1003, route: [0, 3, 7],      progress: 2, dest: 7,  status: 'ARCHIVED' },
      { id: 504, droneId: 104, packageId: 1004, route: [0, 1, 10, 12], progress: 3, dest: 12, status: 'ARCHIVED' },
      { id: 505, droneId: 105, packageId: 1005, route: [0, 1, 10, 13], progress: 3, dest: 13, status: 'ARCHIVED' },
    ],
    pads: [
      { id: 1, node: 8,  name: 'Charging_Pad_1', occupied: false },
      { id: 2, node: 9,  name: 'Charging_Pad_2', occupied: false },
      { id: 3, node: 14, name: 'Charging_Pad_3', occupied: false },
    ],
    highlightedEdges: [],
    events: [
      '━━━ SIMULATION COMPLETE ━━━',
      '✔ Packages Delivered:         5',
      '✔ Restricted Zone Violations: 0',
      '✔ Battery Depletion Failures: 0',
      '✔ Traffic Integrity:          PASSED',
      '✔ Archived Missions:          5',
      '━━━━━━━━━━━━━━━━━━━━━━━━━━━━',
    ],
  },
];

export default simulationStates;
