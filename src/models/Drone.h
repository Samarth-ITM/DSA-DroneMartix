#ifndef DRONE_H
#define DRONE_H

#include <string>
using namespace std;

class Drone {
private:
  int droneID;
  float battery;
  int currentNode;
  bool available;
  int assignedPackageID; // -1 if none
  string status; // "IDLE", "DELIVERING", "RETURNING", "CHARGING", "EMERGENCY"

public:
  Drone();
  Drone(int id, float bat, int node, bool avail);

  int getDroneID() const;
  float getBattery() const;
  int getCurrentNode() const;
  bool isAvailable() const;
  int getAssignedPackageID() const;
  string getStatus() const;
  void setStatus(const string &newStatus);

  void charge(float amount);
  void drainBattery(float amount);
  void updateBattery(float level);
  void updateLocation(int node);
  void assignPackage(int packageID);
  void setAvailable(bool avail);
  void clearAssignedPackage();

  // Priority Queue ordering: Max Heap on battery
  bool operator<(const Drone &other) const;
};

#endif // DRONE_H
