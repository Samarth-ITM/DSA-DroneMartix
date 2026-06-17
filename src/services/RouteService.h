#ifndef ROUTESERVICE_H
#define ROUTESERVICE_H

#include "../managers/AirspaceManager.h"
#include "../managers/PadManager.h"
#include "../services/AddressService.h"
#include <vector>
using namespace std;

class RouteService {
private:
  const AirspaceManager &airspaceMgr;
  const PadManager &padMgr;
  const AddressService &addressService;

public:
  RouteService(const AirspaceManager &airspace, const PadManager &pads,
               const AddressService &addresses);

  // Map a coordinate to the nearest delivery graph node
  int mapCoordinateToNode(Coordinate coordinate) const;

  // Calculate delivery route from warehouse to customer
  vector<int> calculateRoute(int sourceNode, int destNode,
                             int &outDistance) const;

  // Calculate emergency route from current node to nearest available charging
  // pad
  vector<int> calculateEmergencyRoute(int currentNode, int &outPadID,
                                      int &outDistance) const;

  // Calculate return route to the nearest warehouse/depot node
  vector<int> calculateReturnRoute(int currentNode, int &outDistance) const;

  // Calculate path battery consumption cost
  float calculateBatteryCost(const vector<int> &path,
                             bool hasPayload = true) const;
};

#endif // ROUTESERVICE_H
