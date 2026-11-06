#include "SPIDriver.h"
#include "GPIOPin.h"
#include <thread>
#include <chrono>
#include <iostream>

class UWBDriver {
private:
    SPIDriver& spi;
    GPIOPin rst_pin;
    GPIOPin irq_pin;

public:
    // vi tager imod SPI-driveren samt GPIO-numrene til Reset og Interrupt
    UWBDriver(SPIDriver& spi_driver, int rst_num, int irq_num) 
        : spi(spi_driver), 
          rst_pin(rst_num, "in"),  // reset starter som Output
          irq_pin(irq_num, "in")    // interrupt skal være et Input
    {}

    // tjekker om UWB har data (Interrupt)
    bool dataReady() {
        // hvis IRQ-pinnen går HØJ, er data klar
        return irq_pin.read() == 1; 
    }
};