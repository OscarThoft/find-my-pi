#include "game_session.h"
#include <chrono>
#include <iostream>

game_session::game_session() : status(INACTIVE) {}

game_session::Status game_session::getStatus() const {
    return status;
}

void game_session::setStatus(Status s) {
    if (status == s) {
    return;
    }

    if(s == INACTIVE) {
        status = s;
        startTime = 0.0;
        endTime = 0.0;

    } else if(s == RUNNING) {
        status = s;
        endTime = 0.0;

        // Konverterer det aktuelle tidspunkt fra steady_clock til sekunder som double
        auto duration = std::chrono::duration<double> (
            std::chrono::steady_clock::now().time_since_epoch() 
        );

        startTime = duration.count(); //henter double værdien for tiden siden urets referencepunkt

    } else if(s == FOUND) {
        status = s;

        auto duration = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()
        );

        endTime = duration.count();

    } else if(s == LOST) {
        status = s;
    }   
}

double game_session::getTimeRunning() const {
    if (startTime == 0.0) {
        std::cout << "Failed: Time not started \n";
        return 0.0;
    }

    if(status == FOUND) {
        return endTime - startTime; //Returnere tiden det tog i sekunder
    }

    auto duration = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()
        );

    double currentTime = duration.count();

    return currentTime - startTime; //Returnere tiden det har taget foreløbigt

}