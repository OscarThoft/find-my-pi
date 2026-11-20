#pragma once
#include "SPIDriver.h"
#include "GPIOPin.h"
#include <thread>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <atomic>


class UWBDriver
{
private:
    SPIDriver &spi;  // SPI driver
    GPIOPin rst_pin; // reset pin
    GPIOPin irq_pin; // interrupt pin

    std::mutex SPImtx;           // mutex tages når der skal kommunikeres over SPI (undgår race condition mellem IRQThread og UWBDriver)
    std::thread IRQThread;       // separat thread, der tjekker for interrupts
    std::atomic<bool> runThread = false; // sættes true så længe vi vil tjekke for interrupts

    void interruptWorker(); // loop mens runThread er sat. check flaget hver x ms

    bool readRegister(uint8_t reg, uint16_t subAddr, uint8_t *dataOut, size_t len);        // læser data fra UWB
    bool writeRegister(uint8_t reg, uint16_t subAddr, const uint8_t *dataIn, size_t len);   // sender data til UWB

    // Hjaelpere: skriv/laes 1, 2 eller 4 bytes som et tal (little endian)
    bool write8(uint8_t reg, uint16_t subAddr, uint8_t value);
    bool write16(uint8_t reg, uint16_t subAddr, uint16_t value);
    bool write32(uint8_t reg, uint16_t subAddr, uint32_t value);
    bool read32(uint8_t reg, uint16_t subAddr, uint32_t &value);

public:
    // vi tager imod SPI-driveren samt GPIO-numrene til Reset og Interrupt
    UWBDriver(SPIDriver &spi_driver, unsigned int rst_num, unsigned int irq_num)
        : spi(spi_driver),
          rst_pin(rst_num, "out"), // reset starter som Output
          irq_pin(irq_num, "in")   // interrupt skal være et Input
    {}

    ~UWBDriver(){
        runThread = false; // stop interrupt worker
        if (IRQThread.joinable())
            IRQThread.join(); // vent på at interrupt worker er færdig
    };

    // Saetter chippen op. Skal kaldes hver gang programmet starter.
    // shortAddr = modulets egen adresse (fx 1 paa Gemmer, 2 paa Soeger)
    bool init(uint16_t shortAddr); // opsæt UWB

    void beginListen(); // kald interrupt worker og sæt atomic flaget

    bool getDistance(uint8_t reg, uint8_t *dataOut, size_t len); // læs afstand fra UWB. returnerer true hvis der er en ny afstand, ellers false
};