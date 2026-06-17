#include "AirspaceManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

AirspaceManager::AirspaceManager() {}

bool AirspaceManager::loadCityMap(const string &filepath) {
  ifstream file(filepath);
  if (!file.is_open()) {
    cerr << "Error: Could not open city graph file " << filepath
              << endl;
    return false;
  }

  graph.clear();
  restrictedNodes.clear();
  nodeNames.clear();
  nodeTypes.clear();

  string line;
  while (getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    stringstream ss(line);
    string command;
    ss >> command;

    if (command == "NODE") {
      int nodeID;
      char type;
      string name;
      if (ss >> nodeID >> type >> name) {
        addNode(nodeID, type, name);
      }
    } else if (command == "EDGE") {
      int u, v, weight, capacity;
      if (ss >> u >> v >> weight >> capacity) {
        addEdge(u, v, weight, capacity);
      }
    }
  }
  file.close();
  return true;
}

void AirspaceManager::addNode(int nodeID, char type, const string &name) {
  nodeNames[nodeID] = name;
  nodeTypes[nodeID] = type;
  if (graph.find(nodeID) == graph.end()) {
    graph[nodeID] = vector<Edge>();
  }
  if (type == 'R') {
    markRestricted(nodeID);
  }
}

void AirspaceManager::addEdge(int u, int v, int weight, int capacity) {
  // Add forward edge
  if (graph.find(u) == graph.end())
    graph[u] = vector<Edge>();
  graph[u].push_back({v, weight, capacity, 0});

  // Add backward edge for undirected graph
  if (graph.find(v) == graph.end())
    graph[v] = vector<Edge>();
  graph[v].push_back({u, weight, capacity, 0});
}

void AirspaceManager::markRestricted(int nodeID) {
  restrictedNodes.insert(nodeID);
}

void AirspaceManager::removeRestricted(int nodeID) {
  restrictedNodes.erase(nodeID);
}

bool AirspaceManager::isRestricted(int nodeID) const {
  return restrictedNodes.count(nodeID) > 0;
}

bool AirspaceManager::reserveCorridorTraffic(int u, int v) {
  // Check u -> v
  auto it_u = graph.find(u);
  if (it_u == graph.end())
    return false;

  Edge *forwardEdge = nullptr;
  for (auto &edge : it_u->second) {
    if (edge.destination == v) {
      forwardEdge = &edge;
      break;
    }
  }

  // Check v -> u
  auto it_v = graph.find(v);
  if (it_v == graph.end())
    return false;

  Edge *backwardEdge = nullptr;
  for (auto &edge : it_v->second) {
    if (edge.destination == u) {
      backwardEdge = &edge;
      break;
    }
  }

  if (!forwardEdge || !backwardEdge) {
    return false; // Edge doesn't exist
  }

  if (forwardEdge->currentTraffic >= forwardEdge->capacity) {
    return false; // Congested
  }

  // Increment both since it's an undirected corridor
  forwardEdge->currentTraffic++;
  backwardEdge->currentTraffic++;
  return true;
}

void AirspaceManager::releaseCorridorTraffic(int u, int v) {
  // Decrement u -> v
  auto it_u = graph.find(u);
  if (it_u != graph.end()) {
    for (auto &edge : it_u->second) {
      if (edge.destination == v) {
        if (edge.currentTraffic > 0)
          edge.currentTraffic--;
        break;
      }
    }
  }

  // Decrement v -> u
  auto it_v = graph.find(v);
  if (it_v != graph.end()) {
    for (auto &edge : it_v->second) {
      if (edge.destination == u) {
        if (edge.currentTraffic > 0)
          edge.currentTraffic--;
        break;
      }
    }
  }
}

const unordered_map<int, vector<Edge>> &
AirspaceManager::getGraph() const {
  return graph;
}

const unordered_set<int> &AirspaceManager::getRestrictedNodes() const {
  return restrictedNodes;
}

string AirspaceManager::getNodeName(int nodeID) const {
  auto it = nodeNames.find(nodeID);
  if (it == nodeNames.end()) {
    return "Node_" + to_string(nodeID);
  }
  return it->second;
}

char AirspaceManager::getNodeType(int nodeID) const {
  auto it = nodeTypes.find(nodeID);
  if (it == nodeTypes.end()) {
    return 'C'; // default to Checkpoint
  }
  return it->second;
}

vector<int> AirspaceManager::getAllNodeIDs() const {
  vector<int> nodes;
  for (const auto &pair : graph) {
    nodes.push_back(pair.first);
  }
  return nodes;
}
