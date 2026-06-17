#include "DroneManager.h"
#include <fstream>
#include <sstream>
#include <queue>
#include <iostream>
#include <limits>
#include <algorithm>
#include "../services/RouteService.h"
#include "PadManager.h"
#include "AirspaceManager.h"

using namespace std;

DroneManager::DroneManager() {}

bool DroneManager::loadDronesFromFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not open drones file " << filepath << endl;
        return false;
    }

    drones.clear();
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        stringstream ss(line);
        int id;
        float battery;
        int node;
        int availVal;
        
        if (ss >> id >> battery >> node >> availVal) {
            Drone d(id, battery, node, availVal == 1);
            drones[id] = d;
        }
    }
    file.close();
    return true;
}

bool DroneManager::saveDronesToFile(const string& filepath) const {
    ofstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not write to drones file " << filepath << endl;
        return false;
    }

    file << "# droneID battery currentNode available\n";
    for (const auto& pair : drones) {
        const Drone& d = pair.second;
        file << d.getDroneID() << " " 
             << d.getBattery() << " " 
             << d.getCurrentNode() << " " 
             << (d.isAvailable() ? 1 : 0) << "\n";
    }
    file.close();
    return true;
}

bool DroneManager::registerDrone(const Drone& drone) {
    if (drones.find(drone.getDroneID()) != drones.end()) {
        return false; // Already registered
    }
    drones[drone.getDroneID()] = drone;
    return true;
}

bool DroneManager::removeDrone(int droneID) {
    auto it = drones.find(droneID);
    if (it == drones.end()) {
        return false;
    }
    drones.erase(it);
    return true;
}

Drone* DroneManager::findDrone(int droneID) {
    auto it = drones.find(droneID);
    if (it == drones.end()) {
        return nullptr;
    }
    return &(it->second);
}

const Drone* DroneManager::findDroneConst(int droneID) const {
    auto it = drones.find(droneID);
    if (it == drones.end()) {
        return nullptr;
    }
    return &(it->second);
}

vector<Drone> DroneManager::getAllDrones() const {
    vector<Drone> list;
    for (const auto& pair : drones) {
        list.push_back(pair.second);
    }
    return list;
}

bool DroneManager::selectBestAvailableDrone(Drone& outDrone) const {
    priority_queue<Drone> pq;

    for (const auto& pair : drones) {
        if (pair.second.isAvailable() && pair.second.getBattery() > 10.0f) { // drone must have at least 10% battery
            pq.push(pair.second);
        }
    }

    if (pq.empty()) {
        return false;
    }

    outDrone = pq.top();
    return true;
}

Drone DroneManager::selectBestDrone(
    int pickupNode,
    int destinationNode,
    const RouteService& routeService,
    const PadManager& padMgr,
    const AirspaceManager& airspaceMgr
) const {
    vector<Drone> candidates;
    for (const auto& pair : drones) {
        if (pair.second.isAvailable() && pair.second.getBattery() > 10.0f) {
            candidates.push_back(pair.second);
        }
    }

    if (candidates.empty()) {
        return Drone();
    }

    sort(candidates.begin(), candidates.end(), [](const Drone& a, const Drone& b) {
        return a.getDroneID() < b.getDroneID();
    });

    cout << "\nDrone Evaluation\n\n";
    
    Drone bestDrone;
    float highestMargin = -999999.0f;
    bool found = false;

    for (const auto& d : candidates) {
        // Calculate pickup route and cost
        int pickupDist = 0;
        vector<int> pickupRoute;
        if (d.getCurrentNode() != pickupNode) {
            pickupRoute = routeService.calculateRoute(d.getCurrentNode(), pickupNode, pickupDist);
            if (pickupRoute.empty()) {
                continue; // Unreachable pickup
            }
        } else {
            pickupRoute = {pickupNode};
        }
        float pickupCost = routeService.calculateBatteryCost(pickupRoute, false);

        // Calculate delivery route and cost
        int deliveryDist = 0;
        vector<int> deliveryRoute = routeService.calculateRoute(pickupNode, destinationNode, deliveryDist);
        if (deliveryRoute.empty()) {
            continue; // Unreachable destination
        }
        float deliveryCost = routeService.calculateBatteryCost(deliveryRoute, true);

        // Calculate return route and cost
        int returnDist = 0;
        vector<int> returnRoute = routeService.calculateReturnRoute(destinationNode, returnDist);
        float returnCost = numeric_limits<float>::max();
        if (!returnRoute.empty()) {
            returnCost = routeService.calculateBatteryCost(returnRoute, false);
        }

        // Calculate nearest charging pad route and cost
        int padNodeID = -1;
        int padDist = 0;
        // Call with quiet=true to avoid double-printing during evaluation
        int padID = padMgr.findNearestAvailablePad(destinationNode, airspaceMgr.getGraph(), airspaceMgr.getRestrictedNodes(), padNodeID, padDist, true);
        float nearestPadCost = numeric_limits<float>::max();
        if (padID != -1 && padNodeID != -1) {
            vector<int> padRoute = Dijkstra::shortestPath(destinationNode, padNodeID, airspaceMgr.getGraph(), airspaceMgr.getRestrictedNodes());
            if (!padRoute.empty()) {
                nearestPadCost = routeService.calculateBatteryCost(padRoute, false);
            }
        }

        float safeExitCost = min(returnCost, nearestPadCost);
        if (safeExitCost == numeric_limits<float>::max()) {
            continue; // No safe way out of destination
        }

        float requiredEnergy = pickupCost + deliveryCost + safeExitCost;
        float energyMargin = d.getBattery() - requiredEnergy;

        cout << "Drone " << d.getDroneID() << "\n";
        cout << "Battery: " << static_cast<int>(d.getBattery()) << "\n";
        cout << "Delivery Cost: " << static_cast<int>(pickupCost + deliveryCost) << "\n"; // Total delivery cost (pickup + delivery)
        cout << "Return Cost: " << (returnCost == numeric_limits<float>::max() ? -1 : static_cast<int>(returnCost)) << "\n";
        cout << "Nearest Pad Cost: " << (nearestPadCost == numeric_limits<float>::max() ? -1 : static_cast<int>(nearestPadCost)) << "\n";
        cout << "Safe Exit Cost: " << static_cast<int>(safeExitCost) << "\n";
        cout << "Total Required: " << static_cast<int>(requiredEnergy) << "\n";
        cout << "Energy Margin: " << static_cast<int>(energyMargin) << "\n";
        cout << "--------------------\n";

        if (energyMargin >= 0.0f) {
            if (energyMargin > highestMargin) {
                highestMargin = energyMargin;
                bestDrone = d;
                found = true;
            }
        }
    }

    if (found) {
        cout << "Selected Drone: " << bestDrone.getDroneID() << " wins.\n";
        return bestDrone;
    }

    return Drone();
}

bool DroneManager::canCompleteRoute(const Drone& drone, int routeCost) const {
    return drone.getBattery() >= routeCost;
}

void DroneManager::updateBattery(int droneID, float batteryLevel) {
    auto it = drones.find(droneID);
    if (it != drones.end()) {
        float diff = batteryLevel - it->second.getBattery();
        if (diff > 0) {
            it->second.charge(diff);
        } else {
            it->second.drainBattery(-diff);
        }
    }
}

void DroneManager::updateLocation(int droneID, int nodeID) {
    auto it = drones.find(droneID);
    if (it != drones.end()) {
        it->second.updateLocation(nodeID);
    }
}

void DroneManager::setAvailability(int droneID, bool available) {
    auto it = drones.find(droneID);
    if (it != drones.end()) {
        it->second.setAvailable(available);
    }
}
