#ifndef AIRSPACEMANAGER_H
#define AIRSPACEMANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "../algorithms/Dijkstra.h" // Includes Edge definition
using namespace std;

class AirspaceManager {
private:
    unordered_map<int, vector<Edge>> graph;
    unordered_set<int> restrictedNodes;
    unordered_map<int, string> nodeNames;
    unordered_map<int, char> nodeTypes; // 'W' = Warehouse, 'C' = Checkpoint, 'D' = Delivery, 'P' = Charging Pad, 'R' = Restricted

public:
    AirspaceManager();

    bool loadCityMap(const string& filepath);
    
    void addNode(int nodeID, char type, const string& name);
    void addEdge(int u, int v, int weight, int capacity);
    
    void markRestricted(int nodeID);
    void removeRestricted(int nodeID);
    bool isRestricted(int nodeID) const;

    // Traffic reservation on flight corridor
    bool reserveCorridorTraffic(int u, int v);
    void releaseCorridorTraffic(int u, int v);

    // Getters
    const unordered_map<int, vector<Edge>>& getGraph() const;
    const unordered_set<int>& getRestrictedNodes() const;
    string getNodeName(int nodeID) const;
    char getNodeType(int nodeID) const;
    
    vector<int> getAllNodeIDs() const;
};

#endif // AIRSPACEMANAGER_H
