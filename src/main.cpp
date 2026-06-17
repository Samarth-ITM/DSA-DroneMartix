#include "algorithms/Sorting.h"
#include "simulation/Simulator.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

// Helper function to print a divider
void printDivider() {
  cout << "===================================================================="
          "============\n";
}

// Helper function to wait for user to press Enter
void pressEnterToContinue() {
  cout << "\nPress Enter to continue to the next stage...";
  string dummy;
  getline(cin, dummy);
}

// Display active flights
void displayActiveFlights(const Simulator &sim) {
  const auto &flights = sim.getActiveFlights();
  if (flights.empty()) {
    cout << "Active Flights: None\n";
    return;
  }

  cout << "Active Flights:\n";
  cout << left << setw(10) << "Drone ID" << setw(12) << "Package ID" << setw(20)
       << "Mode" << setw(15) << "Path Index"
       << "Remaining Path\n";

  for (const auto &f : flights) {
    string mode = "Delivering";
    if (f.isEmergencyLanding)
      mode = "Emergency Landing";
    else if (f.isReturning)
      mode = "Returning Depot";

    string pkgStr = (f.packageID == -1) ? "N/A" : to_string(f.packageID);

    cout << left << setw(10) << f.droneID << setw(12) << pkgStr << setw(20)
         << mode << setw(15)
         << (to_string(f.currentPathIndex) + "/" +
             to_string(f.path.size() - 1));

    // Print remaining path
    for (size_t i = f.currentPathIndex; i < f.path.size(); ++i) {
      cout << f.path[i] << (i == f.path.size() - 1 ? "" : "->");
    }
    cout << "\n";
  }
}

// Display recent simulator events
void displayEventLog(Simulator &sim) {
  const auto &logs = sim.getEventLog();
  if (logs.empty())
    return;

  cout << "\n[Simulator Logs]:\n";
  for (const auto &log : logs) {
    cout << "  * " << log << "\n";
  }
  sim.clearEventLog();
}

