#ifndef ADDRESSSERVICE_H
#define ADDRESSSERVICE_H

#include "../models/Coordinate.h"
#include <string>
#include <unordered_map>
using namespace std;

class AddressService {
private:
  unordered_map<int, Coordinate> orderLookup;
  unordered_map<int, int> orderToNode; // maps orderID to graph node ID

public:
  AddressService();

  bool loadAddressesFromFile(const string &filepath);
  bool saveAddressesToFile(const string &filepath) const;

  void registerAddress(int orderID, double lat, double lon, int nodeID);

  // Convert customer order ID into GPS coordinates
  bool getCoordinates(int orderID, Coordinate &outCoord) const;
  Coordinate getCoordinates(int orderID) const;

  // Find node ID by coordinate match or closest distance
  int findNodeByCoordinate(Coordinate coord) const;

  // Get target node ID for order ID
  int getNodeID(int orderID) const;
};

#endif // ADDRESSSERVICE_H
