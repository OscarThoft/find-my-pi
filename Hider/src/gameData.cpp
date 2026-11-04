#include "gameData.h"

double gameData::ProcessData(double t1, double t2, double t3, double t4){
    double times[] = {t1, t2, t3, t4};
    //sætter tiderne fra pikosekunder til sekunder
    for (int i = 0; i < 4; ++i) {
        times[i] *= 1e-12;
    }
    //udregner ToF
    double ToF = ((times[3] - times[0]) - (times[2] - times[1])) / 2.0;
    //speed of light
    constexpr double c = 299'792'458.0; // m/s

    //distance udregning
    distanceData = c * ToF;
    return distanceData;
};