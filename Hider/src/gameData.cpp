#include "gameData.h"
#include <cstdint>
#include <bit>


void gameData::ProcessData(int64_t t1, int64_t t2, int64_t t3, int64_t t4, int64_t t5, int64_t t6){
    //40 bit maske
   constexpr uint64_t MASK_40BIT = 0x000000FFFFFFFFFFULL;

    //træk fra i heltal og mask til 40 bit
    uint64_t T_round_G_raw = (t4 - t1) & MASK_40BIT;
    uint64_t T_reply_G_raw = (t5 - t4) & MASK_40BIT;
    uint64_t T_round_S_raw = (t6 - t3) & MASK_40BIT;
    uint64_t T_reply_S_raw = (t3 - t2) & MASK_40BIT;

    //konverter derefter til double
    double T_round_G = static_cast<double>(T_round_G_raw);
    double T_reply_G = static_cast<double>(T_reply_G_raw);
    double T_round_S = static_cast<double>(T_round_S_raw);
    double T_reply_S = static_cast<double>(T_reply_S_raw);

    //udregner ToF 
    double ToF = (T_round_G * T_round_S - T_reply_G *T_reply_S)/(T_round_G + T_round_S + T_reply_G + T_reply_S);
    
    //speed of light i ps
    constexpr double c = 0.000299792458; // m/ps

    //ganger 15.65ps på for at omregne fra tidstempler til actual distance
    constexpr double TICK_TO_PS = 15.6500400641;

    //distance udregning
    distanceData = c * ToF * TICK_TO_PS;
};

double gameData::getDistanceData(){
    return distanceData;
}