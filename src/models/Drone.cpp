#include "Drone.h"

using namespace std;

Drone::Drone()
    : droneID(-1), battery(0.0f), currentNode(0), available(false),
      assignedPackageID(-1), status("IDLE") {}

Drone::Drone(int id, float bat, int node, bool avail)
    : droneID(id), battery(bat), currentNode(node), available(avail),
      assignedPackageID(-1), status("IDLE") {}

int Drone::getDroneID() const { return droneID; }

float Drone::getBattery() const { return battery; }

int Drone::getCurrentNode() const { return currentNode; }

bool Drone::isAvailable() const { return available; }

int Drone::getAssignedPackageID() const { return assignedPackageID; }

string Drone::getStatus() const { return status; }

void Drone::setStatus(const string &newStatus) { status = newStatus; }

void Drone::charge(float amount) {
  battery += amount;
  if (battery > 100.0f) {
    battery = 100.0f;
  }
}

void Drone::drainBattery(float amount) {
  battery -= amount;
  if (battery < 0.0f) {
    battery = 0.0f;
  }
}

void Drone::updateBattery(float level) {
  battery = level;
  if (battery > 100.0f)
    battery = 100.0f;
  if (battery < 0.0f)
    battery = 0.0f;
}

void Drone::updateLocation(int node) { currentNode = node; }

void Drone::assignPackage(int packageID) {
  assignedPackageID = packageID;
  available = false;
}

void Drone::setAvailable(bool avail) { available = avail; }

void Drone::clearAssignedPackage() { assignedPackageID = -1; }

bool Drone::operator<(const Drone &other) const {
  return battery < other.battery;
}
