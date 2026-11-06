#ifndef GAME_STATUS_H
#define GAME_STATUS_H

class gameStatus
{
public:
    enum Status
    {
        INACTIVE,
        RUNNING,
        FOUND
    };
    gameStatus();
    void setStatus(Status s);
    Status getStatus() const;

private:
    Status status;
};

#endif