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
    // Vi tager imod SPI-driveren samt GPIO-numrene til Reset og Interrupt
    UWBDriver(SPIDriver& spi_driver, int rst_num, int irq_num) 
        : spi(spi_driver), 
          rst_pin(rst_num, "out"),  // Reset skal være et Output
          irq_pin(irq_num, "in")    // Interrupt skal være et Input
    {
        // 1. Initialiser GPIO-pins
        rst_pin.init();
        irq_pin.init();
    }

    // Denne kaldes i starten af dit program efter rawSPI er oprettet
    bool initializeModule() {
        std::cout << "Starter hardware reset af UWB-modul..." << std::endl;

        // 2. TRÆK RESET LAV (0)
        rst_pin.write(0); 
        
        // 3. VENT I 20 MILLISEKUNDER
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        // 4. FRIGIV RESET / TRÆK DEN HØJ (1)
        rst_pin.write(1);

        // 5. VENT PÅ CHIPPENS INTERNE KRYSTAL (Typisk 5-10 ms er nok, vi giver den 15 ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        
        std::cout << "UWB-modul er resat og klar til SPI-konfiguration." << std::endl;

        // Her kan du nu lave dine første SPI-kald, f.eks. læse Device ID
        // for at bekræfte at chippen overhovedet har overlevet sit reset.
        return true;
    }

    // Funktion til at tjekke om modulet råber på opmærksomhed (Interrupt)
    bool hasDataToRead() {
        // Hvis IRQ-pinnen går HØJ (eller LAV, afhængig af chippens konfiguration),
        // så betyder det, at der er data klar!
        return irq_pin.read() == 1; 
    }
};
