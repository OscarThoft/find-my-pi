#include "gameData.h"
#include <cstdint>


void gameData::ProcessData(int64_t t1, int64_t t2, int64_t t3, int64_t t4){
    
    //udregner ToF. (T_round - T_reply)/2
    double ToF = ((t4 - t1) - (t3 - t2)) / 2.0;
    //sætter ToF fra pikosekunder til sekunder
    ToF *= 1e-12;
    //speed of light
    constexpr double c = 299'792'458.0; // m/s

    //distance udregning
    distanceData = c * ToF;
};

double gameData::getDistanceData(){
    return distanceData;
}