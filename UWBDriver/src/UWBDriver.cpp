#include "UWBDriver.h"
#include <iostream>
#include <array>
#include <cstdint>
#include <cstring>
#include <chrono>

constexpr uint32_t EXPECTED_DEV_ID = 0xDECA0130;
constexpr size_t MAX_DATA_LEN = 128;

// SYS_STATUS (register 0x0F), 5 bytes, de relevante bit ligger i de 4 nederste
constexpr uint8_t REG_SYS_STATUS = 0x0F;

constexpr uint32_t SYS_STATUS_TXFRS = 0x00000080;  // bit 7:  frame sendt
constexpr uint32_t SYS_STATUS_RXDFR = 0x00002000;  // bit 13: frame modtaget
constexpr uint32_t SYS_STATUS_RXFCG = 0x00004000;  // bit 14: CRC ok
constexpr uint32_t SYS_STATUS_RXFCE = 0x00008000;  // bit 15: CRC-fejl
constexpr uint32_t SYS_STATUS_RXRFTO = 0x00020000; // bit 17: timeout

void UWBDriver::interruptWorker()
{
    while (runThread)
    {
        // Vent på IRQ; ved timeout tjekkes runThread igen
        if (!irq_pin.waitForInterrupt(std::chrono::milliseconds(100)))
            continue;

        // 1. Læs SYS_STATUS og saml de 4 bytes (little endian)
        std::array<uint8_t, 4> raw{};
        if (!readRegister(REG_SYS_STATUS, 0, raw.data(), raw.size()))
            continue;

        const uint32_t status = static_cast<uint32_t>(raw[0]) |
                                (static_cast<uint32_t>(raw[1]) << 8) |
                                (static_cast<uint32_t>(raw[2]) << 16) |
                                (static_cast<uint32_t>(raw[3]) << 24);

        uint32_t handled = 0; // de bit, vi rydder til sidst

        // 2. Frame sendt
        if (status & SYS_STATUS_TXFRS)
        {
            // læs TX_TIME (t1 hos Gemmer, t3 hos Søger)
            handled |= SYS_STATUS_TXFRS;
        }

        // 3. Frame modtaget med korrekt CRC
        if (status & SYS_STATUS_RXFCG)
        {
            // læs længde fra RX_FINFO, frame fra RX_BUFFER, tidsstempel fra RX_TIME
            // læg resultatet i køen til resten af programmet
            handled |= SYS_STATUS_RXFCG | SYS_STATUS_RXDFR;
        }

        // 4. Modtagefejl eller timeout
        if (status & (SYS_STATUS_RXFCE | SYS_STATUS_RXRFTO))
        {
            // målingen er mislykket: kassér den, og tænd modtageren igen
            handled |= status & (SYS_STATUS_RXFCE | SYS_STATUS_RXRFTO);
        }

        // 5. Ryd de behandlede flag (skriv 1 til dem), så IRQ går lav
        const std::array<uint8_t, 4> clear = {
            static_cast<uint8_t>(handled & 0xFF),
            static_cast<uint8_t>((handled >> 8) & 0xFF),
            static_cast<uint8_t>((handled >> 16) & 0xFF),
            static_cast<uint8_t>((handled >> 24) & 0xFF)};
        writeRegister(REG_SYS_STATUS, 0, clear.data(), clear.size());
    }
}

bool UWBDriver::readRegister(uint8_t reg, uint16_t subAddr, uint8_t *dataOut, size_t len)
{
    if(dataOut == nullptr || len == 0 || len > MAX_DATA_LEN)
        return false;

    std::lock_guard<std::mutex> lock(SPImtx); // lås mutexen for at undgå race conditions

    std::array<uint8_t, 1 + MAX_DATA_LEN> tx{};
    std::array<uint8_t, 1 + MAX_DATA_LEN> rx{};

    tx[0] = reg & 0x3F; // bit 7 = 0 (læs), bit 6 = 0 (ingen sub-adresse)

    if (!spi.transfer(tx.data(), rx.data(), 1 + len))
        return false;

    std::memcpy(dataOut, rx.data() + 1, len);
    return true;
}

// Skriv et register (uden sub-adresse)
bool UWBDriver::writeRegister(uint8_t reg, uint16_t subAddr, const uint8_t *dataIn, size_t len)
{
    if(dataIn == nullptr || len == 0 || len > MAX_DATA_LEN)
        return false;

    std::lock_guard<std::mutex> lock(SPImtx); // lås mutexen for at undgå race conditions

    std::array<uint8_t, 1 + MAX_DATA_LEN> tx{};
    std::array<uint8_t, 1 + MAX_DATA_LEN> rx{};

    tx[0] = 0x80 | (reg & 0x3F); // bit 7 = 1 (skriv)
    std::memcpy(tx.data() + 1, dataIn, len);

    return spi.transfer(tx.data(), rx.data(), 1 + len);
}

// init:
// 1. læs UWB's device ID (register 0x00)
// 2. konfigurér systemet (SYS_CFG) (register 0x04)
// 3. opsæt antenne-delay (sensor kalibrering)
// 4. begge moduler opsættes ens:
// 4.1. PAN ID og short adress (register 0x03)
// 4.2. kanalindstillinger (CHAN_CTRL/TXFCTRL)
// 5. konfigurér SYS_MASK register
bool UWBDriver::init() {
    
}

void UWBDriver::beginListen()
{
    
}

bool UWBDriver::getDistance(uint8_t reg, uint8_t *dataOut, size_t len)
{
    return readRegister(reg, dataOut, len, 0); // subAddr = 0 for afstandsdata
}
