#pragma once
#include <string>
#include <cstdint>

class SPIDriver {
private:
    int spi_fd = -1;
    uint32_t speed_hz;
    uint8_t mode;
    uint8_t bits_per_word;
    
    bool openSPI();
    void closeSPI();

public:
    // device eks: "/dev/spidev0.0", speed eks: 2000000 (2MHz), mode eks: 0
    SPIDriver(uint32_t speed, uint8_t mode = 0);
    ~SPIDriver();
    
    // Den vigtigste funktion: Sender og modtager data samtidigt (Full Duplex)
    bool transfer(uint8_t* tx_buf, uint8_t* rx_buf, size_t length);
};
