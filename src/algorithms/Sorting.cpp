#include "Sorting.h"

using namespace std;

void Sorting::merge(vector<Drone>& drones, int left, int mid, int right, bool ascending) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Drone> L(n1);
    vector<Drone> R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = drones[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = drones[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        bool condition = false;
        if (ascending) {
            condition = L[i].getBattery() <= R[j].getBattery();
        } else {
            condition = L[i].getBattery() >= R[j].getBattery();
        }

        if (condition) {
            drones[k] = L[i];
            i++;
        } else {
            drones[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        drones[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        drones[k] = R[j];
        j++;
        k++;
    }
}

void Sorting::mergeSortHelper(vector<Drone>& drones, int left, int right, bool ascending) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(drones, left, mid, ascending);
        mergeSortHelper(drones, mid + 1, right, ascending);
        merge(drones, left, mid, right, ascending);
    }
}

void Sorting::mergeSortDrones(vector<Drone>& drones, bool ascending) {
    if (drones.empty()) return;
    mergeSortHelper(drones, 0, drones.size() - 1, ascending);
}

int Sorting::partition(vector<Drone>& drones, int low, int high, bool ascending) {
    float pivot = drones[high].getBattery();
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        bool condition = false;
        if (ascending) {
            condition = drones[j].getBattery() < pivot;
        } else {
            condition = drones[j].getBattery() > pivot;
        }

        if (condition) {
            i++;
            swap(drones[i], drones[j]);
        }
    }
    swap(drones[i + 1], drones[high]);
    return (i + 1);
}

void Sorting::quickSortHelper(vector<Drone>& drones, int low, int high, bool ascending) {
    if (low < high) {
        int pi = partition(drones, low, high, ascending);
        quickSortHelper(drones, low, pi - 1, ascending);
        quickSortHelper(drones, pi + 1, high, ascending);
    }
}

void Sorting::quickSortDrones(vector<Drone>& drones, bool ascending) {
    if (drones.empty()) return;
    quickSortHelper(drones, 0, drones.size() - 1, ascending);
}
