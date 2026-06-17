#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
using namespace std;

class Package {
private:
  int packageID;
  string source;
  string destination;
  int sourceNodeID;
  int destNodeID;
  int priority;
  string status; // "Pending", "In-Flight", "Delivered", "Returned"

public:
  Package();
  Package(int id, const string &src, const string &dest, int srcNode,
          int destNode, int prio);

  int getPackageID() const;
  string getSource() const;
  string getDestination() const;
  int getSourceNodeID() const;
  int getDestNodeID() const;
  int getPriority() const;
  string getStatus() const;

  void markAssigned();
  void markDelivered();
  void markReturned();
  void setStatus(const string &newStatus);
};

#endif // PACKAGE_H
