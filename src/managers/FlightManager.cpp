#include "FlightManager.h"

using namespace std;

FlightManager::FlightManager() {}

void FlightManager::saveState(int droneID, const FlightState& state) {
    flightHistories[droneID].push(state);
}

bool FlightManager::undoFlight(int droneID, FlightState& outState) {
    auto it = flightHistories.find(droneID);
    if (it == flightHistories.end() || it->second.empty()) {
        return false;
    }

    outState = it->second.top();
    it->second.pop();
    return true;
}

bool FlightManager::hasHistory(int droneID) const {
    auto it = flightHistories.find(droneID);
    if (it == flightHistories.end()) {
        return false;
    }
    return !it->second.empty();
}

void FlightManager::clearHistory(int droneID) {
    auto it = flightHistories.find(droneID);
    if (it != flightHistories.end()) {
        // Clear the stack
        stack<FlightState> emptyStack;
        it->second = emptyStack;
    }
}

size_t FlightManager::getHistoryDepth(int droneID) const {
    auto it = flightHistories.find(droneID);
    if (it == flightHistories.end()) {
        return 0;
    }
    return it->second.size();
}
