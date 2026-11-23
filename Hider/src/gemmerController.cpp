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

double gemmerController::requestDistanceData()
{
    return distanceData;
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
        double currDistance = requestDistanceData();
        




    }

}