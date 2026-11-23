#include "inc/RPI2_controller.h"

RPI2_controller::RPI2_controller()
    : rawData(0), distanceData(0.0)
{
}

void RPI2_controller::startGame()
{
    // send en ack til seeker
    initSPI();
    gamestatus.setStatus(gameStatus::Status::RUNNING);
}

double RPI2_controller::requestDistanceData()
{
    return distanceData;
}
void RPI2_controller::dataReady()
{
    // vent på interrupt
}

void RPI2_controller::buttonPressed()
{
    gamestatus.setStatus(gameStatus::Status::FOUND);
}

void RPI2_controller::initSPI()
{
    // kald SPI klassen
}

void RPI2_controller::run() {
    startGame();

    while(gameStatus::Status::RUNNING) {
        double currDistance = requestDistanceData();
        




    }

}