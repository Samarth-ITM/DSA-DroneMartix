#include "Mission.h"

using namespace std;

Mission::Mission()
    : missionID(-1), droneID(-1), packageID(-1), sourceNode(-1),
      destinationNode(-1), routeCost(0), currentRouteIndex(0),
      estimatedBatteryConsumption(0), status("PENDING"),
      consecutiveWaitTicks(0) {}

Mission::Mission(int mID, int dID, int pID, int src, int dest,
                 const vector<int> &rt, int cost, int estBat)
    : missionID(mID), droneID(dID), packageID(pID), sourceNode(src),
      destinationNode(dest), route(rt), routeCost(cost), currentRouteIndex(0),
      estimatedBatteryConsumption(estBat), status("PENDING"),
      consecutiveWaitTicks(0) {}
