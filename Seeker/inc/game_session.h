#pragma once

class game_session {
public:
    enum Status {
        INACTIVE,
        RUNNING,
        FOUND,
        LOST
    };

    game_session();

    void setStatus(Status s); //INACTIVE, RUNNING, FOUND eller LOST
    Status getStatus() const;
    double getStartTime() const;
    double getEndTime() const;

private:
    Status status;
    double startTime = 0.0;
    double endTime = 0.0;

};