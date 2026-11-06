#pragma once
#include <string>
#include <cstdint>

class SPIDriver {
private:
    int spi_fd = -1;
    uint32_t speed_hz;      // dataoverførselshastighed (clockspeed)
    uint8_t mode;           // sættes til mode 0. (inaktiv 0V, data klar ved rising edge)
    uint8_t bits_per_word;  // word i denne kontekst er karakter: 8 bits = 1 byte
    
    bool openSPI();
    void closeSPI();

public:
    SPIDriver(uint32_t speed, uint8_t mode = 0);
    ~SPIDriver();
    
    // sender og modtager data samtidigt (Full Duplex)
    bool transfer(uint8_t* tx_buf, uint8_t* rx_buf, size_t length);
};