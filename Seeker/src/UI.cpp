#include "../inc/UI.h"
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

bool UI::startMenu()
{
    char s;
    std::cout << "Find my pi\n";
    std::cout << "Press s to start\n";
    std::cin >> s;
    if (s == 's')
    {
        return true;
        std::cout << "Game starting now...";
    }
    else
    {
        std::cout << "Press s to start";
    }
}

bool UI::playAgain()
{
    char yn;
    std::cout << "Do you want to play again? y/n\n";
    std::cin >> yn;

    if (yn == 'y')
    {
        std::cout << "Starting game...";
        return true;
    }
    else if (yn == 'n')
    {
        std::cout << "Thank you for playing <3";
        return false;
    }
    else
    {
        std::cout << "please press y or n";
        return playAgain();
    }
}

void UI::connectionFailed()
{
    std::cout << "Failed to connect with server..";
}
