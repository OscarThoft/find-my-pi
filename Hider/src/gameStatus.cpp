#include "gameStatus.h"

void gameStatus::setStatus(Status s)
{
    status = s;
}

gameStatus::Status gameStatus::getStatus() const
{
    return status;
}

gameStatus::gameStatus()
    : status(INACTIVE)
{
}