#include "Package.h"

using namespace std;

Package::Package()
    : packageID(-1), source(""), destination(""), sourceNodeID(-1),
      destNodeID(-1), priority(0), status("Pending") {}

Package::Package(int id, const string &src, const string &dest, int srcNode,
                 int destNode, int prio)
    : packageID(id), source(src), destination(dest), sourceNodeID(srcNode),
      destNodeID(destNode), priority(prio), status("Pending") {}

int Package::getPackageID() const { return packageID; }

string Package::getSource() const { return source; }

string Package::getDestination() const { return destination; }

int Package::getSourceNodeID() const { return sourceNodeID; }

int Package::getDestNodeID() const { return destNodeID; }

int Package::getPriority() const { return priority; }

string Package::getStatus() const { return status; }

void Package::markAssigned() { status = "In-Flight"; }

void Package::markDelivered() { status = "Delivered"; }

void Package::markReturned() { status = "Returned"; }

void Package::setStatus(const string &newStatus) { status = newStatus; }
