#ifndef DRONEMANAGER_H
#define DRONEMANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "../models/Drone.h"
#include "../algorithms/Dijkstra.h"
using namespace std;

class RouteService;
class PadManager;
class AirspaceManager;

class DroneManager {
private:
    unordered_map<int, Drone> drones;

public:
    DroneManager();

    bool loadDronesFromFile(const string& filepath);
    bool saveDronesToFile(const string& filepath) const;

    bool registerDrone(const Drone& drone);
    bool removeDrone(int droneID);
    
    // Find a drone by ID
    Drone* findDrone(int droneID);
    const Drone* findDroneConst(int droneID) const;

    // Get list of all drones
    vector<Drone> getAllDrones() const;

    // Select the best available drone using a priority queue (Max Heap)
    // Returns true if a drone was found, writes to outDrone
    bool selectBestAvailableDrone(Drone& outDrone) const;

    // Selects the best drone using intelligent scoring based on battery and distance
    Drone selectBestDrone(
        int pickupNode,
        int destinationNode,
        const RouteService& routeService,
        const PadManager& padMgr,
        const AirspaceManager& airspaceMgr
    ) const;

    // Checks if drone has enough battery for the route cost
    bool canCompleteRoute(const Drone& drone, int routeCost) const;

    // Updates battery level of a drone
    void updateBattery(int droneID, float batteryLevel);
    
    // Update drone location
    void updateLocation(int droneID, int nodeID);

    // Set drone availability
    void setAvailability(int droneID, bool available);
};

#endif // DRONEMANAGER_H
