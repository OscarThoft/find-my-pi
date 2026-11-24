#include "UWBModule.h"

constexpr const char *SPI_DEVICE = "/dev/spidev0.0";
constexpr uint32_t SPI_SPEED_HZ = 1000000;
constexpr uint8_t SPI_MODE = 0;
constexpr uint8_t SPI_BITS = 8;

UWBModule::UWBModule(uint16_t ownAddr, uint16_t peerAddr) :
    spi(SPI_DEVICE, SPI_SPEED_HZ, SPI_MODE, SPI_BITS),
    driver(spi),
    ranging(driver, ownAddr, peerAddr),
    myAddr(ownAddr) {}

bool UWBModule::init() {
    return driver.init(MyAddr);
}

bool UWBModule::measure(Timestamps &ts) {
    return ranging.measure(ts);
}

bool UWBModule::respond(std::chrono::milliseconds timeout) {
    return ranging.respond(timeout);
}