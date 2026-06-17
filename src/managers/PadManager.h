#ifndef PADMANAGER_H
#define PADMANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include "../models/ChargingPad.h"
#include "../algorithms/Dijkstra.h"
using namespace std;

class PadManager {
private:
    unordered_map<int, ChargingPad> pads;

public:
    PadManager();

    bool loadPadsFromFile(const string& filepath);
    bool savePadsToFile(const string& filepath) const;

    void registerPad(const ChargingPad& pad);
    
    // Find nearest available pad node
    // Returns padID, and sets outNodeID and outDistance
    int findNearestAvailablePad(
        int droneNode,
        const unordered_map<int, vector<Edge>>& graph,
        const unordered_set<int>& restrictedNodes,
        int& outNodeID,
        int& outDistance,
        bool quiet = false
    ) const;

    bool reservePad(int padID);
    void releasePad(int padID);
    void releasePadByNode(int nodeID);
    int getPadIDByNode(int nodeID) const;

    vector<ChargingPad> getAllPads() const;
    const ChargingPad* findPadConst(int padID) const;
};

#endif // PADMANAGER_H
