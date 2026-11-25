#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "timestamps.h"


class gameData{
    private:
    double distanceData;
    public:
    void ProcessData(const Timestamps& timestamps);
    double getDistanceData();
};

#endif