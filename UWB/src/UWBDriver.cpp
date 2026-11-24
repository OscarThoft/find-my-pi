#include "UWBDriver.h"
#include "DW1000registre.h"
#include <array>
#include <cstring>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace std::chrono_literals;

constexpr uint32_t EXPECTED_DEV_ID = 0xDECA0130;
constexpr uint16_t PAN_ID = 0xDECA;       // fælles netværks-ID, ens på begge moduler
constexpr uint16_t ANTENNA_DELAY = 16459; // startværdi; kalibreres mod en kendt afstand
constexpr size_t MAX_DATA_LEN = 128;  // største antal data-bytes i en SPI-overførsel
constexpr size_t MAX_HEADER_LEN = 3;      // SPI-headeren er 1-3 bytes
constexpr auto POLL_INTERVAL = 100us;     // pause mellem to læsninger af SYS_STATUS
constexpr auto TX_TIMEOUT = 10ms;         // en frame er sendt på under 1 ms

// SPI: header på 1-3 bytes efterfulgt af data
//   byte 0: bit 7 = skriv, bit 6 = sub-adresse følger, bit 5-0 = register
//   byte 1: bit 7 = en byte mere følger, bit 6-0 = sub-adressens 7 laveste bit
//   byte 2: sub-adressens 8 højeste bit
static size_t buildHeader(uint8_t *header, uint8_t reg, uint16_t subAddr, bool write)
{
    header[0] = (write ? 0x80 : 0x00) | (reg & 0x3F);
    if (subAddr == 0)
        return 1;

    header[0] |= 0x40;
    header[1] = subAddr & 0x7F;
    if (subAddr < 128)
        return 2;

    header[1] |= 0x80;
    header[2] = (subAddr >> 7) & 0xFF;
    return 3;
}

bool UWBDriver::readRegister(uint8_t reg, uint8_t *dataOut, size_t len, uint16_t subAddr)
{
    if (dataOut == nullptr || len == 0 || len > MAX_DATA_LEN)
        return false;

    std::array<uint8_t, MAX_HEADER_LEN + MAX_DATA_LEN> tx{}, rx{};
    const size_t headerLen = buildHeader(tx.data(), reg, subAddr, false);

    if (!spi.transfer(tx.data(), rx.data(), headerLen + len))
        return false;

    std::memcpy(dataOut, rx.data() + headerLen, len);
    return true;
}

bool UWBDriver::writeRegister(uint8_t reg, const uint8_t *dataIn, size_t len, uint16_t subAddr)
{
    if (dataIn == nullptr || len == 0 || len > MAX_DATA_LEN)
        return false;

    std::array<uint8_t, MAX_HEADER_LEN + MAX_DATA_LEN> tx{}, rx{};
    const size_t headerLen = buildHeader(tx.data(), reg, subAddr, true);
    std::memcpy(tx.data() + headerLen, dataIn, len);

    return spi.transfer(tx.data(), rx.data(), headerLen + len);
}

// Deler et tal op i bytes, mindste byte først
template <typename T>
bool UWBDriver::writeValue(uint8_t reg, uint16_t subAddr, T value)
{
    static_assert(std::is_unsigned<T>::value, "Angiv typen: writeValue<uint16_t>(...)");

    std::array<uint8_t, sizeof(T)> raw{};
    for (size_t i = 0; i < raw.size(); ++i)
        raw[i] = static_cast<uint8_t>(value >> (8 * i));
    return writeRegister(reg, raw.data(), raw.size(), subAddr);
}

// Samler bytes til et tal. bytes kan være mindre end sizeof(T), fx 5 til et tidsstempel
template <typename T>
bool UWBDriver::readValue(uint8_t reg, uint16_t subAddr, T &value, size_t bytes)
{
    static_assert(std::is_unsigned<T>::value, "readValue kræver en unsigned type");

    std::array<uint8_t, sizeof(T)> raw{};
    if (bytes > raw.size() || !readRegister(reg, raw.data(), bytes, subAddr))
        return false;

    value = 0;
    for (size_t i = 0; i < bytes; ++i)
        value |= static_cast<T>(raw[i]) << (8 * i);
    return true;
}

// Polling: spørg chippen, til der er sket noget

bool UWBDriver::waitForStatus(uint32_t mask, std::chrono::microseconds timeout, uint32_t &status)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true)
    {
        if (!readValue<uint32_t>(REG_SYS_STATUS, 0, status))
            return false; // SPI-fejl
        if (status & mask)
            return true; // en af de ønskede hændelser er sket
        if (std::chrono::steady_clock::now() >= deadline)
            return false; // timeout

        std::this_thread::sleep_for(POLL_INTERVAL);
    }
}

