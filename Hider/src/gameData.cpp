#include "gameData.h"
#include <cstdint>
#include <iostream>


void gameData::ProcessData(const Timestamps& timestamps){
    //tjekker om tidsværdierne ikke er invalide tal
    if (timestamps.t1 <= 0 || timestamps.t2 <= 0 || timestamps.t3 <= 0 || timestamps.t4 <= 0 || timestamps.t5 <= 0 || timestamps.t6 <= 0)
    {
        std::cout << "Fejl: Alle tidsværdier skal være positive." << std::endl;
        return;
    }

    //tjekker om tidsværdierne bliver sendt i rigtig rækkefølge
    if (timestamps.t1 >= timestamps.t2 || timestamps.t2 >= timestamps.t3 || timestamps.t3 >= timestamps.t4 || timestamps.t4 >= timestamps.t5 || timestamps.t5 >= timestamps.t6)
    {
        std::cout << "Fejl: Tidsværdierne skal være i stigende rækkefølge." << std::endl;
        return;
    }

    //40 bit maske
   constexpr uint64_t MASK_40BIT = 0x000000FFFFFFFFFFULL;

    //træk fra i heltal og mask til 40 bit
    uint64_t T_round_G_raw = (timestamps.t4 - timestamps.t1) & MASK_40BIT;
    uint64_t T_reply_G_raw = (timestamps.t5 - timestamps.t4) & MASK_40BIT;
    uint64_t T_round_S_raw = (timestamps.t6 - timestamps.t3) & MASK_40BIT;
    uint64_t T_reply_S_raw = (timestamps.t3 - timestamps.t2) & MASK_40BIT;

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