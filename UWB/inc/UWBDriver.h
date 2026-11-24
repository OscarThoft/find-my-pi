#pragma once
#include "SPIDriver.h"
#include <chrono>
#include <cstddef>
#include <cstdint>

// Driver til DW1000 (UWB Click) med polling: ingen interrupt, ingen tråd.
// Pi'en læser selv SYS_STATUS over SPI for at se, hvornår en frame er sendt eller modtaget.
class UWBDriver
{
private:
    SPIDriver &spi;

    // Rå registeradgang. subAddr = offset inde i registret (0 = fra starten)
    bool readRegister(uint8_t reg, uint8_t *dataOut, size_t len, uint16_t subAddr);
    bool writeRegister(uint8_t reg, const uint8_t *dataIn, size_t len, uint16_t subAddr);

    // Skriv/læs et tal (little endian). Typen T bestemmer antal bytes og skal angives:
    //   writeValue<uint16_t>(reg, sub, 0x8870)   skriver 2 bytes
    //   readValue<uint64_t>(reg, sub, t, 5)      læser 5 bytes (40-bit tidsstempel)
    template <typename T>
    bool writeValue(uint8_t reg, uint16_t subAddr, T value);
    template <typename T>
    bool readValue(uint8_t reg, uint16_t subAddr, T &value, size_t bytes = sizeof(T));

    // Polling: læser SYS_STATUS, til en af bittene i mask er sat, eller tiden er gået
    bool waitForStatus(uint32_t mask, std::chrono::microseconds timeout, uint32_t &status);

    void stopReceive();   // sluk sender/modtager og nulstil modtageren
    void resetReceiver(); // nulstil modtageren efter fejl

public:
    static constexpr size_t MAX_FRAME_LEN = 125; // data-bytes i en frame (uden de 2 CRC-bytes)

    explicit UWBDriver(SPIDriver &spi_driver) : spi(spi_driver) {}

    UWBDriver(const UWBDriver &) = delete;
    UWBDriver &operator=(const UWBDriver &) = delete;

    // Sætter chippen op. Kaldes hver gang programmet starter.
    // shortAddr = modulets egen adresse (fx 1 på Gemmer, 2 på Søger)
    bool init(uint16_t shortAddr);

    // Sender en frame og venter, til den er sendt.
    // expectResponse = true: chippen tænder selv modtageren lige efter sendingen.
    // txTime = chippens tidsstempel for sendingen (40 bit).
    bool sendFrame(const uint8_t *data, size_t len, bool expectResponse, uint64_t &txTime);

    // Tænder modtageren. Bruges, når man vil lytte uden at have sendt først.
    bool startReceive();

    // Venter på en frame (modtageren skal være tændt). Returnerer true, når en frame
    // med korrekt CRC er modtaget; false ved timeout. Efter kaldet er modtageren slukket.
    // len = antal data-bytes, rxTime = chippens tidsstempel for modtagelsen (40 bit).
    bool receiveFrame(uint8_t *data, size_t maxLen, size_t &len, uint64_t &rxTime,
                      std::chrono::milliseconds timeout);
};