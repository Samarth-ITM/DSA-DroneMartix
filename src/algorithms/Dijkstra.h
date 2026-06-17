#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct Edge {
  int destination;
  int weight;
  int capacity;
  int currentTraffic;
};

struct DijkstraResult {
  unordered_map<int, int> distances;    // nodeID -> shortest distance
  unordered_map<int, int> predecessors; // nodeID -> previous node in path
};

class Dijkstra {
public:
  // Run Dijkstra's algorithm from a source node
  static DijkstraResult
  run(int source, const unordered_map<int, vector<Edge>> &graph,
      const unordered_set<int> &restrictedNodes);

  // Get shortest path from source to destination
  static vector<int>
  shortestPath(int source, int destination,
               const unordered_map<int, vector<Edge>> &graph,
               const unordered_set<int> &restrictedNodes);

  // Get shortest distance from source to destination
  static int
  shortestDistance(int source, int destination,
                   const unordered_map<int, vector<Edge>> &graph,
                   const unordered_set<int> &restrictedNodes);

  // Find nearest charging pad among the given location nodes
  static int
  nearestChargingPad(int droneNode, const vector<int> &padNodes,
                     const unordered_map<int, vector<Edge>> &graph,
                     const unordered_set<int> &restrictedNodes,
                     int &outDistance);
};

#endif // DIJKSTRA_H
