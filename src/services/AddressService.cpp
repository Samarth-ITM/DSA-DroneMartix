#include "AddressService.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

AddressService::AddressService() {}

bool AddressService::loadAddressesFromFile(const string &filepath) {
  ifstream file(filepath);
  if (!file.is_open()) {
    cerr << "Error: Could not open addresses file " << filepath << endl;
    return false;
  }

  orderLookup.clear();
  orderToNode.clear();

  string line;
  while (getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    stringstream ss(line);
    int orderID;
    double lat;
    double lon;
    int nodeID;

    if (ss >> orderID >> lat >> lon >> nodeID) {
      registerAddress(orderID, lat, lon, nodeID);
    }
  }
  file.close();
  return true;
}

bool AddressService::saveAddressesToFile(const string &filepath) const {
  ofstream file(filepath);
  if (!file.is_open()) {
    cerr << "Error: Could not write to addresses file " << filepath << endl;
    return false;
  }

  file << "# orderID latitude longitude nodeID\n";
  for (const auto &pair : orderLookup) {
    int orderID = pair.first;
    const Coordinate &coord = pair.second;
    int nodeID = -1;
    auto it = orderToNode.find(orderID);
    if (it != orderToNode.end()) {
      nodeID = it->second;
    }
    file << orderID << " " << coord.latitude << " " << coord.longitude << " "
         << nodeID << "\n";
  }
  file.close();
  return true;
}

void AddressService::registerAddress(int orderID, double lat, double lon,
                                     int nodeID) {
  Coordinate coord = {lat, lon};
  orderLookup[orderID] = coord;
  orderToNode[orderID] = nodeID;
}

bool AddressService::getCoordinates(int orderID, Coordinate &outCoord) const {
  auto it = orderLookup.find(orderID);
  if (it == orderLookup.end()) {
    return false;
  }
  outCoord = it->second;
  return true;
}

Coordinate AddressService::getCoordinates(int orderID) const {
  auto it = orderLookup.find(orderID);
  if (it == orderLookup.end()) {
    return {0.0, 0.0};
  }
  return it->second;
}

int AddressService::findNodeByCoordinate(Coordinate coord) const {
  double minDistance = 999999999.0;
  int bestNode = -1;
  for (const auto &pair : orderLookup) {
    int orderID = pair.first;
    const Coordinate &registered = pair.second;
    double dy = registered.latitude - coord.latitude;
    double dx = registered.longitude - coord.longitude;
    double dist = dy * dy + dx * dx;
    if (dist < minDistance) {
      minDistance = dist;
      bestNode = getNodeID(orderID);
    }
  }
  return bestNode;
}

int AddressService::getNodeID(int orderID) const {
  auto it = orderToNode.find(orderID);
  if (it == orderToNode.end()) {
    return -1;
  }
  return it->second;
}
