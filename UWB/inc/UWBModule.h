#pragma once

#include "SPIDriver.h"
#include "UWBDriver.h"
#include "UWBRanging.h"

#include <chrono>
#include <cstdint>

class UWBModule {
    private:
        SPIDriver spi;
        UWBDriver driver;
        UWBRanging ranging;
        uint16_t myAddr;

    public:
        UWBModule(uint16_t ownAddr, uint16_t peerAddr);

        bool init();
        bool measure(Timestamps &ts);
        bool respond(std::chrono::milliseconds timeout);
};
