#include "PadManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

PadManager::PadManager() {}

bool PadManager::loadPadsFromFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not open pads file " << filepath << endl;
        return false;
    }

    pads.clear();
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        stringstream ss(line);
        int padID;
        int locationNode;
        int occupiedVal;

        if (ss >> padID >> locationNode >> occupiedVal) {
            ChargingPad pad(padID, locationNode, occupiedVal == 1);
            pads[padID] = pad;
        }
    }
    file.close();
    return true;
}

bool PadManager::savePadsToFile(const string& filepath) const {
    ofstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Could not write to pads file " << filepath << endl;
        return false;
    }

    file << "# padID locationNode occupied\n";
    for (const auto& pair : pads) {
        const ChargingPad& pad = pair.second;
        file << pad.getPadID() << " " 
             << pad.getLocationNode() << " " 
             << (pad.isOccupied() ? 1 : 0) << "\n";
    }
    file.close();
    return true;
}

void PadManager::registerPad(const ChargingPad& pad) {
    pads[pad.getPadID()] = pad;
}

int PadManager::findNearestAvailablePad(
    int droneNode,
    const unordered_map<int, vector<Edge>>& graph,
    const unordered_set<int>& restrictedNodes,
    int& outNodeID,
    int& outDistance,
    bool quiet
) const {
    vector<ChargingPad> sortedPads;
    for (const auto& pair : pads) {
        sortedPads.push_back(pair.second);
    }
    sort(sortedPads.begin(), sortedPads.end(), [](const ChargingPad& a, const ChargingPad& b) {
        return a.getPadID() < b.getPadID();
    });

    DijkstraResult res = Dijkstra::run(droneNode, graph, restrictedNodes);

    int selectedPadID = -1;
    int minDistance = numeric_limits<int>::max();
    int minNodeID = -1;

    for (const auto& pad : sortedPads) {
        if (!quiet) {
            cout << "Pad " << pad.getPadID() << "\n\n";
        }
        if (pad.isOccupied()) {
            if (!quiet) {
                cout << "Occupied\n\nIgnored\n\n-------------------\n\n";
            }
            continue;
        }

        int node = pad.getLocationNode();
        auto it = res.distances.find(node);
        int dist = (it != res.distances.end()) ? it->second : numeric_limits<int>::max();

        if (dist == numeric_limits<int>::max()) {
            if (!quiet) {
                cout << "Distance: unreachable\n\nIgnored\n\n-------------------\n\n";
            }
        } else {
            if (!quiet) {
                cout << "Distance: " << dist << "\n\nCandidate\n\n-------------------\n\n";
            }
            if (dist < minDistance) {
                minDistance = dist;
                minNodeID = node;
                selectedPadID = pad.getPadID();
            }
        }
    }

    if (selectedPadID != -1) {
        if (!quiet) {
            cout << "Selected Pad: " << selectedPadID << "\n";
        }
        outNodeID = minNodeID;
        outDistance = minDistance;
        return selectedPadID;
    }

    outNodeID = -1;
    outDistance = -1;
    return -1;
}

bool PadManager::reservePad(int padID) {
    auto it = pads.find(padID);
    if (it != pads.end() && !it->second.isOccupied()) {
        it->second.occupy();
        return true;
    }
    return false;
}

void PadManager::releasePad(int padID) {
    auto it = pads.find(padID);
    if (it != pads.end()) {
        it->second.release();
    }
}

void PadManager::releasePadByNode(int nodeID) {
    for (auto& pair : pads) {
        if (pair.second.getLocationNode() == nodeID) {
            pair.second.release();
            break;
        }
    }
}

int PadManager::getPadIDByNode(int nodeID) const {
    for (const auto& pair : pads) {
        if (pair.second.getLocationNode() == nodeID) {
            return pair.second.getPadID();
        }
    }
    return -1;
}

vector<ChargingPad> PadManager::getAllPads() const {
    vector<ChargingPad> list;
    for (const auto& pair : pads) {
        list.push_back(pair.second);
    }
    return list;
}

const ChargingPad* PadManager::findPadConst(int padID) const {
    auto it = pads.find(padID);
    if (it == pads.end()) {
        return nullptr;
    }
    return &(it->second);
}
