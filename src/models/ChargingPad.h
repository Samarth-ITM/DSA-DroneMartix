#ifndef CHARGINGPAD_H
#define CHARGINGPAD_H
namespace std {}
using namespace std;

class ChargingPad {
private:
  int padID;
  int locationNode;
  bool occupied;

public:
  ChargingPad();
  ChargingPad(int id, int loc, bool occ);

  int getPadID() const;
  int getLocationNode() const;
  bool isOccupied() const;

  void occupy();
  void release();
};

#endif // CHARGINGPAD_H
