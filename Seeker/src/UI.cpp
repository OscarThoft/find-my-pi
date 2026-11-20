#include "UI.h"
#include <iostream>

void UI::showStarted()
{
    std::cout << "Game started" << std::endl;
}

void UI::showDistance(double d)
{
    std::cout << "Distance: " << d << " Meters" << std::endl;
}

void UI::showLost()
{
    std::cout << "I'm lost... :( trying to reconnect...." << std::endl;
}

void UI::showFound(double RunningTime)
{
    std::cout << "Found in " << RunningTime << " Seconds" << std::endl;
}