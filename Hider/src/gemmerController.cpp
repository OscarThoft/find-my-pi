#include "inc/gemmerController.h"

gemmerController::gemmerController()
    : rawData(0), distanceData(0.0)
{
}

void gemmerController::startGame()
{
    uwb.init();
    gamestatus.setStatus(gameStatus::Status::RUNNING);
    run();
}

double gemmerController::requestData()
{
    double distance = gamedata.getDistanceData();

HEAD
    
    return resp;
}


void gemmerController::dataReady()
{
    // vent på interrupt

    return distance;
 869f4dc7e2684e3e756ede81346ac22ab8b10b8b
}

void gemmerController::buttonPressed()
{
    handler.sendData("FOUND");
    gamestatus.setStatus(gameStatus::FOUND);
}

void gemmerController::initSPI()
{
    uwb.init();
}

void gemmerController::run()
{
    uwb.measure(t);
    bool flag = 1;
    while (flag)
    {
        while (gamestatus.getStatus() == gameStatus::RUNNING)
        {
            gamedata.ProcessData(&t);
            handler.sendData(gamedata.getDistanceData());
        }

        if (flag) //(handler.if play() again?) til sara mus
        {
            gamestatus.setStatus(gameStatus::RUNNING);
        }
        else
        {
            uwb.stopUWB();
            flag = 0;
        }
    }
}
