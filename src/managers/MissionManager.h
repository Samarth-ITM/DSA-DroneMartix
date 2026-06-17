#ifndef MISSIONMANAGER_H
#define MISSIONMANAGER_H

#include <unordered_map>
#include <vector>
#include "../models/Mission.h"
using namespace std;

class MissionManager {
private:
    unordered_map<int, Mission> missions;
    int nextMissionID;

public:
    MissionManager();

    // Create and store a new mission
    int createMission(int droneID, int packageID, int src, int dest, const vector<int>& route, int cost, int estBat);
    
    // Mission states
    bool startMission(int missionID);
    bool updateMission(int missionID, const Mission& updatedMission);
    bool cancelMission(int missionID);
    bool completeMission(int missionID);
    bool archiveMission(int missionID);
    bool removeMission(int missionID);

    // Getters
    Mission* getMission(int missionID);
    const Mission* getMissionConst(int missionID) const;
    vector<Mission> getAllMissions() const;
    vector<Mission> getMissionsByStatus(const string& status) const;
};

#endif // MISSIONMANAGER_H
