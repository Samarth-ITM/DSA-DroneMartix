#include "ChargingPad.h"
using namespace std;

ChargingPad::ChargingPad() : padID(-1), locationNode(-1), occupied(false) {}

ChargingPad::ChargingPad(int id, int loc, bool occ)
    : padID(id), locationNode(loc), occupied(occ) {}

int ChargingPad::getPadID() const { return padID; }

int ChargingPad::getLocationNode() const { return locationNode; }

bool ChargingPad::isOccupied() const { return occupied; }

void ChargingPad::occupy() { occupied = true; }

void ChargingPad::release() { occupied = false; }
