#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <unordered_map>
#include <stack>
#include <vector>
#include "../models/FlightState.h"
using namespace std;

class FlightManager {
private:
    // Stack of flight states per drone (droneID -> stack of states)
    unordered_map<int, stack<FlightState>> flightHistories;

public:
    FlightManager();

    // Save current flight state of a drone
    void saveState(int droneID, const FlightState& state);

    // Undo flight step (pop and return last safe state)
    // Returns true if rollback was successful, writes to outState
    bool undoFlight(int droneID, FlightState& outState);

    // Check if drone has any history to rollback
    bool hasHistory(int droneID) const;

    // Clear history when flight is finished
    void clearHistory(int droneID);
    
    // Get depth of history
    size_t getHistoryDepth(int droneID) const;
};

#endif // FLIGHTMANAGER_H
