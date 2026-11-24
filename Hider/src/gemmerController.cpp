#include "inc/gemmerController.h"

gemmerController::gemmerController()
    : rawData(0), distanceData(0.0)
{
}

void gemmerController::startGame()
{
    // send en ack til seeker
    initSPI();

    gamestatus.setStatus(gameStatus::Status::RUNNING);
}

gemmerController::status_distance gemmerController::requestData()
{
    status_distance resp;
    resp.state = std::to_string(gamestatus.getStatus());
    resp.distance = gamedata.getDistanceData();

    return resp;
}


void gemmerController::dataReady()
{
    // vent på interrupt
}

void gemmerController::buttonPressed()
{
    gamestatus.setStatus(gameStatus::Status::FOUND);
}

void gemmerController::initSPI()
{
    // kald SPI klassen
}

void gemmerController::run() {
    startGame();

    while(gameStatus::Status::RUNNING) {
        
        




    }

}