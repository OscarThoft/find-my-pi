#include "../inc/soegerController.h"
#include "../inc/game_session.h"
#include <iostream>
#include <chrono>

void soegerController::startGame() {
    initSPI(); 
    session.setStatus(game_session::RUNNING); //Set status til running
}

void soegerController::initSPI() {
    //Init for UWB (kommer senere)
}

void soegerController::found() {
    session.setStatus(game_session::FOUND);
    ui.showFound(calculateTime());
}

double soegerController::calculateTime() const {
    double startTime = session.getStartTime();
    double endTime = session.getEndTime();    

    if (startTime == 0.0) {
        std::cout << "Failed: Time not started \n";
        return 0.0;
    }


    if(session.getStatus() == game_session::FOUND) {
        return endTime - startTime; //Returnere tiden det tog i sekunder
    }

    auto duration = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()
        );

    double currentTime = duration.count();

    return currentTime - startTime; //Returnere tiden det har taget foreløbigt

}