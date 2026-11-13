#ifndef RPI2_CONTROLLER
#define RPI2_CONTROLLER
#include <iostream>

class RPI2_controller
{
public:
    void startGame();
    double RequestDistanceData();
    void dataReady();
    void buttonPressed();
    void initSPI();

private:
    int rawData;
    double distanceData;
};