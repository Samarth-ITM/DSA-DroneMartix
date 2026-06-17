#include "MissionManager.h"
#include <iostream>
#include <limits>

using namespace std;

MissionManager::MissionManager() : nextMissionID(501) {} // Start mission IDs at 501 as in the example

int MissionManager::createMission(int droneID, int packageID, int src, int dest, const vector<int>& route, int cost, int estBat) {
    int mID = nextMissionID++;
    Mission m(mID, droneID, packageID, src, dest, route, cost, estBat);
    missions[mID] = m;
    return mID;
}

bool MissionManager::startMission(int missionID) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        it->second.status = "ACTIVE";
        return true;
    }
    return false;
}

bool MissionManager::updateMission(int missionID, const Mission& updatedMission) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        it->second = updatedMission;
        return true;
    }
    return false;
}

bool MissionManager::cancelMission(int missionID) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        it->second.status = "CANCELLED";
        
        // Auto-clean if count of inactive (ARCHIVED/CANCELLED) exceeds 50
        int inactiveCount = 0;
        int oldestID = numeric_limits<int>::max();
        for (const auto& pair : missions) {
            if (pair.second.status == "ARCHIVED" || pair.second.status == "CANCELLED") {
                inactiveCount++;
                if (pair.first < oldestID) {
                    oldestID = pair.first;
                }
            }
        }
        if (inactiveCount > 50 && oldestID != numeric_limits<int>::max()) {
            missions.erase(oldestID);
            cout << "Auto-clean: Removed oldest inactive mission #" << oldestID << " from table.\n";
        }
        return true;
    }
    return false;
}

bool MissionManager::completeMission(int missionID) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        it->second.status = "DELIVERED";
        return true;
    }
    return false;
}

bool MissionManager::archiveMission(int missionID) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        it->second.status = "ARCHIVED";
        
        // Auto-clean if count of inactive (ARCHIVED/CANCELLED) exceeds 50
        int inactiveCount = 0;
        int oldestID = numeric_limits<int>::max();
        for (const auto& pair : missions) {
            if (pair.second.status == "ARCHIVED" || pair.second.status == "CANCELLED") {
                inactiveCount++;
                if (pair.first < oldestID) {
                    oldestID = pair.first;
                }
            }
        }
        if (inactiveCount > 50 && oldestID != numeric_limits<int>::max()) {
            missions.erase(oldestID);
            cout << "Auto-clean: Removed oldest inactive mission #" << oldestID << " from table.\n";
        }
        return true;
    }
    return false;
}

bool MissionManager::removeMission(int missionID) {
    auto it = missions.find(missionID);
    if (it != missions.end()) {
        missions.erase(it);
        return true;
    }
    return false;
}

Mission* MissionManager::getMission(int missionID) {
    auto it = missions.find(missionID);
    if (it == missions.end()) {
        return nullptr;
    }
    return &(it->second);
}

const Mission* MissionManager::getMissionConst(int missionID) const {
    auto it = missions.find(missionID);
    if (it == missions.end()) {
        return nullptr;
    }
    return &(it->second);
}

vector<Mission> MissionManager::getAllMissions() const {
    vector<Mission> list;
    for (const auto& pair : missions) {
        list.push_back(pair.second);
    }
    return list;
}

vector<Mission> MissionManager::getMissionsByStatus(const string& status) const {
    vector<Mission> list;
    for (const auto& pair : missions) {
        if (pair.second.status == status) {
            list.push_back(pair.second);
        }
    }
    return list;
}
