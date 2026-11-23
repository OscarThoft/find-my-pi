#ifndef UI_H
#define UI_H
#include <iostream>

class UI
{
public:
    bool startMenu();
    void showStarted();
    void showDistance(double d);
    void showLost();
    bool playAgain();
    void showFound(double RunningTime);
};

#endif