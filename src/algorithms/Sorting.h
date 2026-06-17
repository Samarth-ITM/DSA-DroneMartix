#ifndef SORTING_H
#define SORTING_H

#include <vector>
#include "../models/Drone.h"

using namespace std;

class Sorting {
private:
    // Helper for Merge Sort
    static void merge(vector<Drone>& drones, int left, int mid, int right, bool ascending);
    static void mergeSortHelper(vector<Drone>& drones, int left, int right, bool ascending);

    // Helpers for Quick Sort
    static int partition(vector<Drone>& drones, int low, int high, bool ascending);
    static void quickSortHelper(vector<Drone>& drones, int low, int high, bool ascending);

public:
    // Merge Sort wrapper
    static void mergeSortDrones(vector<Drone>& drones, bool ascending = false);

    // Quick Sort wrapper
    static void quickSortDrones(vector<Drone>& drones, bool ascending = false);
};

#endif // SORTING_H
