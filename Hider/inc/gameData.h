#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <cstdint>


class gameData{
    private:
    double distanceData;
    public:
    double ProcessData(int64_t t1, int64_t t2, int64_t t3, int64_t t4);
};

#endif