// init: køres hver gang programmet starter

bool UWBDriver::init(uint16_t shortAddr)
{
    // 1. Tjek at chippen svarer
    uint32_t devId = 0;
    if (!readValue<uint32_t>(REG_DEV_ID, 0, devId) || devId != EXPECTED_DEV_ID)
    {
        std::cerr << "UWB init: forkert eller intet Device ID (laeste 0x" << std::hex
                  << devId << std::dec << ", forventede 0xdeca0130)." << std::endl;
        return false;
    }

    // 2. Software-reset (erstatter RST-ledningen)
    bool ok = true;
    ok &= writeValue<uint8_t>(REG_PMSC, SUB_PMSC_CTRL0, 0x01);     // systemur på krystallen
    ok &= writeValue<uint8_t>(REG_PMSC, SUB_PMSC_SOFTRESET, 0x00); // nulstil alt
    std::this_thread::sleep_for(1ms);
    ok &= writeValue<uint8_t>(REG_PMSC, SUB_PMSC_SOFTRESET, 0xF0); // slip reset
    std::this_thread::sleep_for(5ms);

    // 3. Indlæs LDE-mikrokoden (kræves for gyldige modtage-tidsstempler)
    ok &= writeValue<uint16_t>(REG_PMSC, SUB_PMSC_CTRL0, 0x0301);
    ok &= writeValue<uint16_t>(REG_OTP_IF, SUB_OTP_CTRL, 0x8000);
    std::this_thread::sleep_for(1ms);
    ok &= writeValue<uint16_t>(REG_PMSC, SUB_PMSC_CTRL0, 0x0200);

    // 4. Ret standardværdierne (User Manual afsnit 2.5.5): kanal 5, 16 MHz PRF, 6,8 Mbit/s
    ok &= writeValue<uint32_t>(REG_CHAN_CTRL, 0, 0x21040055);
    ok &= writeValue<uint16_t>(REG_AGC_CTRL, SUB_AGC_TUNE1, 0x8870);
    ok &= writeValue<uint32_t>(REG_AGC_CTRL, SUB_AGC_TUNE2, 0x2502A907);
    ok &= writeValue<uint32_t>(REG_DRX_CONF, SUB_DRX_TUNE2, 0x311A002D);
    ok &= writeValue<uint8_t>(REG_LDE_CTRL, SUB_LDE_CFG1, 0x6D);
    ok &= writeValue<uint16_t>(REG_LDE_CTRL, SUB_LDE_REPC, 0x428E); // præambelkode 4, 6,8 Mbit/s
    ok &= writeValue<uint16_t>(REG_LDE_CTRL, SUB_LDE_CFG2, 0x1607);
    ok &= writeValue<uint32_t>(REG_TX_POWER, 0, 0x0E082848);
    ok &= writeValue<uint32_t>(REG_RF_CONF, SUB_RF_TXCTRL, 0x001E3FE0);
    ok &= writeValue<uint8_t>(REG_TX_CAL, SUB_TC_PGDELAY, 0xC0);
    ok &= writeValue<uint8_t>(REG_FS_CTRL, SUB_FS_PLLTUNE, 0xBE);

    // 5. Egen adresse: bit 0-15 = short address, bit 16-31 = PAN ID
    const uint32_t panAdr = (static_cast<uint32_t>(PAN_ID) << 16) | shortAddr;
    ok &= writeValue<uint32_t>(REG_PANADR, 0, panAdr);

    // 6. Antenne-delay på sende- og modtagesiden
    ok &= writeValue<uint16_t>(REG_TX_ANTD, 0, ANTENNA_DELAY);
    ok &= writeValue<uint16_t>(REG_LDE_CTRL, SUB_LDE_RXANTD, ANTENNA_DELAY);

    // 7. Ryd gamle flag (skriv 1 for at rydde)
    ok &= writeValue<uint32_t>(REG_SYS_STATUS, 0, 0xFFFFFFFF);

    // 8. Bekræft at adressen sidder i chippen
    uint32_t readBack = 0;
    if (!ok || !readValue<uint32_t>(REG_PANADR, 0, readBack) || readBack != panAdr)
    {
        std::cerr << "UWB init: opsaetningen blev ikke gemt i chippen." << std::endl;
        return false;
    }
    return true;
}

