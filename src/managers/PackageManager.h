#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <queue>
#include <unordered_map>
#include <vector>
#include "../models/Package.h"
using namespace std;

class PackageManager {
private:
    queue<Package> dispatchQueue;
    unordered_map<int, Package> packageDb;

public:
    PackageManager();

    // Add package to dispatch queue and register in DB
    void enqueuePackage(const Package& package);

    // Get next package in line (FIFO)
    bool hasPendingPackages() const;
    Package getNextPackage(); // returns front and pops from queue

    // Find package by ID in DB
    Package* findPackage(int packageID);
    const Package* findPackageConst(int packageID) const;

    // Get all registered packages
    vector<Package> getAllPackages() const;
    
    // Get count of pending packages
    size_t getPendingCount() const;
};

#endif // PACKAGEMANAGER_H
