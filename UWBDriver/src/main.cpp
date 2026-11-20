#include "SPIDriver.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

// ---------- Indstillinger ----------
constexpr uint32_t SPI_SPEED_HZ = 1000000; // sæt ned til 100000 ved ustabil forbindelse
constexpr uint8_t SPI_MODE = 0;
constexpr uint16_t PAN_ID = 0xDECA; // fælles netværks-ID

// ---------- DW1000 registre ----------
constexpr uint8_t REG_DEV_ID = 0x00; // 4 bytes
constexpr uint8_t REG_EUI = 0x01;    // 8 bytes
constexpr uint8_t REG_PANADR = 0x03; // 4 bytes: [0-1] short address, [2-3] PAN ID
constexpr uint32_t EXPECTED_DEV_ID = 0xDECA0130;

constexpr size_t MAX_DATA_LEN = 16;

// Læs et register (uden sub-adresse)
bool readRegister(SPIDriver &spi, uint8_t reg, uint8_t *dataOut, size_t len)
{
    if (len > MAX_DATA_LEN)
        return false;

    std::array<uint8_t, 1 + MAX_DATA_LEN> tx{};
    std::array<uint8_t, 1 + MAX_DATA_LEN> rx{};

    tx[0] = reg & 0x3F; // bit 7 = 0 (læs), bit 6 = 0 (ingen sub-adresse)

    if (!spi.transfer(tx.data(), rx.data(), 1 + len))
        return false;

    std::memcpy(dataOut, rx.data() + 1, len);
    return true;
}

// Skriv et register (uden sub-adresse)
bool writeRegister(SPIDriver &spi, uint8_t reg, const uint8_t *dataIn, size_t len)
{
    if (len > MAX_DATA_LEN)
        return false;

    std::array<uint8_t, 1 + MAX_DATA_LEN> tx{};
    std::array<uint8_t, 1 + MAX_DATA_LEN> rx{};

    tx[0] = 0x80 | (reg & 0x3F); // bit 7 = 1 (skriv)
    std::memcpy(tx.data() + 1, dataIn, len);

    return spi.transfer(tx.data(), rx.data(), 1 + len);
}

void printHex(uint32_t value, int width)
{
    std::cout << "0x" << std::hex << std::uppercase << std::setw(width)
              << std::setfill('0') << value << std::dec;
}

int main(int argc, char *argv[])
{
    // ---------- Modulnummer fra kommandolinjen ----------
    if (argc != 2)
    {
        std::cerr << "Brug: " << argv[0] << " <modulnummer 1-254>" << std::endl;
        return 1;
    }

    const int moduleNumber = std::atoi(argv[1]);
    if (moduleNumber < 1 || moduleNumber > 254)
    {
        std::cerr << "Fejl: modulnummer skal vaere mellem 1 og 254." << std::endl;
        return 1;
    }

    const uint16_t shortAddr = static_cast<uint16_t>(moduleNumber);

    std::cout << "Opsaetter modul " << moduleNumber << std::endl;

    SPIDriver spi(SPI_SPEED_HZ, SPI_MODE, 8);

    // ---------- 1. Tjek Device ID ----------
    std::cout << "\n--- 1. Tjekker Device ID ---" << std::endl;

    std::array<uint8_t, 4> devId{};
    if (!readRegister(spi, REG_DEV_ID, devId.data(), devId.size()))
    {
        std::cerr << "Fejl: SPI-laesning af Device ID fejlede." << std::endl;
        return 1;
    }

    const uint32_t id = (static_cast<uint32_t>(devId[3]) << 24) |
                        (static_cast<uint32_t>(devId[2]) << 16) |
                        (static_cast<uint32_t>(devId[1]) << 8) |
                        static_cast<uint32_t>(devId[0]);

    std::cout << "Device ID: ";
    printHex(id, 8);
    std::cout << std::endl;

    if (id != EXPECTED_DEV_ID)
    {
        std::cerr << "Fejl: forventede 0xDECA0130." << std::endl;
        if (id == 0x00000000 || id == 0xFFFFFFFF)
        {
            std::cerr << " -> Chippen svarer ikke. Tjek stroem, GND, MISO, CS, "
                         "og at RST ikke holdes lav."
                      << std::endl;
        }
        else if (id == 0xBC950360)
        {
            std::cerr << " -> Svaret er forskudt en bit. Tag stroemmen fra modulet "
                         "og saet den til igen, saenk SPI_SPEED_HZ, tjek SCK og GND."
                      << std::endl;
        }
        return 1;
    }
    std::cout << " -> OK" << std::endl;

    // ---------- 2. Skriv og bekraeft EUI ----------
    std::cout << "\n--- 2. Skriver EUI (register 0x01) ---" << std::endl;

    // Little-endian: mindst betydende byte foerst
    const std::array<uint8_t, 8> eui = {
        static_cast<uint8_t>(moduleNumber), 0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0xDE};

    if (!writeRegister(spi, REG_EUI, eui.data(), eui.size()))
    {
        std::cerr << "Fejl: skrivning af EUI fejlede." << std::endl;
        return 1;
    }

    std::array<uint8_t, 8> euiRead{};
    if (!readRegister(spi, REG_EUI, euiRead.data(), euiRead.size()))
    {
        std::cerr << "Fejl: tilbagelaesning af EUI fejlede." << std::endl;
        return 1;
    }

    std::cout << "EUI: ";
    for (int i = 7; i >= 0; --i)
    {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<int>(euiRead[i]) << (i > 0 ? ":" : "");
    }
    std::cout << std::dec << std::endl;

    if (euiRead != eui)
    {
        std::cerr << "Fejl: EUI blev ikke gemt korrekt." << std::endl;
        return 1;
    }
    std::cout << " -> OK" << std::endl;

    // ---------- 3. Skriv og bekraeft PAN ID + Short Address ----------
    std::cout << "\n--- 3. Skriver PAN ID og Short Address (register 0x03) ---" << std::endl;

    const std::array<uint8_t, 4> panAdr = {
        static_cast<uint8_t>(shortAddr & 0xFF), static_cast<uint8_t>(shortAddr >> 8),
        static_cast<uint8_t>(PAN_ID & 0xFF), static_cast<uint8_t>(PAN_ID >> 8)};

    if (!writeRegister(spi, REG_PANADR, panAdr.data(), panAdr.size()))
    {
        std::cerr << "Fejl: skrivning af PAN ID / Short Address fejlede." << std::endl;
        return 1;
    }

    std::array<uint8_t, 4> panAdrRead{};
    if (!readRegister(spi, REG_PANADR, panAdrRead.data(), panAdrRead.size()))
    {
        std::cerr << "Fejl: tilbagelaesning af PAN ID / Short Address fejlede." << std::endl;
        return 1;
    }

    const uint16_t shortRead = static_cast<uint16_t>((panAdrRead[1] << 8) | panAdrRead[0]);
    const uint16_t panRead = static_cast<uint16_t>((panAdrRead[3] << 8) | panAdrRead[2]);

    std::cout << "PAN ID: ";
    printHex(panRead, 4);
    std::cout << " | Short Address: ";
    printHex(shortRead, 4);
    std::cout << std::endl;

    if (panAdrRead != panAdr)
    {
        std::cerr << "Fejl: PAN ID / Short Address blev ikke gemt korrekt." << std::endl;
        return 1;
    }
    std::cout << " -> OK" << std::endl;

    std::cout << "\nModul " << moduleNumber << " er sat op." << std::endl;
    return 0;
}