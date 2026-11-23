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
    char x;
    std::cout << "Find my pi\n";
    std::cout << "Press s to start, press q to quit\n";
    std::cin >> x;
    if (x == 's')
    {
        std::cout << "Game starting now...";
        return true;
    }
    else if (x == 'q')
    {
        std::cout << "Quiting...";
        return false;
    }
    else
    {
        std::cout << "Press s to start";
        return startMenu();
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
