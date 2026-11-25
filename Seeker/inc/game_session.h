#pragma once

class game_session {
public:
    enum Status {
        INACTIVE,
        RUNNING,
        FOUND
    };

    game_session();

    void setStatus(Status s); //INACTIVE, RUNNING eller FOUND 
    Status getStatus() const;
    double getStartTime() const;
    double getEndTime() const;

private:
    Status status;
    double startTime = 0.0;
    double endTime = 0.0;

};