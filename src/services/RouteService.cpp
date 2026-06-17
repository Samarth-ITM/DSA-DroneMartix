#include "RouteService.h"
#include <limits>

using namespace std;

RouteService::RouteService(const AirspaceManager &airspace,
                           const PadManager &pads,
                           const AddressService &addresses)
    : airspaceMgr(airspace), padMgr(pads), addressService(addresses) {}

int RouteService::mapCoordinateToNode(Coordinate coordinate) const {
  return addressService.findNodeByCoordinate(coordinate);
}

vector<int> RouteService::calculateRoute(int sourceNode, int destNode,
                                         int &outDistance) const {
  outDistance =
      Dijkstra::shortestDistance(sourceNode, destNode, airspaceMgr.getGraph(),
                                 airspaceMgr.getRestrictedNodes());
  if (outDistance == numeric_limits<int>::max()) {
    return vector<int>();
  }
  return Dijkstra::shortestPath(sourceNode, destNode, airspaceMgr.getGraph(),
                                airspaceMgr.getRestrictedNodes());
}

vector<int> RouteService::calculateEmergencyRoute(int currentNode,
                                                  int &outPadID,
                                                  int &outDistance) const {
  int padNodeID = -1;
  outPadID = padMgr.findNearestAvailablePad(currentNode, airspaceMgr.getGraph(),
                                            airspaceMgr.getRestrictedNodes(),
                                            padNodeID, outDistance);

  if (outPadID == -1 || padNodeID == -1) {
    return vector<int>();
  }

  return Dijkstra::shortestPath(currentNode, padNodeID, airspaceMgr.getGraph(),
                                airspaceMgr.getRestrictedNodes());
}

vector<int> RouteService::calculateReturnRoute(int currentNode,
                                               int &outDistance) const {
  // Find all warehouses
  vector<int> warehouses;
  const auto &graph = airspaceMgr.getGraph();
  for (const auto &pair : graph) {
    int nodeID = pair.first;
    if (airspaceMgr.getNodeType(nodeID) == 'W') {
      warehouses.push_back(nodeID);
    }
  }

  outDistance = numeric_limits<int>::max();
  int nearestWarehouse = -1;

  // Use Dijkstra's single source short distance to find nearest
  DijkstraResult res =
      Dijkstra::run(currentNode, graph, airspaceMgr.getRestrictedNodes());
  for (int w : warehouses) {
    auto it = res.distances.find(w);
    if (it != res.distances.end() && it->second < outDistance) {
      outDistance = it->second;
      nearestWarehouse = w;
    }
  }

  if (nearestWarehouse == -1 || outDistance == numeric_limits<int>::max()) {
    return vector<int>();
  }

  return Dijkstra::shortestPath(currentNode, nearestWarehouse, graph,
                                airspaceMgr.getRestrictedNodes());
}

float RouteService::calculateBatteryCost(const vector<int> &path,
                                         bool hasPayload) const {
  if (path.size() < 2)
    return 0.0f;

  float totalCost = 0.0f;
  const auto &graph = airspaceMgr.getGraph();

  for (size_t i = 0; i < path.size() - 1; ++i) {
    int u = path[i];
    int v = path[i + 1];

    int weight = 0;
    auto it = graph.find(u);
    if (it != graph.end()) {
      for (const auto &edge : it->second) {
        if (edge.destination == v) {
          weight = edge.weight;
          break;
        }
      }
    }

    int windPenalty = 3;
    int payloadPenalty = hasPayload ? 2 : 0;
    totalCost += (weight + windPenalty + payloadPenalty);
  }

  return totalCost;
}
