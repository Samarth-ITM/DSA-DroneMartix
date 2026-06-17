#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "../managers/AirspaceManager.h"
#include "../managers/DroneManager.h"
#include "../managers/FlightManager.h"
#include "../managers/MissionManager.h"
#include "../managers/PackageManager.h"
#include "../managers/PadManager.h"
#include "../services/AddressService.h"
#include "../services/RouteService.h"
#include <string>
#include <vector>
using namespace std;

struct ActiveFlight {
  int droneID;
  int packageID;    // -1 if returning/charging without package
  vector<int> path; // Full route node sequence
  size_t currentPathIndex;
  bool isEmergencyLanding; // True if flying to charging pad
  bool isReturning;        // True if flying back to warehouse
  int reservedPadID;       // ID of pad reserved, -1 if none
  float estimatedBatteryConsumption;
  int missionID;
};

class Simulator {
private:
  DroneManager droneMgr;
  PackageManager packageMgr;
  AirspaceManager airspaceMgr;
  FlightManager flightMgr;
  PadManager padMgr;
  AddressService addressService;
  RouteService routeService;
  MissionManager missionMgr;

  vector<ActiveFlight> activeFlights;
  vector<string> eventLog;
  int simulationTimeStep;

  // Helper: Find active flight index by droneID
  int findActiveFlightIndex(int droneID) const;

  // Corridor reservation tracking helpers
  bool reserveCorridorForMission(Mission &mission, int u, int v);
  void releaseCorridorForMission(Mission &mission, int u, int v);
  void releaseMissionReservations(Mission &mission);
  bool validateTrafficIntegrity() const;

public:
  Simulator();

  bool initialize(const string &graphFile, const string &dronesFile,
                  const string &padsFile, const string &addressesFile);

  bool saveState(const string &dronesFile, const string &padsFile,
                 const string &addressesFile) const;

  // Dispatch package from queue
  bool dispatchPackage();

  // Perform one step of simulation
  void simulateStep();

  // Advance simulation by one tick
  void advanceTick();

  // Show live fleet locations
  void displayDroneLocations() const;

  // Trigger storm (rollback active flights and redirect to pads)
  void triggerStorm();

  // Manual undo of last step for a specific drone
  bool undoLastStep(int droneID);

  // Add manual package
  void addManualPackage(int id, const string &src, const string &dest,
                        int srcNode, int destNode, int prio);

  // Getters for CLI rendering
  const DroneManager &getDroneMgr() const;
  DroneManager &getDroneMgrWritable();
  const PackageManager &getPackageMgr() const;
  PackageManager &getPackageMgrWritable();
  const AirspaceManager &getAirspaceMgr() const;
  const PadManager &getPadMgr() const;
  const FlightManager &getFlightMgr() const;
  const AddressService &getAddressService() const;
  const RouteService &getRouteService() const;
  const MissionManager &getMissionMgr() const;
  MissionManager &getMissionMgrWritable();
  const vector<ActiveFlight> &getActiveFlights() const;
  const vector<string> &getEventLog() const;
  int getSimulationTimeStep() const;

  void clearEventLog();
  void addEvent(const string &event);
};

#endif // SIMULATOR_H
