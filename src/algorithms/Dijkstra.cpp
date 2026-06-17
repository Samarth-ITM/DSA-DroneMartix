#include "Dijkstra.h"
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;

DijkstraResult Dijkstra::run(
    int source,
    const unordered_map<int, vector<Edge>>& graph,
    const unordered_set<int>& restrictedNodes
) {
    DijkstraResult result;
    
    // Min-heap for Dijkstra: pair<distance, nodeID>
    priority_queue<
        pair<int, int>, 
        vector<pair<int, int>>, 
        greater<pair<int, int>>
    > pq;

    // Initialize distances
    for (const auto& pair : graph) {
        result.distances[pair.first] = numeric_limits<int>::max();
    }
    
    // Check if source node is valid and not restricted
    if (graph.find(source) == graph.end() || restrictedNodes.count(source)) {
        return result;
    }

    result.distances[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [dist, u] = pq.top();
        pq.pop();

        if (dist > result.distances[u]) {
            continue;
        }

        auto it = graph.find(u);
        if (it == graph.end()) {
            continue;
        }

        for (const auto& edge : it->second) {
            int v = edge.destination;

            // Skip restricted zones
            if (restrictedNodes.count(v)) {
                continue;
            }

            // Skip congested corridors
            if (edge.currentTraffic >= edge.capacity) {
                continue;
            }

            int newDist = dist + edge.weight;
            if (newDist < result.distances[v]) {
                result.distances[v] = newDist;
                result.predecessors[v] = u;
                pq.push({newDist, v});
            }
        }
    }

    return result;
}

vector<int> Dijkstra::shortestPath(
    int source,
    int destination,
    const unordered_map<int, vector<Edge>>& graph,
    const unordered_set<int>& restrictedNodes
) {
    vector<int> path;
    
    // Check if source or destination is restricted
    if (restrictedNodes.count(source) || restrictedNodes.count(destination)) {
        return path;
    }

    DijkstraResult result = run(source, graph, restrictedNodes);

    if (result.distances.find(destination) == result.distances.end() || 
        result.distances[destination] == numeric_limits<int>::max()) {
        return path; // No path exists
    }

    // Reconstruct path
    int curr = destination;
    while (curr != source) {
        path.push_back(curr);
        auto it = result.predecessors.find(curr);
        if (it == result.predecessors.end()) {
            return vector<int>(); // Corrupted path trace
        }
        curr = it->second;
    }
    path.push_back(source);
    reverse(path.begin(), path.end());

    return path;
}

int Dijkstra::shortestDistance(
    int source,
    int destination,
    const unordered_map<int, vector<Edge>>& graph,
    const unordered_set<int>& restrictedNodes
) {
    if (restrictedNodes.count(source) || restrictedNodes.count(destination)) {
        return numeric_limits<int>::max();
    }

    DijkstraResult result = run(source, graph, restrictedNodes);
    
    auto it = result.distances.find(destination);
    if (it == result.distances.end()) {
        return numeric_limits<int>::max();
    }
    return it->second;
}

int Dijkstra::nearestChargingPad(
    int droneNode,
    const vector<int>& padNodes,
    const unordered_map<int, vector<Edge>>& graph,
    const unordered_set<int>& restrictedNodes,
    int& outDistance
) {
    outDistance = numeric_limits<int>::max();
    int nearestPadNode = -1;

    if (padNodes.empty()) {
        return -1;
    }

    DijkstraResult result = run(droneNode, graph, restrictedNodes);

    for (int padNode : padNodes) {
        auto it = result.distances.find(padNode);
        if (it != result.distances.end() && it->second < outDistance) {
            outDistance = it->second;
            nearestPadNode = padNode;
        }
    }

    return nearestPadNode;
}
