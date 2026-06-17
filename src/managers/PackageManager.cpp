#include "PackageManager.h"

using namespace std;

PackageManager::PackageManager() {}

void PackageManager::enqueuePackage(const Package &package) {
  packageDb[package.getPackageID()] = package;
  dispatchQueue.push(package);
}

bool PackageManager::hasPendingPackages() const {
  return !dispatchQueue.empty();
}

Package PackageManager::getNextPackage() {
  if (dispatchQueue.empty()) {
    return Package();
  }
  Package pkg = dispatchQueue.front();
  dispatchQueue.pop();

  // Update package status in database to assigned
  auto it = packageDb.find(pkg.getPackageID());
  if (it != packageDb.end()) {
    it->second.markAssigned();
    pkg = it->second;
  }
  return pkg;
}

Package *PackageManager::findPackage(int packageID) {
  auto it = packageDb.find(packageID);
  if (it == packageDb.end()) {
    return nullptr;
  }
  return &(it->second);
}

const Package *PackageManager::findPackageConst(int packageID) const {
  auto it = packageDb.find(packageID);
  if (it == packageDb.end()) {
    return nullptr;
  }
  return &(it->second);
}

vector<Package> PackageManager::getAllPackages() const {
  vector<Package> list;
  for (const auto &pair : packageDb) {
    list.push_back(pair.second);
  }
  return list;
}

size_t PackageManager::getPendingCount() const { return dispatchQueue.size(); }
