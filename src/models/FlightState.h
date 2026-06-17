#ifndef FLIGHTSTATE_H
#define FLIGHTSTATE_H

#include <vector>
using namespace std;

struct FlightState {
  int currentNode;
  float battery;
  vector<int> path; // Remaining route path
};

#endif // FLIGHTSTATE_H
