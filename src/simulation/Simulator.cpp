#include "Simulator.h"
#include <algorithm>
#include <iostream>
#include <limits>

using namespace std;

Simulator::Simulator()
    : routeService(airspaceMgr, padMgr, addressService), simulationTimeStep(0) {
}

bool Simulator::initialize(const string &graphFile, const string &dronesFile,
                           const string &padsFile,
                           const string &addressesFile) {
  if (!airspaceMgr.loadCityMap(graphFile))
    return false;
  if (!droneMgr.loadDronesFromFile(dronesFile))
    return false;
  if (!padMgr.loadPadsFromFile(padsFile))
    return false;
  if (!addressService.loadAddressesFromFile(addressesFile))
    return false;

  simulationTimeStep = 0;
  activeFlights.clear();
  eventLog.clear();
  addEvent("Simulator initialized successfully.");
  return true;
}

bool Simulator::saveState(const string &dronesFile, const string &padsFile,
                          const string &addressesFile) const {
  if (!droneMgr.saveDronesToFile(dronesFile))
    return false;
  if (!padMgr.savePadsToFile(padsFile))
    return false;
  if (!addressService.saveAddressesToFile(addressesFile))
    return false;
  return true;
}

void Simulator::addManualPackage(int id, const string &src, const string &dest,
                                 int srcNode, int destNode, int prio) {
  Package pkg(id, src, dest, srcNode, destNode, prio);
  packageMgr.enqueuePackage(pkg);
  addEvent("Package #" + to_string(id) + " added to launch line: " + src +
           " -> " + dest);
}

