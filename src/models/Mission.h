#ifndef MISSION_H
#define MISSION_H

#include <string>
#include <vector>
using namespace std;

struct ReservedCorridor {
  int fromNode;
  int toNode;
};

class Mission {
public:
  int missionID;
  int droneID;
  int packageID;
  int sourceNode;
  int destinationNode;
  vector<int> route;
  int routeCost;
  int currentRouteIndex;
  int estimatedBatteryConsumption;
  string status; // PENDING, ACTIVE, DELIVERED, RETURNING, EMERGENCY, CANCELLED,
                 // BLOCKED, ARCHIVED

  vector<ReservedCorridor> activeReservations;
  int consecutiveWaitTicks;

  Mission();
  Mission(int mID, int dID, int pID, int src, int dest, const vector<int> &rt,
          int cost, int estBat);
};

#endif // MISSION_H
