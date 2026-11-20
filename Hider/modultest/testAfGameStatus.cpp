#include "gameStatus.h"
#include <iostream>

int main()
{
    gameStatus status;

    status.setStatus(gameStatus::RUNNING);

    std::cout << "Status er: " << status.getStatus() << std::endl;
}