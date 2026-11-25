#include <iostream>
#include "gameData.h"

int main() {
    gameData data;

    //timestamps i pikosekunder
    Timestamps timestamps{
        1000,
        1200,
        2200,
        2610,
        3610,
        3810
    };


    //beregner distance
    data.ProcessData(timestamps);

    double distance = data.getDistanceData();
    std::cout << "Distance: " << distance << " meter" << std::endl;

    return 0;
}