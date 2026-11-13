#ifndef RPI2_CONTROLLER
#define RPI2_CONTROLLER
#include <iostream>
#include "inc/gameData.h"
#include "inc/gameStatus.h"

class RPI2_controller
{
public:
    RPI2_controller();
    void startGame();
    double RequestDistanceData();
    void dataReady();
    void buttonPressed();
    void initSPI();

private:
    int rawData;
    double distanceData;
    gameStatus gamestatus;
    gameData gamedata;
};

#endif