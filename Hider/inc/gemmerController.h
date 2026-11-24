#ifndef RPI2_CONTROLLER
#define RPI2_CONTROLLER
#include <iostream>
#include "inc/gameData.h"
#include "inc/gameStatus.h"
#include "../../server/server.h"

class gemmerController
{
public:
    struct status_distance {
        std::string state;
        double distance;
    };

    gemmerController();
    void startGame();
    status_distance requestData();
    void dataReady();
    void buttonPressed();
    void initSPI();
    void run();

    

private:
    int rawData;
    double distanceData;
    gameStatus gamestatus;
    gameData gamedata;
    GameHandler handler{gamestatus, gamedata, *this};

};


#endif