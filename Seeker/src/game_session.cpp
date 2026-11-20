#include "game_session.h"
#include <chrono>

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

double game_session::getStartTime() const {
    return startTime;
}

double game_session::getEndTime() const {
    return endTime;
}