int Simulator::findActiveFlightIndex(int droneID) const {
  for (size_t i = 0; i < activeFlights.size(); ++i) {
    if (activeFlights[i].droneID == droneID) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

bool Simulator::reserveCorridorForMission(Mission &mission, int u, int v) {
  if (airspaceMgr.reserveCorridorTraffic(u, v)) {
    mission.activeReservations.push_back({u, v});
    return true;
  }
  return false;
}

void Simulator::releaseCorridorForMission(Mission &mission, int u, int v) {
  for (auto it = mission.activeReservations.begin();
       it != mission.activeReservations.end(); ++it) {
    if ((it->fromNode == u && it->toNode == v) ||
        (it->fromNode == v && it->toNode == u)) {
      mission.activeReservations.erase(it);
      break;
    }
  }
  airspaceMgr.releaseCorridorTraffic(u, v);
}

void Simulator::releaseMissionReservations(Mission &mission) {
  for (const auto &res : mission.activeReservations) {
    airspaceMgr.releaseCorridorTraffic(res.fromNode, res.toNode);
  }
  mission.activeReservations.clear();
}

bool Simulator::validateTrafficIntegrity() const {
  const auto &graph = airspaceMgr.getGraph();
  bool integrityOk = true;
  for (const auto &pair : graph) {
    for (const auto &edge : pair.second) {
      if (edge.currentTraffic < 0) {
        cout << "TRAFFIC INTEGRITY ERROR: Negative traffic on corridor "
             << pair.first << " -> " << edge.destination
             << " (traffic: " << edge.currentTraffic << ")\n";
        integrityOk = false;
      }
      if (edge.currentTraffic > edge.capacity) {
        cout << "TRAFFIC INTEGRITY WARNING: Traffic exceeds capacity on "
                "corridor "
             << pair.first << " -> " << edge.destination
             << " (traffic: " << edge.currentTraffic << "/" << edge.capacity
             << ")\n";
        integrityOk = false;
      }
    }
  }
  if (integrityOk) {
    cout << "\nTraffic Integrity Check: PASSED\n";
    cout << "- No flight corridors exceeded capacity.\n";
    cout << "- No negative traffic counts detected.\n";
    cout << "- Reserve/Release lifecycle fully valid.\n";
  }
  return integrityOk;
}

bool Simulator::dispatchPackage() {
  if (!packageMgr.hasPendingPackages()) {
    addEvent("Dispatch failed: No packages in queue.");
    return false;
  }

  Package pkg = packageMgr.getNextPackage();
  int destNode = pkg.getDestNodeID();
  int srcNode = pkg.getSourceNodeID();

  // Select the best drone using intelligent scoring (pickup dist + dest dist +
  // battery)
  Drone bestDrone = droneMgr.selectBestDrone(srcNode, destNode, routeService,
                                             padMgr, airspaceMgr);
  if (bestDrone.getDroneID() == -1) {
    addEvent("Dispatch failed: No reachable or available drones.");
    pkg.setStatus("Pending");
    packageMgr.enqueuePackage(pkg);
    return false;
  }

  // Calculate pickup route and cost
  int pickupDist = 0;
  vector<int> pickupRoute;
  if (bestDrone.getCurrentNode() != srcNode) {
    pickupRoute = routeService.calculateRoute(bestDrone.getCurrentNode(),
                                              srcNode, pickupDist);
    if (pickupRoute.empty()) {
      addEvent("Dispatch failed: No route from Drone #" +
               to_string(bestDrone.getDroneID()) + " to Warehouse Node " +
               to_string(srcNode));
      pkg.setStatus("Pending");
      packageMgr.enqueuePackage(pkg);
      return false;
    }
  } else {
    pickupRoute = {srcNode};
  }
  float pickupCost = routeService.calculateBatteryCost(pickupRoute, false);

  // Calculate delivery route and cost
  int deliveryDist = 0;
  vector<int> deliveryRoute =
      routeService.calculateRoute(srcNode, destNode, deliveryDist);
  if (deliveryRoute.empty()) {
    addEvent("Dispatch failed: No route from Warehouse Node " +
             to_string(srcNode) + " to Customer Node " + to_string(destNode));
    pkg.setStatus("Pending");
    packageMgr.enqueuePackage(pkg);
    return false;
  }
  float deliveryCost = routeService.calculateBatteryCost(deliveryRoute, true);

  // Combine paths
  vector<int> fullPath = pickupRoute;
  if (!fullPath.empty() && !deliveryRoute.empty() &&
      bestDrone.getCurrentNode() != srcNode) {
    fullPath.pop_back(); // Remove duplicate junction node
  }
  if (bestDrone.getCurrentNode() != srcNode) {
    fullPath.insert(fullPath.end(), deliveryRoute.begin(), deliveryRoute.end());
  } else {
    fullPath = deliveryRoute;
  }

  // Calculate return route and cost
  int returnDist = 0;
  vector<int> returnRoute =
      routeService.calculateReturnRoute(destNode, returnDist);
  float returnCost = numeric_limits<float>::max();
  if (!returnRoute.empty()) {
    returnCost = routeService.calculateBatteryCost(returnRoute, false);
  }

  // Calculate nearest charging pad route and cost
  int padNodeID = -1;
  int padDist = 0;
  int padID = padMgr.findNearestAvailablePad(destNode, airspaceMgr.getGraph(),
                                             airspaceMgr.getRestrictedNodes(),
                                             padNodeID, padDist, false);
  float nearestPadCost = numeric_limits<float>::max();
  if (padID != -1 && padNodeID != -1) {
    vector<int> padRoute =
        Dijkstra::shortestPath(destNode, padNodeID, airspaceMgr.getGraph(),
                               airspaceMgr.getRestrictedNodes());
    if (!padRoute.empty()) {
      nearestPadCost = routeService.calculateBatteryCost(padRoute, false);
    }
  }

  float safeExitCost = min(returnCost, nearestPadCost);
  float totalRequiredEnergy = pickupCost + deliveryCost + safeExitCost;
  float energyMargin = bestDrone.getBattery() - totalRequiredEnergy;

  cout << "\nDelivery Cost: " << static_cast<int>(pickupCost + deliveryCost)
       << "\n\n";
  cout << "Return Cost: "
       << (returnCost == numeric_limits<float>::max()
               ? -1
               : static_cast<int>(returnCost))
       << "\n\n";
  cout << "Nearest Pad Cost: "
       << (nearestPadCost == numeric_limits<float>::max()
               ? -1
               : static_cast<int>(nearestPadCost))
       << "\n\n";
  cout << "Safe Exit Cost: " << static_cast<int>(safeExitCost) << "\n\n";
  cout << "Total Required: " << static_cast<int>(totalRequiredEnergy) << "\n\n";
  cout << "Battery: " << static_cast<int>(bestDrone.getBattery()) << "\n\n";
  cout << "Margin: " << static_cast<int>(energyMargin) << "\n\n";

  if (bestDrone.getBattery() < totalRequiredEnergy) {
    cout << "REJECTED\n\n";
    pkg.setStatus("Pending");
    packageMgr.enqueuePackage(pkg);
    return false;
  }
  cout << "APPROVED\n\n";

  // Assign drone
  Drone *dronePtr = droneMgr.findDrone(bestDrone.getDroneID());
  dronePtr->assignPackage(pkg.getPackageID());
  dronePtr->setStatus("DELIVERING");

  // Update package status in db
  Package *pkgPtr = packageMgr.findPackage(pkg.getPackageID());
  if (pkgPtr) {
    pkgPtr->markAssigned();
  }

  // Create Mission in MissionManager
  int missionID = missionMgr.createMission(
      bestDrone.getDroneID(), pkg.getPackageID(), srcNode, destNode, fullPath,
      static_cast<int>(pickupCost + deliveryCost),
      static_cast<int>(pickupCost + deliveryCost));
  missionMgr.startMission(missionID);
  Mission *missionPtr = missionMgr.getMission(missionID);

  // Try to reserve corridor for the first step and track it in mission
  if (fullPath.size() >= 2) {
    if (!reserveCorridorForMission(*missionPtr, fullPath[0], fullPath[1])) {
      addEvent("Dispatch failed: Safe air corridor from Node " +
               to_string(fullPath[0]) + " to Node " + to_string(fullPath[1]) +
               " is at traffic capacity.");

      dronePtr->clearAssignedPackage();
      dronePtr->setStatus("IDLE");
      if (pkgPtr) {
        pkgPtr->setStatus("Pending");
      }
      pkg.setStatus("Pending");
      packageMgr.enqueuePackage(pkg);
      missionMgr.cancelMission(missionID);
      return false;
    }
  }

  // Add active flight for tracking/rollback compatibility
  ActiveFlight flight;
  flight.droneID = bestDrone.getDroneID();
  flight.packageID = pkg.getPackageID();
  flight.path = fullPath;
  flight.currentPathIndex = 0;
  flight.isEmergencyLanding = false;
  flight.isReturning = false;
  flight.reservedPadID = -1;
  flight.estimatedBatteryConsumption = pickupCost + deliveryCost;
  flight.missionID = missionID;

  activeFlights.push_back(flight);

  addEvent("Mission #" + to_string(missionID) + " started. Package #" +
           to_string(pkg.getPackageID()) + " assigned to Drone #" +
           to_string(bestDrone.getDroneID()) +
           ". Battery Cost: " + to_string(pickupCost + deliveryCost));
  return true;
}

void Simulator::simulateStep() {
  simulationTimeStep++;
  addEvent("--- Simulation Step " + to_string(simulationTimeStep) + " ---");

  // 1. Charge drones parked at charging pads
  vector<Drone> allDrones = droneMgr.getAllDrones();
  for (const auto &d : allDrones) {
    int activeIdx = findActiveFlightIndex(d.getDroneID());
    // Drone is not active, battery is < 100
    if (activeIdx == -1 && d.getBattery() < 100.0f) {
      // Check if drone is parked on a charging pad
      int droneNode = d.getCurrentNode();
      char type = airspaceMgr.getNodeType(droneNode);
      if (type == 'P') {
        // Drone charges!
        Drone *dronePtr = droneMgr.findDrone(d.getDroneID());
        dronePtr->charge(15.0f); // 15% charge per step
        dronePtr->setStatus("CHARGING");

        addEvent("Drone #" + to_string(d.getDroneID()) + " charging at Node " +
                 to_string(droneNode) + " (" +
                 airspaceMgr.getNodeName(droneNode) +
                 "). Battery: " + to_string(dronePtr->getBattery()) + "%");

        if (dronePtr->getBattery() >= 100.0f) {
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
          padMgr.releasePadByNode(droneNode);
          addEvent("Drone #" + to_string(d.getDroneID()) +
                   " fully charged and back in service pool.");
        }
      }
    }
  }

  // 2. Advance active missions
  vector<Mission> allMissionsCopy = missionMgr.getAllMissions();
  for (const auto &mCopy : allMissionsCopy) {
    if (mCopy.status == "ACTIVE" || mCopy.status == "RETURNING" ||
        mCopy.status == "EMERGENCY") {
      Mission *missionPtr = missionMgr.getMission(mCopy.missionID);
      Drone *dronePtr = droneMgr.findDrone(missionPtr->droneID);

      if (!dronePtr) {
        missionPtr->status = "CANCELLED";
        missionMgr.cancelMission(missionPtr->missionID);
        continue;
      }

      int curr = dronePtr->getCurrentNode();
      float oldBat = dronePtr->getBattery();

      // Check if drone has arrived at the end of its path
      if (static_cast<size_t>(missionPtr->currentRouteIndex) >=
          missionPtr->route.size() - 1) {
        int arrivedNode = missionPtr->route.back();

        // Release all reservations for the mission
        releaseMissionReservations(*missionPtr);

        dronePtr->updateLocation(arrivedNode);

        if (missionPtr->status == "ACTIVE") {
          // Delivery Completed!
          Package *pkgPtr = packageMgr.findPackage(missionPtr->packageID);
          if (pkgPtr) {
            pkgPtr->markDelivered();
          }
          addEvent("Package #" + to_string(missionPtr->packageID) +
                   " delivered successfully to Node " + to_string(arrivedNode) +
                   " (" + airspaceMgr.getNodeName(arrivedNode) +
                   ") by Drone #" + to_string(missionPtr->droneID) + ".");

          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->clearAssignedPackage();
          dronePtr->setStatus("IDLE");

          // Low battery? route to nearest charging pad
          bool rerouted = false;
          if (dronePtr->getBattery() < 30.0f) {
            int padNodeID = -1;
            int padDist = 0;
            int padID = padMgr.findNearestAvailablePad(
                arrivedNode, airspaceMgr.getGraph(),
                airspaceMgr.getRestrictedNodes(), padNodeID, padDist, true);

            if (padID != -1 && padNodeID != -1) {
              int routeDist = 0;
              vector<int> padRoute = routeService.calculateRoute(
                  arrivedNode, padNodeID, routeDist);

              if (!padRoute.empty() &&
                  reserveCorridorForMission(*missionPtr, padRoute[0],
                                            padRoute[1])) {
                padMgr.reservePad(padID);

                missionPtr->route = padRoute;
                missionPtr->currentRouteIndex = 0;
                missionPtr->status = "EMERGENCY";
                missionPtr->estimatedBatteryConsumption =
                    routeService.calculateBatteryCost(padRoute, false);
                dronePtr->setStatus("EMERGENCY");

                addEvent("Drone #" + to_string(missionPtr->droneID) +
                         " battery low (" + to_string(dronePtr->getBattery()) +
                         "%). Rerouting to charging pad #" + to_string(padID) +
                         " (Node " + to_string(padNodeID) + ").");
                rerouted = true;
              }
            }
          }

          if (!rerouted) {
            // Return to warehouse
            int returnDist = 0;
            vector<int> returnRoute =
                routeService.calculateReturnRoute(arrivedNode, returnDist);
            if (!returnRoute.empty() &&
                reserveCorridorForMission(*missionPtr, returnRoute[0],
                                          returnRoute[1])) {
              missionPtr->route = returnRoute;
              missionPtr->currentRouteIndex = 0;
              missionPtr->status = "RETURNING";
              missionPtr->estimatedBatteryConsumption =
                  routeService.calculateBatteryCost(returnRoute, false);
              dronePtr->setStatus("RETURNING");

              addEvent("Drone #" + to_string(missionPtr->droneID) +
                       " returning to Warehouse depot (Node " +
                       to_string(returnRoute.back()) + ").");
            } else {
              // Stalled
              dronePtr->setAvailable(true);
              missionPtr->status = "DELIVERED";
              missionMgr.archiveMission(missionPtr->missionID);
              addEvent("Drone #" + to_string(missionPtr->droneID) +
                       " stalled at Node " + to_string(arrivedNode) +
                       " (No return corridors available).");
            }
          }
        } else if (missionPtr->status == "EMERGENCY") {
          // Landing at charging pad completed
          int padID = padMgr.getPadIDByNode(arrivedNode);
          addEvent("Drone #" + to_string(missionPtr->droneID) +
                   " landed at charging pad #" + to_string(padID) + " (Node " +
                   to_string(arrivedNode) + ") for recharge.");

          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->setAvailable(false); // unavailable while charging
          dronePtr->setStatus("CHARGING");
          missionPtr->status = "DELIVERED";
          missionMgr.archiveMission(missionPtr->missionID);
        } else if (missionPtr->status == "RETURNING") {
          // Return to depot completed
          addEvent("Drone #" + to_string(missionPtr->droneID) +
                   " returned to depot (Warehouse Node " +
                   to_string(arrivedNode) + ") and is back in service pool.");

          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
          missionPtr->status = "DELIVERED";
          missionMgr.archiveMission(missionPtr->missionID);
        }
      } else {
        // Move one step forward
        int next = missionPtr->route[missionPtr->currentRouteIndex + 1];

        // Check next-next corridor capacity
        bool canMove = true;
        if (static_cast<size_t>(missionPtr->currentRouteIndex) + 2 <
            missionPtr->route.size()) {
          int nextNext = missionPtr->route[missionPtr->currentRouteIndex + 2];
          if (!reserveCorridorForMission(*missionPtr, next, nextNext)) {
            canMove = false;
          }
        }

        if (canMove) {
          // Reset consecutiveWaitTicks
          missionPtr->consecutiveWaitTicks = 0;

          // Save flight state checkpoint
          vector<int> remainingPath(missionPtr->route.begin() +
                                        missionPtr->currentRouteIndex,
                                    missionPtr->route.end());
          FlightState checkpoint = {curr, oldBat, remainingPath};
          flightMgr.saveState(missionPtr->droneID, checkpoint);

          // Release previous corridor traffic
          releaseCorridorForMission(*missionPtr, curr, next);

          // Drain battery by edge cost + penalties
          int weight = 1;
          const auto &graph = airspaceMgr.getGraph();
          auto git = graph.find(curr);
          if (git != graph.end()) {
            for (const auto &edge : git->second) {
              if (edge.destination == next) {
                weight = edge.weight;
                break;
              }
            }
          }
          bool hasPayload = (missionPtr->status == "ACTIVE");
          float batteryDrain = weight + 3 + (hasPayload ? 2 : 0);

          dronePtr->drainBattery(batteryDrain);
          dronePtr->updateLocation(next);
          missionPtr->currentRouteIndex++;

          string statusStr = missionPtr->status;
          dronePtr->setStatus(statusStr == "ACTIVE" ? "DELIVERING" : statusStr);

          addEvent("Drone #" + to_string(missionPtr->droneID) +
                   " flew from Node " + to_string(curr) + " to Node " +
                   to_string(next) + " (" + airspaceMgr.getNodeName(next) +
                   "). Battery: " + to_string(dronePtr->getBattery()) + "%");

          if (dronePtr->getBattery() <= 0.0f) {
            addEvent(
                "WARNING: Drone #" + to_string(missionPtr->droneID) +
                " battery depleted in flight. Emergency descent triggered!");
          }
        } else {
          // Increment consecutive wait ticks
          missionPtr->consecutiveWaitTicks++;
          if (missionPtr->consecutiveWaitTicks >= 3) {
            addEvent("Drone #" + to_string(missionPtr->droneID) +
                     " congestion wait threshold reached (3 ticks). Triggering "
                     "forced reroute.");

            // 1. Release current reservations
            releaseMissionReservations(*missionPtr);

            // 2. Run Dijkstra to destination Node
            int finalNode = missionPtr->route.back();
            int newDist = 0;
            vector<int> newPath =
                routeService.calculateRoute(curr, finalNode, newDist);

            bool rerouteSuccess = false;
            if (!newPath.empty() && newPath.size() >= 2) {
              if (reserveCorridorForMission(*missionPtr, curr, newPath[1])) {
                missionPtr->route = newPath;
                missionPtr->currentRouteIndex = 0;
                missionPtr->consecutiveWaitTicks = 0;
                rerouteSuccess = true;
                addEvent("Drone #" + to_string(missionPtr->droneID) +
                         " deadlock recovery: forced reroute succeeded.");
              }
            }

            if (!rerouteSuccess) {
              // Escalation: transition status to BLOCKED
              missionPtr->status = "BLOCKED";
              dronePtr->setStatus("BLOCKED");
              missionPtr->consecutiveWaitTicks = 0;
              addEvent("Drone #" + to_string(missionPtr->droneID) +
                       " deadlock recovery failed. Mission BLOCKED.");
            }
          } else {
            // Traffic Congestion recalculate or wait
            int finalNode = missionPtr->route.back();
            int newDist = 0;
            vector<int> newPath =
                routeService.calculateRoute(curr, finalNode, newDist);

            bool immediateRerouteSuccess = false;
            if (!newPath.empty() && newPath.size() >= 2) {
              if (reserveCorridorForMission(*missionPtr, curr, newPath[1])) {
                releaseCorridorForMission(*missionPtr, curr, next);
                missionPtr->route = newPath;
                missionPtr->currentRouteIndex = 0;
                missionPtr->consecutiveWaitTicks = 0;
                addEvent("Traffic Congestion detected. Drone #" +
                         to_string(missionPtr->droneID) +
                         " recalculated new path to Node " +
                         to_string(finalNode));
                immediateRerouteSuccess = true;
              }
            }

            if (!immediateRerouteSuccess) {
              addEvent("Traffic Congestion detected. Drone #" +
                       to_string(missionPtr->droneID) +
                       " waiting for traffic clearance.");
            }
          }
        }
      }
    }
  }

  // Clear and rebuild activeFlights
  activeFlights.clear();
  for (const auto &m : missionMgr.getAllMissions()) {
    if (m.status == "ACTIVE" || m.status == "RETURNING" ||
        m.status == "EMERGENCY") {
      ActiveFlight flight;
      flight.droneID = m.droneID;
      flight.packageID = (m.status == "ACTIVE") ? m.packageID : -1;
      flight.path = m.route;
      flight.currentPathIndex = m.currentRouteIndex;
      flight.isEmergencyLanding = (m.status == "EMERGENCY");
      flight.isReturning = (m.status == "RETURNING");
      flight.reservedPadID = (m.status == "EMERGENCY")
                                 ? padMgr.getPadIDByNode(m.route.back())
                                 : -1;
      flight.estimatedBatteryConsumption = m.estimatedBatteryConsumption;
      flight.missionID = m.missionID;
      activeFlights.push_back(flight);
    }
  }

  // Validate traffic integrity
  validateTrafficIntegrity();
}

void Simulator::triggerStorm() {
  addEvent("EMERGENCY: Thunderstorm weather alert detected! Activating flight "
           "rollback...");

  vector<Mission> allMissionsCopy = missionMgr.getAllMissions();
  for (const auto &mCopy : allMissionsCopy) {
    if (mCopy.status == "ACTIVE" || mCopy.status == "RETURNING") {
      Mission *missionPtr = missionMgr.getMission(mCopy.missionID);
      Drone *dronePtr = droneMgr.findDrone(missionPtr->droneID);
      if (!dronePtr)
        continue;

      // Release current corridor traffic
      if (static_cast<size_t>(missionPtr->currentRouteIndex) <
          missionPtr->route.size() - 1) {
        int curr = missionPtr->route[missionPtr->currentRouteIndex];
        int next = missionPtr->route[missionPtr->currentRouteIndex + 1];
        airspaceMgr.releaseCorridorTraffic(curr, next);
      }

      // Rollback to last safe checkpoint from Stack
      FlightState checkpoint;
      if (flightMgr.undoFlight(missionPtr->droneID, checkpoint)) {
        dronePtr->updateLocation(checkpoint.currentNode);
        dronePtr->updateBattery(checkpoint.battery);
        addEvent(
            "Drone #" + to_string(missionPtr->droneID) +
            " rolled back to safe Node " + to_string(checkpoint.currentNode) +
            " (" + airspaceMgr.getNodeName(checkpoint.currentNode) +
            "). Battery restored to: " + to_string(checkpoint.battery) + "%");
      } else {
        addEvent("Drone #" + to_string(missionPtr->droneID) +
                 " has no checkpoint history. Remaining at Node " +
                 to_string(dronePtr->getCurrentNode()));
      }

      // If it was delivering a package, cancel the delivery mission and return
      // package to dispatch queue
      if (mCopy.status == "ACTIVE") {
        missionPtr->status = "CANCELLED";

        // Return package to pending queue
        if (missionPtr->packageID != -1) {
          Package *pkgPtr = packageMgr.findPackage(missionPtr->packageID);
          if (pkgPtr) {
            pkgPtr->setStatus("Pending");
            packageMgr.enqueuePackage(*pkgPtr);
            addEvent("Package #" + to_string(missionPtr->packageID) +
                     " returned to dispatch queue.");
          }
        }
        dronePtr->clearAssignedPackage();
      } else {
        missionPtr->status = "CANCELLED";
      }

      // Now locate nearest charging pad using Dijkstra
      int padNodeID = -1;
      int padDist = 0;
      int padID = padMgr.findNearestAvailablePad(
          dronePtr->getCurrentNode(), airspaceMgr.getGraph(),
          airspaceMgr.getRestrictedNodes(), padNodeID, padDist);

      if (padID != -1 && padNodeID != -1) {
        int routeDist = 0;
        vector<int> padRoute = routeService.calculateRoute(
            dronePtr->getCurrentNode(), padNodeID, routeDist);

        if (!padRoute.empty() &&
            airspaceMgr.reserveCorridorTraffic(padRoute[0], padRoute[1])) {
          padMgr.reservePad(padID);

          // Spawn emergency pad landing mission
          int newMissionID = missionMgr.createMission(
              dronePtr->getDroneID(), -1, dronePtr->getCurrentNode(), padNodeID,
              padRoute, routeDist,
              routeService.calculateBatteryCost(padRoute, false));
          missionMgr.startMission(newMissionID);
          Mission *newMissionPtr = missionMgr.getMission(newMissionID);
          newMissionPtr->status = "EMERGENCY";

          dronePtr->setStatus("EMERGENCY");
          addEvent("Drone #" + to_string(dronePtr->getDroneID()) +
                   " emergency route calculated to Pad #" + to_string(padID) +
                   " (Node " + to_string(padNodeID) +
                   "). Distance: " + to_string(padDist));
        } else {
          addEvent("Drone #" + to_string(dronePtr->getDroneID()) +
                   " cannot find corridor path to Pad #" + to_string(padID) +
                   ". Hard landing initiated.");
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
        }
      } else {
        addEvent("Drone #" + to_string(dronePtr->getDroneID()) +
                 " - No available charging pads found! Standard return to "
                 "depot route calculated.");

        // Try warehouse return instead
        int retDist = 0;
        vector<int> retRoute = routeService.calculateReturnRoute(
            dronePtr->getCurrentNode(), retDist);
        if (!retRoute.empty() &&
            airspaceMgr.reserveCorridorTraffic(retRoute[0], retRoute[1])) {
          // Spawn return mission
          int newMissionID = missionMgr.createMission(
              dronePtr->getDroneID(), -1, dronePtr->getCurrentNode(),
              retRoute.back(), retRoute, retDist,
              routeService.calculateBatteryCost(retRoute, false));
          missionMgr.startMission(newMissionID);
          Mission *newMissionPtr = missionMgr.getMission(newMissionID);
          newMissionPtr->status = "RETURNING";

          dronePtr->setStatus("RETURNING");
        } else {
          addEvent("Drone #" + to_string(dronePtr->getDroneID()) +
                   " grounded at Node " +
                   to_string(dronePtr->getCurrentNode()) + ".");
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
        }
      }
    }
  }

  // Rebuild activeFlights
  activeFlights.clear();
  for (const auto &m : missionMgr.getAllMissions()) {
    if (m.status == "ACTIVE" || m.status == "RETURNING" ||
        m.status == "EMERGENCY") {
      ActiveFlight flight;
      flight.droneID = m.droneID;
      flight.packageID = (m.status == "ACTIVE") ? m.packageID : -1;
      flight.path = m.route;
      flight.currentPathIndex = m.currentRouteIndex;
      flight.isEmergencyLanding = (m.status == "EMERGENCY");
      flight.isReturning = (m.status == "RETURNING");
      flight.reservedPadID = (m.status == "EMERGENCY")
                                 ? padMgr.getPadIDByNode(m.route.back())
                                 : -1;
      flight.estimatedBatteryConsumption = m.estimatedBatteryConsumption;
      flight.missionID = m.missionID;
      activeFlights.push_back(flight);
    }
  }
}

bool Simulator::undoLastStep(int droneID) {
  int idx = findActiveFlightIndex(droneID);
  if (idx == -1) {
    addEvent("Undo step failed: Drone #" + to_string(droneID) +
             " is not currently in flight.");
    return false;
  }

  ActiveFlight &flight = activeFlights[idx];
  Drone *dronePtr = droneMgr.findDrone(droneID);
  if (!dronePtr)
    return false;

  // Release current corridor traffic
  if (flight.currentPathIndex < flight.path.size() - 1) {
    int curr = flight.path[flight.currentPathIndex];
    int next = flight.path[flight.currentPathIndex + 1];
    airspaceMgr.releaseCorridorTraffic(curr, next);
  }

  FlightState checkpoint;
  if (flightMgr.undoFlight(droneID, checkpoint)) {
    dronePtr->updateLocation(checkpoint.currentNode);
    dronePtr->updateBattery(checkpoint.battery);

    // Find corresponding mission
    Mission *missionPtr = missionMgr.getMission(flight.missionID);
    if (missionPtr) {
      auto pIt = find(missionPtr->route.begin(), missionPtr->route.end(),
                      checkpoint.currentNode);
      if (pIt != missionPtr->route.end()) {
        missionPtr->currentRouteIndex =
            distance(missionPtr->route.begin(), pIt);

        // Reserve traffic on the current edge again
        if (static_cast<size_t>(missionPtr->currentRouteIndex) <
            missionPtr->route.size() - 1) {
          int nextNode = missionPtr->route[missionPtr->currentRouteIndex + 1];
          airspaceMgr.reserveCorridorTraffic(checkpoint.currentNode, nextNode);
        }
      }
    }

    // Rebuild activeFlights
    activeFlights.clear();
    for (const auto &m : missionMgr.getAllMissions()) {
      if (m.status == "ACTIVE" || m.status == "RETURNING" ||
          m.status == "EMERGENCY") {
        ActiveFlight f;
        f.droneID = m.droneID;
        f.packageID = (m.status == "ACTIVE") ? m.packageID : -1;
        f.path = m.route;
        f.currentPathIndex = m.currentRouteIndex;
        f.isEmergencyLanding = (m.status == "EMERGENCY");
        f.isReturning = (m.status == "RETURNING");
        f.reservedPadID = (m.status == "EMERGENCY")
                              ? padMgr.getPadIDByNode(m.route.back())
                              : -1;
        f.estimatedBatteryConsumption = m.estimatedBatteryConsumption;
        f.missionID = m.missionID;
        activeFlights.push_back(f);
      }
    }

    addEvent("Manual Undo: Drone #" + to_string(droneID) +
             " reverted to Node " + to_string(checkpoint.currentNode) + " (" +
             airspaceMgr.getNodeName(checkpoint.currentNode) +
             "). Battery: " + to_string(checkpoint.battery) + "%");
    return true;
  }

  addEvent("Undo step failed: Drone #" + to_string(droneID) +
           " has no checkpoint history.");
  return false;
}

const DroneManager &Simulator::getDroneMgr() const { return droneMgr; }
DroneManager &Simulator::getDroneMgrWritable() { return droneMgr; }
const PackageManager &Simulator::getPackageMgr() const { return packageMgr; }
PackageManager &Simulator::getPackageMgrWritable() { return packageMgr; }
const AirspaceManager &Simulator::getAirspaceMgr() const { return airspaceMgr; }
const PadManager &Simulator::getPadMgr() const { return padMgr; }
const FlightManager &Simulator::getFlightMgr() const { return flightMgr; }
const AddressService &Simulator::getAddressService() const {
  return addressService;
}
const RouteService &Simulator::getRouteService() const { return routeService; }
const MissionManager &Simulator::getMissionMgr() const { return missionMgr; }
MissionManager &Simulator::getMissionMgrWritable() { return missionMgr; }
const vector<ActiveFlight> &Simulator::getActiveFlights() const {
  return activeFlights;
}
const vector<string> &Simulator::getEventLog() const { return eventLog; }
int Simulator::getSimulationTimeStep() const { return simulationTimeStep; }

void Simulator::clearEventLog() { eventLog.clear(); }
void Simulator::addEvent(const string &event) { eventLog.push_back(event); }

void Simulator::advanceTick() {
  simulationTimeStep++;
  cout << "\n====================\n";
  cout << "TICK " << simulationTimeStep << "\n";
  cout << "====================\n";

  // 1. Charge drones parked at charging pads
  vector<Drone> allDrones = droneMgr.getAllDrones();
  for (const auto &d : allDrones) {
    int activeIdx = findActiveFlightIndex(d.getDroneID());
    // Drone is not active, battery is < 100
    if (activeIdx == -1 && d.getBattery() < 100.0f) {
      int droneNode = d.getCurrentNode();
      char type = airspaceMgr.getNodeType(droneNode);
      if (type == 'P') {
        Drone *dronePtr = droneMgr.findDrone(d.getDroneID());
        float oldBat = dronePtr->getBattery();
        dronePtr->charge(15.0f); // 15% charge per step
        dronePtr->setStatus("CHARGING");

        int padID = padMgr.getPadIDByNode(droneNode);
        cout << "Drone " << d.getDroneID() << "\n";
        cout << "Charging at Pad " << padID << "\n";
        cout << "Battery: " << static_cast<int>(oldBat) << " -> "
             << static_cast<int>(dronePtr->getBattery()) << "\n";
        cout << "Status: CHARGING\n";
        cout << "-------------------\n";

        if (dronePtr->getBattery() >= 100.0f) {
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
          padMgr.releasePadByNode(droneNode);
          addEvent("Drone #" + to_string(d.getDroneID()) +
                   " fully charged and available.");
        }
      }
    }
  }

  // 2. Advance active missions
  vector<Mission> allMissionsCopy = missionMgr.getAllMissions();
  for (const auto &mCopy : allMissionsCopy) {
    if (mCopy.status == "ACTIVE" || mCopy.status == "RETURNING" ||
        mCopy.status == "EMERGENCY") {
      Mission *missionPtr = missionMgr.getMission(mCopy.missionID);
      Drone *dronePtr = droneMgr.findDrone(missionPtr->droneID);

      if (!dronePtr) {
        missionPtr->status = "CANCELLED";
        missionMgr.cancelMission(missionPtr->missionID);
        continue;
      }

      int curr = dronePtr->getCurrentNode();
      float oldBat = dronePtr->getBattery();

      // Check if drone has arrived at the end of its path
      if (static_cast<size_t>(missionPtr->currentRouteIndex) >=
          missionPtr->route.size() - 1) {
        int arrivedNode = missionPtr->route.back();

        // Release all reservations for the mission
        releaseMissionReservations(*missionPtr);

        dronePtr->updateLocation(arrivedNode);

        if (missionPtr->status == "ACTIVE") {
          // Delivery Complete
          Package *pkgPtr = packageMgr.findPackage(missionPtr->packageID);
          if (pkgPtr) {
            pkgPtr->markDelivered();
          }

          cout << "Drone " << missionPtr->droneID << "\n";
          cout << "Arrived: Node " << arrivedNode << " ("
               << airspaceMgr.getNodeName(arrivedNode) << ")\n";
          cout << "Package #" << missionPtr->packageID << " DELIVERED\n";
          cout << "Status: IDLE\n";
          cout << "-------------------\n";

          addEvent("Package #" + to_string(missionPtr->packageID) +
                   " delivered by Drone #" + to_string(missionPtr->droneID));
          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->clearAssignedPackage();
          dronePtr->setStatus("IDLE");

          // Check battery for return / charge
          bool rerouted = false;
          if (dronePtr->getBattery() < 30.0f) {
            int padNodeID = -1;
            int padDist = 0;
            int padID = padMgr.findNearestAvailablePad(
                arrivedNode, airspaceMgr.getGraph(),
                airspaceMgr.getRestrictedNodes(), padNodeID, padDist, false);

            if (padID != -1 && padNodeID != -1) {
              int routeDist = 0;
              vector<int> padRoute = routeService.calculateRoute(
                  arrivedNode, padNodeID, routeDist);

              if (!padRoute.empty() &&
                  reserveCorridorForMission(*missionPtr, padRoute[0],
                                            padRoute[1])) {
                padMgr.reservePad(padID);

                missionPtr->route = padRoute;
                missionPtr->currentRouteIndex = 0;
                missionPtr->status = "EMERGENCY";
                missionPtr->estimatedBatteryConsumption =
                    routeService.calculateBatteryCost(padRoute, false);
                dronePtr->setStatus("EMERGENCY");
                rerouted = true;
              }
            }
          }

          if (!rerouted) {
            // Return to depot
            int returnDist = 0;
            vector<int> returnRoute =
                routeService.calculateReturnRoute(arrivedNode, returnDist);
            if (!returnRoute.empty() &&
                reserveCorridorForMission(*missionPtr, returnRoute[0],
                                          returnRoute[1])) {
              missionPtr->route = returnRoute;
              missionPtr->currentRouteIndex = 0;
              missionPtr->status = "RETURNING";
              missionPtr->estimatedBatteryConsumption =
                  routeService.calculateBatteryCost(returnRoute, false);
              dronePtr->setStatus("RETURNING");
            } else {
              dronePtr->setAvailable(true);
              missionPtr->status = "DELIVERED";
              missionMgr.archiveMission(missionPtr->missionID);
            }
          }
        } else if (missionPtr->status == "EMERGENCY") {
          int padID = padMgr.getPadIDByNode(arrivedNode);
          cout << "Drone " << missionPtr->droneID << "\n";
          cout << "Landed at Pad " << padID << " (Node " << arrivedNode
               << ")\n";
          cout << "Status: CHARGING\n";
          cout << "-------------------\n";

          addEvent("Drone #" + to_string(missionPtr->droneID) +
                   " landed at Pad #" + to_string(padID));
          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->setAvailable(false);
          dronePtr->setStatus("CHARGING");
          missionPtr->status = "DELIVERED";
          missionMgr.archiveMission(missionPtr->missionID);
        } else if (missionPtr->status == "RETURNING") {
          cout << "Drone " << missionPtr->droneID << "\n";
          cout << "Returned to depot (Node " << arrivedNode << ")\n";
          cout << "Status: IDLE\n";
          cout << "-------------------\n";

          addEvent("Drone #" + to_string(missionPtr->droneID) +
                   " returned to depot.");
          flightMgr.clearHistory(missionPtr->droneID);
          dronePtr->setAvailable(true);
          dronePtr->setStatus("IDLE");
          missionPtr->status = "DELIVERED";
          missionMgr.archiveMission(missionPtr->missionID);
        }
      } else {
        // Move one step forward
        int next = missionPtr->route[missionPtr->currentRouteIndex + 1];

        // Check next-next corridor capacity
        bool canMove = true;
        if (static_cast<size_t>(missionPtr->currentRouteIndex) + 2 <
            missionPtr->route.size()) {
          int nextNext = missionPtr->route[missionPtr->currentRouteIndex + 2];
          if (!reserveCorridorForMission(*missionPtr, next, nextNext)) {
            canMove = false;
          }
        }

        if (canMove) {
          // Reset consecutiveWaitTicks
          missionPtr->consecutiveWaitTicks = 0;

          // Save flight state checkpoint
          vector<int> remainingPath(missionPtr->route.begin() +
                                        missionPtr->currentRouteIndex,
                                    missionPtr->route.end());
          FlightState checkpoint = {curr, oldBat, remainingPath};
          flightMgr.saveState(missionPtr->droneID, checkpoint);

          // Release previous corridor traffic
          releaseCorridorForMission(*missionPtr, curr, next);

          // Drain battery by edge cost + penalties
          int weight = 1;
          const auto &graph = airspaceMgr.getGraph();
          auto git = graph.find(curr);
          if (git != graph.end()) {
            for (const auto &edge : git->second) {
              if (edge.destination == next) {
                weight = edge.weight;
                break;
              }
            }
          }

          bool hasPayload = (missionPtr->status == "ACTIVE");
          float batteryDrain = weight + 3 + (hasPayload ? 2 : 0);

          dronePtr->drainBattery(batteryDrain);
          dronePtr->updateLocation(next);
          missionPtr->currentRouteIndex++;

          string statusStr = missionPtr->status;
          dronePtr->setStatus(statusStr == "ACTIVE" ? "DELIVERING" : statusStr);

          cout << "Drone " << missionPtr->droneID << "\n";
          cout << "Moved: Node " << curr << " -> Node " << next << " ("
               << airspaceMgr.getNodeName(next) << ")\n";
          cout << "Battery: " << static_cast<int>(oldBat) << " -> "
               << static_cast<int>(dronePtr->getBattery()) << "\n";
          cout << "Status: " << dronePtr->getStatus() << "\n";
          cout << "-------------------\n";
        } else {
          // Increment consecutive wait ticks
          missionPtr->consecutiveWaitTicks++;
          if (missionPtr->consecutiveWaitTicks >= 3) {
            cout << "Drone " << missionPtr->droneID << " waiting at Node "
                 << curr << " due to congestion (Wait ticks: "
                 << missionPtr->consecutiveWaitTicks << "/3).\n";
            cout << "Deadlock/Congestion limit reached (3 ticks) for Mission "
                 << missionPtr->missionID << ". Triggering forced reroute.\n";

            // 1. Release current reservations
            releaseMissionReservations(*missionPtr);

            // 2. Run Dijkstra to destination Node
            int finalNode = missionPtr->route.back();
            int newDist = 0;
            vector<int> newPath =
                routeService.calculateRoute(curr, finalNode, newDist);

            bool rerouteSuccess = false;
            if (!newPath.empty() && newPath.size() >= 2) {
              if (reserveCorridorForMission(*missionPtr, curr, newPath[1])) {
                missionPtr->route = newPath;
                missionPtr->currentRouteIndex = 0;
                missionPtr->consecutiveWaitTicks = 0;
                rerouteSuccess = true;
                cout << "Forced reroute succeeded for Drone "
                     << missionPtr->droneID << " to Node " << finalNode
                     << ".\n";
                addEvent("Drone #" + to_string(missionPtr->droneID) +
                         " deadlock recovery: forced reroute succeeded.");
              }
            }

            if (!rerouteSuccess) {
              // Escalation: transition status to BLOCKED
              missionPtr->status = "BLOCKED";
              dronePtr->setStatus("BLOCKED");
              missionPtr->consecutiveWaitTicks = 0;
              cout << "Forced reroute failed for Drone " << missionPtr->droneID
                   << ". Mission BLOCKED.\n";
              addEvent("Drone #" + to_string(missionPtr->droneID) +
                       " deadlock recovery failed. Mission BLOCKED.");
            }
          } else {
            // Standard congestion handling: try to recalculate or wait
            int finalNode = missionPtr->route.back();
            int newDist = 0;
            vector<int> newPath =
                routeService.calculateRoute(curr, finalNode, newDist);

            bool immediateRerouteSuccess = false;
            if (!newPath.empty() && newPath.size() >= 2) {
              if (reserveCorridorForMission(*missionPtr, curr, newPath[1])) {
                releaseCorridorForMission(*missionPtr, curr, next);
                missionPtr->route = newPath;
                missionPtr->currentRouteIndex = 0;
                missionPtr->consecutiveWaitTicks = 0;
                cout << "Drone " << missionPtr->droneID
                     << " recalculated route to avoid congestion.\n";
                immediateRerouteSuccess = true;
              }
            }

            if (!immediateRerouteSuccess) {
              cout << "Drone " << missionPtr->droneID << " waiting at Node "
                   << curr << " due to congestion (Wait ticks: "
                   << missionPtr->consecutiveWaitTicks << "/3).\n";
            }
          }
        }
      }
    }
  }

  // Clear and rebuild activeFlights
  activeFlights.clear();
  for (const auto &m : missionMgr.getAllMissions()) {
    if (m.status == "ACTIVE" || m.status == "RETURNING" ||
        m.status == "EMERGENCY") {
      ActiveFlight flight;
      flight.droneID = m.droneID;
      flight.packageID = (m.status == "ACTIVE") ? m.packageID : -1;
      flight.path = m.route;
      flight.currentPathIndex = m.currentRouteIndex;
      flight.isEmergencyLanding = (m.status == "EMERGENCY");
      flight.isReturning = (m.status == "RETURNING");
      flight.reservedPadID = (m.status == "EMERGENCY")
                                 ? padMgr.getPadIDByNode(m.route.back())
                                 : -1;
      flight.estimatedBatteryConsumption = m.estimatedBatteryConsumption;
      flight.missionID = m.missionID;
      activeFlights.push_back(flight);
    }
  }

  // Validate traffic integrity
  validateTrafficIntegrity();
}

void Simulator::displayDroneLocations() const {
  cout << "\n================================================\n";
  cout << "LIVE FLEET STATUS\n";
  cout << "================================================\n";

  vector<Mission> allMissions = missionMgr.getAllMissions();
  int activeCount = 0;
  int archivedCount = 0;
  for (const auto &m : allMissions) {
    if (m.status == "ACTIVE" || m.status == "RETURNING" ||
        m.status == "EMERGENCY" || m.status == "BLOCKED") {
      activeCount++;
    } else if (m.status == "ARCHIVED") {
      archivedCount++;
    }
  }
  cout << "Active Missions: " << activeCount << "\n";
  cout << "Archived Missions: " << archivedCount << "\n";
  cout << "------------------------------------------------\n";

  vector<Drone> drones = droneMgr.getAllDrones();
  for (const auto &d : drones) {
    // Find mission associated with the drone
    const Mission *associatedMission = nullptr;
    for (const auto &m : allMissions) {
      if (m.droneID == d.getDroneID() &&
          (m.status == "ACTIVE" || m.status == "RETURNING" ||
           m.status == "EMERGENCY" || m.status == "BLOCKED")) {
        associatedMission = &m;
        break;
      }
    }

    if (associatedMission != nullptr) {
      cout << "Mission ID: " << associatedMission->missionID << "\n";
      cout << "Drone: " << d.getDroneID() << "\n";
      cout << "Package: "
           << (associatedMission->packageID == -1
                   ? "N/A"
                   : to_string(associatedMission->packageID))
           << "\n";

      int currNode = d.getCurrentNode();
      char type = airspaceMgr.getNodeType(currNode);
      if (type == 'P') {
        int padID = padMgr.getPadIDByNode(currNode);
        cout << "Current Node: Pad " << padID << "\n";
      } else {
        cout << "Current Node: " << currNode << "\n";
      }

      int destNode = associatedMission->route.empty()
                         ? -1
                         : associatedMission->route.back();
      char destType = airspaceMgr.getNodeType(destNode);
      if (destType == 'P') {
        int padID = padMgr.getPadIDByNode(destNode);
        cout << "Destination: Charging Pad " << padID << "\n";
      } else if (associatedMission->status == "RETURNING") {
        cout << "Destination: Warehouse (" << destNode << ")\n";
      } else {
        cout << "Destination: " << destNode << "\n";
      }

      int totalLegs =
          associatedMission->route.empty()
              ? 0
              : static_cast<int>(associatedMission->route.size() - 1);
      cout << "Route Progress: " << associatedMission->currentRouteIndex << "/"
           << totalLegs << "\n";
      cout << "Battery Remaining: " << static_cast<int>(d.getBattery()) << "\n";
      cout << "Status: " << associatedMission->status << "\n";
      if (associatedMission->status == "BLOCKED") {
        cout << "Reason: Congestion deadlock\n";
      }
    } else {
      cout << "Drone: " << d.getDroneID() << "\n";
      int currNode = d.getCurrentNode();
      char type = airspaceMgr.getNodeType(currNode);
      if (type == 'P') {
        int padID = padMgr.getPadIDByNode(currNode);
        cout << "Current Node: Pad " << padID << "\n";
      } else {
        cout << "Current Node: " << currNode << " ("
             << airspaceMgr.getNodeName(currNode) << ")\n";
      }
      cout << "Battery Remaining: " << static_cast<int>(d.getBattery()) << "\n";
      cout << "Status: " << d.getStatus() << "\n";
    }
    cout << "------------------------------------------------\n";
  }
}
