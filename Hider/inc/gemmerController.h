#ifndef RPI2_CONTROLLER
#define RPI2_CONTROLLER
#include <iostream>
#include "inc/gameData.h"
#include "inc/gameStatus.h"

class gemmerController
{
public:
    gemmerController();
    void startGame();
    double requestDistanceData();
    void dataReady();
    void buttonPressed();
    void initSPI();
    void run();

private:
    int rawData;
    double distanceData;
    gameStatus gamestatus;
    gameData gamedata;
    GameHandler handler{gamestatus, gamedata};
};

#endif