// Sending
bool UWBDriver::sendFrame(const uint8_t *data, size_t len, bool expectResponse, uint64_t &txTime)
{
    if (data == nullptr || len == 0 || len > MAX_FRAME_LEN)
        return false;

    // 1. Læg framen i sendebufferen, og skriv længden (inkl. 2 CRC-bytes) i de
    //    10 nederste bit af TX_FCTRL. Resten af registret bevares.
    uint32_t fctrl = 0;
    if (!writeRegister(REG_TX_BUFFER, data, len, 0) ||
        !readValue<uint32_t>(REG_TX_FCTRL, 0, fctrl) ||
        !writeValue<uint32_t>(REG_TX_FCTRL, 0, (fctrl & ~0x3FFu) | static_cast<uint32_t>(len + 2)))
        return false;

    // 2. Ryd gamle flag, og start sendingen
    const uint8_t command = expectResponse ? (SYS_CTRL_TXSTRT | SYS_CTRL_WAIT4RESP)
                                           : SYS_CTRL_TXSTRT;
    if (!writeValue<uint32_t>(REG_SYS_STATUS, 0, SYS_STATUS_ALL_TX | SYS_STATUS_ALL_RX) ||
        !writeValue<uint8_t>(REG_SYS_CTRL, 0, command))
        return false;

    // 3. Poll, til framen er sendt
    uint32_t status = 0;
    if (!waitForStatus(SYS_STATUS_TXFRS, TX_TIMEOUT, status))
    {
        stopReceive();
        return false;
    }

    // 4. Ryd sende-flagene, og hent tidsstemplet (5 bytes)
    return writeValue<uint32_t>(REG_SYS_STATUS, 0, SYS_STATUS_ALL_TX) &&
           readValue<uint64_t>(REG_TX_TIME, 0, txTime, 5);
}

// Modtagelse
bool UWBDriver::startReceive()
{
    return writeValue<uint32_t>(REG_SYS_STATUS, 0, SYS_STATUS_ALL_RX) && // ryd gamle flag
           writeValue<uint16_t>(REG_SYS_CTRL, 0, SYS_CTRL_RXENAB);
}

// Efter en modtagefejl skal modtageren nulstilles, ellers kan næste tidsstempel blive forkert
void UWBDriver::resetReceiver()
{
    writeValue<uint8_t>(REG_PMSC, SUB_PMSC_SOFTRESET, 0xE0); // nulstil kun modtageren
    writeValue<uint8_t>(REG_PMSC, SUB_PMSC_SOFTRESET, 0xF0); // slip reset
    writeValue<uint32_t>(REG_SYS_STATUS, 0, SYS_STATUS_ALL_RX);
}

void UWBDriver::stopReceive()
{
    writeValue<uint8_t>(REG_SYS_CTRL, 0, SYS_CTRL_TRXOFF);
    resetReceiver();
}

bool UWBDriver::receiveFrame(uint8_t *data, size_t maxLen, size_t &len, uint64_t &rxTime,
                             std::chrono::milliseconds timeout)
{
    if (data == nullptr || maxLen == 0)
        return false;

    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true)
    {
        // 1. Poll, til der er modtaget en frame, sket en fejl, eller tiden er gået
        const auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
            deadline - std::chrono::steady_clock::now());

        uint32_t status = 0;
        if (remaining.count() <= 0 ||
            !waitForStatus(SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO,
                           remaining, status))
        {
            stopReceive();
            return false; // timeout
        }

        // 2. God frame: hent længde, indhold og tidsstempel
        if (status & SYS_STATUS_RXFCG)
        {
            // Tidsstemplet er først gyldigt, når LDEDONE er sat
            if (!(status & SYS_STATUS_LDEDONE))
                waitForStatus(SYS_STATUS_LDEDONE, 2ms, status);

            uint32_t finfo = 0;
            bool ok = (status & SYS_STATUS_LDEDONE) && readValue<uint32_t>(REG_RX_FINFO, 0, finfo);

            const size_t frameLen = finfo & 0x7F; // inkl. 2 CRC-bytes
            ok = ok && frameLen > 2 && frameLen - 2 <= maxLen;

            if (ok)
            {
                len = frameLen - 2;
                ok = readRegister(REG_RX_BUFFER, data, len, 0) &&
                     readValue<uint64_t>(REG_RX_TIME, 0, rxTime, 5);
            }

            writeValue<uint32_t>(REG_SYS_STATUS, 0, SYS_STATUS_ALL_RX_GOOD);
            if (ok)
                return true;
        }

        // 3. Fejl eller ubrugelig frame: nulstil modtageren, og lyt videre
        resetReceiver();
        if (!startReceive())
            return false;
    }
}