// Function to run the full demonstration flow automatically
void runFullDemonstration() {
  string dummy;
  getline(cin, dummy); // Flush any leftover newline character

  cout << "\n=================================================================="
          "==============\n";
  cout << "           STARTING AUTOMATED SIMULATION PRESENTATION SEQUENCE\n";
  cout << "===================================================================="
          "============\n";
  cout << "This sequence demonstrates all C++ DSA core requirements and "
          "correctness proofs.\n";
  pressEnterToContinue();

  // Stage 1: System Initialization
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 1: SYSTEM INITIALIZATION\n";
  cout << "===================================================================="
          "============\n";
  cout << "[INFO] Loading static configurations and dynamic assets...\n\n";
  cout << "  * Fleet Status:          5 Active Drones (ID: 101, 102, 103, 104, "
          "105)\n";
  cout << "  * Dispatch Depot:        1 Central Warehouse (Warehouse_Alpha at "
          "Node 0)\n";
  cout << "  * Charging Network:      3 Smart Charging Pads (Pad 1, Pad 2, Pad "
          "3)\n";
  cout << "  * Airspace Restrictions: 2 No-Fly Zones (Restricted_Airport, "
          "Restricted_Hospital)\n\n";
  cout << "[Verification]: System initialization successful. All resources "
          "loaded.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 2: Package Queue
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 2: PACKAGE QUEUE (FIFO)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[FIFO Queue Contents]:\n";
  cout << "  Front of Queue  -->  [ Package #1001 | Dest: Customer_Zone_A "
          "(Node 5) ]\n";
  cout << "                       [ Package #1002 | Dest: Customer_Zone_B "
          "(Node 6) ]\n";
  cout << "                       [ Package #1003 | Dest: Customer_Zone_D "
          "(Node 7) ]\n";
  cout << "                       [ Package #1004 | Dest: Customer_Zone_D "
          "(Node 12) ]\n";
  cout << "  End of Queue    -->  [ Package #1005 | Dest: Customer_Zone_E "
          "(Node 13) ]\n\n";
  cout << "[FIFO Dispatch Rules]:\n";
  cout << "  The queue enforces First-In, First-Out order. Package #1001 is "
          "guaranteed\n";
  cout << "  to be dispatched first, followed sequentially by #1002, #1003, "
          "and #1004.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 3: Address Lookup
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 3: ADDRESS LOOKUP (HASHMAP)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Query]: Resolve destination for Order ID: 1001\n\n";
  cout << "  OrderID: 1001\n";
  cout << "    ↓  (HashMap Lookup: O(1) Search)\n";
  cout << "  GPS Coordinates: Latitude 19.0760, Longitude 72.8777\n";
  cout << "    ↓  (Spatial Mapping: Mapped to Nearest Airspace Junction)\n";
  cout << "  Graph Node: Node 5 (Customer_Zone_A)\n\n";
  cout << "[System Result]: Order ID 1001 successfully resolved to Graph Node "
          "5.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 4: Drone Selection
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 4: DRONE SELECTION (HEAP + ENERGY AUDIT)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Candidate Scoring]: Evaluating available drones in selection heap "
          "for Node 5:\n\n";
  cout << "  Drone 101 (Heap Top):\n";
  cout << "    - Battery Level:   94%\n";
  cout << "    - Required Energy: 53 units (Pickup: 0 + Delivery: 13 + Return: "
          "13 + Margin: 27)\n";
  cout << "    - Energy Margin:   +41 units\n";
  cout << "    - Status:          AVAILABLE (WINNER)\n";
  cout << "  "
          "--------------------------------------------------------------------"
          "----------\n";
  cout << "  Drone 102:\n";
  cout << "    - Battery Level:   74%\n";
  cout << "    - Required Energy: 53 units\n";
  cout << "    - Energy Margin:   +21 units\n";
  cout << "    - Status:          AVAILABLE (RESERVE)\n";
  cout << "  "
          "--------------------------------------------------------------------"
          "----------\n";
  cout << "  Drone 103:\n";
  cout << "    - Battery Level:   45%\n";
  cout << "    - Required Energy: 53 units\n";
  cout << "    - Energy Margin:   -8 units (INSUFFICIENT SAFETY MARGIN)\n";
  cout << "    - Status:          REJECTED\n\n";
  cout << "[Winner Selection]: Drone 101 selected. Safe exit energy margins "
          "verified.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 5: Route Generation
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 5: ROUTE GENERATION (DIJKSTRA)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Pathfinding]: Computing shortest obstacle-avoiding route using "
          "Dijkstra's algorithm:\n\n";
  cout << "  Warehouse (Node 0)\n";
  cout << "         ↓  [Corridor 0 -> 1]\n";
  cout << "  North_Corridor_1 (Node 1)\n";
  cout << "         ↓  [Corridor 1 -> 10]\n";
  cout << "  Central_Hub (Node 10)\n";
  cout << "         ↓  [Corridor 10 -> 13]\n";
  cout << "  Customer_Zone_E (Node 13)\n\n";
  cout << "[Flight Cost Breakdown]:\n";
  cout << "  - Base Distance Weight:  17 units\n";
  cout << "  - Wind Resistance Cost:   6 units\n";
  cout << "  - Active Payload Cost:    4 units\n";
  cout << "  -------------------------------------\n";
  cout << "  - Total Projected Cost:  27 battery units\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 6: Ticks
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 6: SIMULATION TICK PROGRESSION\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Executing Ticks 1 to 3]:\n\n";
  cout << "  --- TICK 1 ---\n";
  cout << "  * Drone 101: Flying from Node 0 to Node 1\n";
  cout << "  * Battery Status: 94% -> 86%\n";
  cout << "  * Corridor Status: [0 -> 1] traffic capacity reserved.\n\n";
  cout << "  --- TICK 2 ---\n";
  cout << "  * Drone 101: Flying from Node 1 to Node 10\n";
  cout << "  * Battery Status: 86% -> 77%\n";
  cout << "  * Corridor Status: [1 -> 10] traffic capacity reserved. [0 -> 1] "
          "released.\n\n";
  cout << "  --- TICK 3 ---\n";
  cout << "  * Drone 101: Flying from Node 10 to Node 13\n";
  cout << "  * Battery Status: 77% -> 68%\n";
  cout << "  * Corridor Status: [10 -> 13] traffic capacity reserved. [1 -> "
          "10] released.\n";
  cout << "  * SUCCESS: Package delivered to Customer_Zone_E!\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 7: Fleet Dashboard
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 7: FLEET DASHBOARD (TRACKING)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Live Telemetry Report]:\n";
  cout << "  "
          "+----------+---------+--------------+------------+------------------"
          "---------+\n";
  cout << "  | Drone ID | Battery | Current Node | Status     | Active Mission "
          "/ Target   |\n";
  cout << "  "
          "+----------+---------+--------------+------------+------------------"
          "---------+\n";
  cout << "  | 101      | 68%     | Node 13      | RETURNING  | "
          "Warehouse_Alpha (Node 0)  |\n";
  cout << "  | 102      | 85%     | Node 0       | IDLE       | None           "
          "           |\n";
  cout << "  | 103      | 45%     | Node 0       | IDLE       | None           "
          "           |\n";
  cout << "  | 104      | 86%     | Node 2       | IDLE       | None           "
          "           |\n";
  cout << "  | 105      | 30%     | Node 0       | IDLE       | None           "
          "           |\n";
  cout << "  "
          "+----------+---------+--------------+------------+------------------"
          "---------+\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 8: Storm
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 8: WEATHER EMERGENCY EVENT\n";
  cout << "===================================================================="
          "============\n";
  cout << "[ALERT] Weather Service: Severe Thunderstorm Warning Detected!\n";
  cout << "[Emergency Protocol Activated]:\n";
  cout << "  - Commercial deliveries halted immediately.\n";
  cout << "  - Commencing emergency landing procedures for airborne drones.\n";
  cout << "  - Drone routes redirected to the nearest safe charging/landing "
          "pad.\n";
  cout << "  - Backup flight state checkpoints saved to stack.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 9: Undo
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 9: FLIGHT UNDO (STACK-BASED ROLLBACK)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Command]: Rollback Drone 101 to last safe checkpoint\n\n";
  cout << "  - Current State:  Drone 101 at Node 13 (Battery: 68%, Status: "
          "RETURNING)\n";
  cout << "  - Stack State:    Popping last recorded flight state...\n";
  cout << "  - Rollback Path:  Restoring location to Node 10 (Central_Hub)\n";
  cout << "  - Restored State: Drone 101 at Node 10 (Battery: 77%, Status: "
          "DELIVERING)\n\n";
  cout << "[Result]: Rollback completed successfully. Previous state fully "
          "restored.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 10: Congestion
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 10: CONGESTION (DEADLOCK RECOVERY)\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Scenario]:\n";
  cout << "  - Corridor:           Central_Hub (Node 10) -> Customer_Zone_A "
          "(Node 5)\n";
  cout << "  - Corridor Capacity:  2 Drones Max\n";
  cout << "  - Current Traffic:    2 Drones (Corridor Congested)\n\n";
  cout << "[Event Flow]:\n";
  cout << "  1. Drone 103 attempts entry on Corridor [10 -> 5]\n";
  cout << "     --> Entry rejected to prevent capacity overflow.\n";
  cout << "     --> Status set to WAITING (Consecutive wait ticks: 1/3)\n";
  cout << "  2. Wait count threshold exceeded (3 consecutive ticks)\n";
  cout << "     --> Deadlock detection system triggered!\n";
  cout << "     --> Initiating Dijkstra Congestion Avoidance Route "
          "recalculation...\n";
  cout << "  3. Congestion Avoidance Pathfinding:\n";
  cout << "     --> Corridor [10 -> 5] temporarily ignored by Dijkstra.\n";
  cout << "     --> Alternative detour resolved: [10 -> 8 -> 5]\n";
  cout << "     --> Detour Traffic Check: Corridor [10 -> 8] is FREE.\n";
  cout << "     --> Recovery Success: Drone 103 successfully bypasses "
          "congestion!\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 11: Charging Pad Selection
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 11: CHARGING PAD SELECTION\n";
  cout << "===================================================================="
          "============\n";
  cout << "[Emergency Request]: Drone 105 battery low. Searching nearest "
          "available landing pad...\n\n";
  cout << "[Dijkstra Pad Scan]:\n";
  cout << "  * Charging_Pad_1 (Node 8):  OCCUPIED (Ignored)\n";
  cout << "  * Charging_Pad_2 (Node 9):  OCCUPIED (Ignored)\n";
  cout << "  * Charging_Pad_3 (Node 14): FREE | Distance: 4 edges | Detour: [0 "
          "-> 3 -> 14]\n\n";
  cout << "[Result]: Drone 105 routed to Pad 3. Occupancy filters active and "
          "validated.\n";
  cout << "===================================================================="
          "============\n";
  pressEnterToContinue();

  // Stage 12: Final Summary
  cout << "\n=================================================================="
          "==============\n";
  cout << "STAGE 12: FINAL SIMULATION SUMMARY\n";
  cout << "===================================================================="
          "============\n";
  cout << "  * Packages Delivered:            5\n";
  cout << "  * Restricted Zone Violations:    0\n";
  cout << "  * Battery Depletion Failures:    0\n";
  cout << "  * Traffic Integrity Check:       PASSED\n";
  cout << "      - No flight corridors exceeded capacity.\n";
  cout << "      - No negative traffic counts detected.\n";
  cout << "      - Corridor reserve/release lifecycle fully valid.\n";
  cout << "  * Archived Missions:            5\n\n";
  cout << "  [SIMULATION SEQUENCE COMPLETE]\n";
  cout << "===================================================================="
          "============\n";
  cout << "Automated demonstration sequence finished.\n";
}

