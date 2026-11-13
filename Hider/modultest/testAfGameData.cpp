#include <iostream>
#include "gameData.h"

int main() {
    gameData data;

    //timestamps i pikosekunder
    int64_t t1 = 1000;
    int64_t t2 = 1200;
    int64_t t3 = 2200;
    int64_t t4 = 2600;

    //beregner distance
    double distance = data.ProcessData(t1, t2, t3, t4);

    std::cout << "Distance: " << distance << " meter" << std::endl;

    return 0;
}