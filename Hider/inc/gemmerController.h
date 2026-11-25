#ifndef RPI2_CONTROLLER
#define RPI2_CONTROLLER
#include <iostream>
#include "inc/gameData.h"
#include "inc/gameStatus.h"
#include "../../server/server.h"
#include "timestamps.h"

class gemmerController
{
public:
    gemmerController();
    void startGame();
    double requestData();
    void buttonPressed();
    void run();
    Timestamps t;

    void startServer();

private:
    gameStatus gamestatus;
    gameData gamedata;
    GameHandler handler{gamestatus, gamedata, *this};
    //UWBModule uwb; // erstat med rigtige
};

#endif