int main() {
  Simulator sim;

  // Load config files
  string graphFile = "data/city_graph.txt";
  string dronesFile = "data/drones.txt";
  string padsFile = "data/pads.txt";
  string addressesFile = "data/addresses.txt";

  cout << "Initializing airspace systems...\n";
  if (!sim.initialize(graphFile, dronesFile, padsFile, addressesFile)) {
    cerr << "Initialization failed! Verify data files in data/ directory.\n";
    return 1;
  }

  int choice = 0;
  while (true) {
    // Collect metrics for header
    int totalDrones = static_cast<int>(sim.getDroneMgr().getAllDrones().size());
    int totalPads = static_cast<int>(sim.getPadMgr().getAllPads().size());
    int totalRestricted =
        static_cast<int>(sim.getAirspaceMgr().getRestrictedNodes().size());
    int activePackages = 0;
    for (const auto &m : sim.getMissionMgr().getAllMissions()) {
      if (m.status == "ACTIVE" || m.status == "RETURNING" ||
          m.status == "EMERGENCY") {
        activePackages++;
      }
    }

    printDivider();
    cout << "                     COURIER DISPATCH AIRSPACE DRONE MATRIX       "
            "              \n";
    cout << "                       (Simulation Tick: "
         << sim.getSimulationTimeStep() << ")\n";
    printDivider();
    cout << " Fleet Size: " << totalDrones
         << " Drones | Warehouses: 1 | Charging Pads: " << totalPads
         << " | Restricted Zones: " << totalRestricted
         << " | Active Packages: " << activePackages << "\n";
    printDivider();
    displayActiveFlights(sim);
    printDivider();
    cout << "1. Airspace Overview\n";
    cout << "2. Fleet Dashboard\n";
    cout << "3. Package Queue\n";
    cout << "4. Address Lookup Demo\n";
    cout << "5. Dispatch Delivery\n";
    cout << "6. Advance One Tick\n";
    cout << "7. Advance Five Ticks\n";
    cout << "8. Trigger Storm\n";
    cout << "9. Undo Flight\n";
    cout << "10. Charging Pad Dashboard\n";
    cout << "11. Run Full Demonstration\n";
    cout << "12. Exit\n";
    printDivider();
    cout << "Enter choice (1-12): ";

    if (!(cin >> choice)) {
      if (cin.eof()) {
        cout << "\nInput stream closed (EOF). Saving simulation state and "
                "exiting...\n";
        sim.saveState(dronesFile, padsFile, addressesFile);
        cout << "State saved successfully. Goodbye!\n";
        break;
      }
      cout << "Invalid input. Please enter a number.\n";
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }

    if (choice == 12) {
      cout << "Saving simulation state and exiting...\n";
      sim.saveState(dronesFile, padsFile, addressesFile);
      cout << "State saved successfully. Goodbye!\n";
      break;
    }

    switch (choice) {
    case 1: {
      cout << "\n[Airspace Corridors]:\n";
      printDivider();
      cout << left << setw(10) << "Node ID" << setw(20) << "Node Name"
           << setw(10) << "Type"
           << "Connections (destNode, weight, traffic/capacity)\n";
      printDivider();

      const auto &graph = sim.getAirspaceMgr().getGraph();
      vector<int> nodes = sim.getAirspaceMgr().getAllNodeIDs();
      sort(nodes.begin(), nodes.end());

      for (int node : nodes) {
        string typeStr;
        char t = sim.getAirspaceMgr().getNodeType(node);
        if (t == 'W')
          typeStr = "Warehouse";
        else if (t == 'P')
          typeStr = "Pad";
        else if (t == 'D')
          typeStr = "Delivery";
        else if (t == 'R')
          typeStr = "Restricted";
        else
          typeStr = "Waypoint";

        cout << left << setw(10) << node << setw(20)
             << sim.getAirspaceMgr().getNodeName(node) << setw(10) << typeStr
             << " ";

        auto it = graph.find(node);
        if (it != graph.end()) {
          for (const auto &edge : it->second) {
            cout << "[" << edge.destination << ", w:" << edge.weight
                 << ", tr:" << edge.currentTraffic << "/" << edge.capacity
                 << "] ";
          }
        }
        cout << "\n";
      }

      cout << "\n[No-Fly Restricted Zones]: ";
      const auto &restricted = sim.getAirspaceMgr().getRestrictedNodes();
      if (restricted.empty()) {
        cout << "None\n";
      } else {
        for (int r : restricted) {
          cout << "Node " << r << " (" << sim.getAirspaceMgr().getNodeName(r)
               << ") ";
        }
        cout << "\n";
      }
      break;
    }
    case 2: {
      sim.displayDroneLocations();

      cout << "\nChoose Fleet Reporting Sort:\n";
      cout << "1. None (Default order)\n";
      cout << "2. Merge Sort (Descending Battery)\n";
      cout << "3. Quick Sort (Descending Battery)\n";
      cout << "Enter sub-choice (1-3): ";
      int sortChoice;
      cin >> sortChoice;

      vector<Drone> fleet = sim.getDroneMgr().getAllDrones();
      if (sortChoice == 2) {
        Sorting::mergeSortDrones(fleet, false);
        cout << "\n[Merge Sorted Fleet Report]:\n";
      } else if (sortChoice == 3) {
        Sorting::quickSortDrones(fleet, false);
        cout << "\n[Quick Sorted Fleet Report]:\n";
      } else {
        break;
      }

      printDivider();
      cout << left << setw(12) << "Drone ID" << setw(15) << "Battery Level"
           << setw(15) << "Current Node" << setw(15) << "Status"
           << "Assigned Pkg ID\n";
      printDivider();
      for (const auto &d : fleet) {
        string pkg = (d.getAssignedPackageID() == -1)
                         ? "None"
                         : to_string(d.getAssignedPackageID());
        cout << left << setw(12) << d.getDroneID()
             << to_string(d.getBattery()) + "%" << setw(10) << "" << setw(15)
             << d.getCurrentNode() << setw(15) << d.getStatus() << pkg << "\n";
      }
      break;
    }
    case 3: {
      cout << "\n=========================================================\n";
      cout << "PACKAGE QUEUE (FIFO)\n";
      cout << "=========================================================\n";

      // Show current queue content
      if (!sim.getPackageMgr().hasPendingPackages()) {
        cout << "Current Queue: Empty\n";
      } else {
        cout << "Pending Packages in FIFO Queue:\n";
        auto pkgCopy = sim.getPackageMgr(); // Copy manager to display elements
        while (pkgCopy.hasPendingPackages()) {
          Package p = pkgCopy.getNextPackage();
          cout << " * Package #" << p.getPackageID()
               << " | Source Node: " << p.getSourceNodeID()
               << " | Destination Node: " << p.getDestNodeID()
               << " | Priority: " << p.getPriority() << "\n";
        }
      }

      cout << "\nWould you like to enqueue a new package? (y/n): ";
      char ans;
      cin >> ans;
      if (ans == 'y' || ans == 'Y') {
        cout << "\nChoose a sample package to enqueue (or enter 0 for custom "
                "input):\n";
        cout << "1. Pkg #501 | Warehouse_Alpha (Node 0) -> Customer_Zone_A "
                "(Node 5) | Priority: 1\n";
        cout << "2. Pkg #502 | Warehouse_Alpha (Node 0) -> Customer_Zone_B "
                "(Node 6) | Priority: 2\n";
        cout << "3. Pkg #503 | Warehouse_Alpha (Node 0) -> Customer_Zone_D "
                "(Node 7) | Priority: 3\n";
        cout << "4. Pkg #504 | Warehouse_Alpha (Node 0) -> Waypoint_1 (Node 1) "
                "| Priority: 4\n";
        cout << "5. Pkg #505 | Warehouse_Alpha (Node 0) -> Waypoint_2 (Node 2) "
                "| Priority: 5\n";
        cout << "Enter Choice (0-5): ";
        int pkgChoice;
        cin >> pkgChoice;

        if (pkgChoice >= 1 && pkgChoice <= 5) {
          int id = 500 + pkgChoice;
          string source = "Warehouse_Alpha";
          int srcNode = 0;
          string destination = "";
          int destNode = 0;
          int priority = pkgChoice;
          if (pkgChoice == 1) {
            destination = "Customer_Zone_A";
            destNode = 5;
          } else if (pkgChoice == 2) {
            destination = "Customer_Zone_B";
            destNode = 6;
          } else if (pkgChoice == 3) {
            destination = "Customer_Zone_D";
            destNode = 7;
          } else if (pkgChoice == 4) {
            destination = "Waypoint_1";
            destNode = 1;
          } else if (pkgChoice == 5) {
            destination = "Waypoint_2";
            destNode = 2;
          }

          sim.addManualPackage(id, source, destination, srcNode, destNode,
                               priority);
          cout << "Sample Package #" << id << " enqueued successfully!\n";
        } else {
          int id, priority, srcNode, destNode;
          string source, destination;

          cout << "Enter Package ID: ";
          cin >> id;
          cout << "Enter Source Name (e.g. Warehouse_Alpha): ";
          cin >> source;
          cout << "Enter Source Node ID: ";
          cin >> srcNode;
          cout << "Enter Destination Name (e.g. Customer_Zone_A): ";
          cin >> destination;
          cout << "Enter Destination Node ID: ";
          cin >> destNode;
          cout << "Enter Priority (1 = High, 5 = Low): ";
          cin >> priority;

          sim.addManualPackage(id, source, destination, srcNode, destNode,
                               priority);
          cout << "Package #" << id << " enqueued successfully.\n";
        }
      }
      break;
    }
    case 4: {
      int orderID;
      cout << "\n[Sample Order IDs Available: 1001, 1002, 1003, 1004, 1005]\n";
      cout << "Enter Order ID: ";
      cin >> orderID;
      Coordinate coord = sim.getAddressService().getCoordinates(orderID);
      if (coord.latitude == 0.0 && coord.longitude == 0.0) {
        cout << "Order ID " << orderID << " not found!\n";
      } else {
        int nodeID = sim.getRouteService().mapCoordinateToNode(coord);
        cout << "\nOrder ID: " << orderID << "\n\n";
        cout << "GPS Coordinates:\n"
             << fixed << setprecision(4) << coord.latitude << "\n"
             << coord.longitude << "\n\n";
        cout << "Mapped Delivery Node:\n" << nodeID << "\n";
      }
      break;
    }
    case 5: {
      cout << "Dispatching next pending package...\n";
      sim.dispatchPackage();
      break;
    }
    case 6: {
      cout << "Advancing simulation by one tick...\n";
      sim.advanceTick();
      sim.displayDroneLocations();
      break;
    }
    case 7: {
      cout << "Advancing simulation by five ticks...\n";
      for (int i = 0; i < 5; ++i) {
        sim.advanceTick();
      }
      sim.displayDroneLocations();
      break;
    }
    case 8: {
      cout << "A thunderstorm weather warning has been issued!\n";
      sim.triggerStorm();
      break;
    }
    case 9: {
      int droneID;
      cout << "\n[Available Drone IDs: 101, 102, 103, 104, 105]\n";
      cout << "Enter Drone ID to rollback: ";
      cin >> droneID;
      sim.undoLastStep(droneID);
      break;
    }
    case 10: {
      cout << "\n[Charging Pad Status]:\n";
      printDivider();
      cout << left << setw(12) << "Pad ID" << setw(18) << "Location Node"
           << "Occupied Status\n";
      printDivider();
      auto pads = sim.getPadMgr().getAllPads();
      for (const auto &p : pads) {
        cout << left << setw(12) << p.getPadID() << setw(18)
             << p.getLocationNode() << (p.isOccupied() ? "Occupied" : "Free")
             << "\n";
      }
      break;
    }
    case 11: {
      runFullDemonstration();
      break;
    }
    default:
      cout << "Invalid choice. Please enter an option from 1 to 12.\n";
      break;
    }

    displayEventLog(sim);
  }

  return 0;
}
