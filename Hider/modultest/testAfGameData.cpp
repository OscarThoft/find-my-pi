#include <iostream>
#include "gameData.h"

int main() {
    gameData data;

    //timestamps i pikosekunder
    int64_t t1 = 1000;
    int64_t t2 = 1200;
    int64_t t3 = 2200;
    int64_t t4 = 2400;
    int64_t t5 = 2600;
    int64_t t6 = 3000;


    //beregner distance
    data.ProcessData(t1, t2, t3, t4, t5, t6);

    double distance = data.getDistanceData();
    std::cout << "Distance: " << distance << " meter" << std::endl;

    return 0;
}