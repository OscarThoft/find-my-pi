// Afstandsmåling mellem to UWB Click-moduler med polling
//
// Brug:  ./uwb_ranging_test gemmer      (initiator: starter målingen og regner afstanden)
//        ./uwb_ranging_test soeger      (responder: svarer)
// Kompilér:
//   g++ -std=c++17 uwb_ranging_test.cpp UWBDriver2.cpp SPIDriver.cpp -o uwb_ranging_test
//
// Én måling er fire frames:
//   Gemmer -> Søger : POLL                 t1 sendt,    t2 modtaget
//   Søger -> Gemmer : RESPONSE             t3 sendt,    t4 modtaget
//   Gemmer -> Søger : FINAL                t5 sendt,    t6 modtaget
//   Søger -> Gemmer : REPORT (t2, t3, t6)  så Gemmeren har alle seks tidsstempler

#include "SPIDriver.h"
#include "UWBDriver2.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// ---------- Adresser ----------
constexpr uint16_t PAN_ID = 0xDECA;
constexpr uint16_t ADDR_GEMMER = 0x0001;
constexpr uint16_t ADDR_SOEGER = 0x0002;

// ---------- Beskedtyper (første byte efter headeren) ----------
constexpr uint8_t MSG_POLL = 0x61;
constexpr uint8_t MSG_RESPONSE = 0x50;
constexpr uint8_t MSG_FINAL = 0x69;
constexpr uint8_t MSG_REPORT = 0x2A;

// ---------- Frame-layout ----------
// byte 0-1: frame control (0x41 0x88 = dataframe med korte adresser)
// byte 2:   sekvensnummer
// byte 3-4: PAN ID          byte 5-6: modtager          byte 7-8: afsender
// byte 9:   beskedtype      byte 10..: payload
constexpr size_t HEADER_LEN = 10;
constexpr size_t TIMESTAMP_LEN = 5; // 40 bit

constexpr auto REPLY_TIMEOUT = 50ms;     // hvor længe vi venter på et svar
constexpr auto MEASURE_INTERVAL = 200ms; // pause mellem to målinger

using Frame = std::array<uint8_t, UWBDriver::MAX_FRAME_LEN>;

// Bygger headeren og returnerer framens længde (uden payload)
size_t buildHeader(Frame &frame, uint8_t seq, uint16_t dest, uint16_t src, uint8_t type)
{
    frame[0] = 0x41;
    frame[1] = 0x88;
    frame[2] = seq;
    frame[3] = PAN_ID & 0xFF;
    frame[4] = PAN_ID >> 8;
    frame[5] = dest & 0xFF;
    frame[6] = dest >> 8;
    frame[7] = src & 0xFF;
    frame[8] = src >> 8;
    frame[9] = type;
    return HEADER_LEN;
}

// Tjekker, at en modtaget frame er til os, fra den rigtige afsender og af den rigtige type
bool isExpected(const Frame &frame, size_t len, uint16_t me, uint16_t from, uint8_t type)
{
    return len >= HEADER_LEN &&
           frame[0] == 0x41 && frame[1] == 0x88 &&
           frame[3] == (PAN_ID & 0xFF) && frame[4] == (PAN_ID >> 8) &&
           frame[5] == (me & 0xFF) && frame[6] == (me >> 8) &&
           frame[7] == (from & 0xFF) && frame[8] == (from >> 8) &&
           frame[9] == type;
}

void putTimestamp(Frame &frame, size_t pos, uint64_t t)
{
    for (size_t i = 0; i < TIMESTAMP_LEN; ++i)
        frame[pos + i] = static_cast<uint8_t>(t >> (8 * i));
}

uint64_t getTimestamp(const Frame &frame, size_t pos)
{
    uint64_t t = 0;
    for (size_t i = 0; i < TIMESTAMP_LEN; ++i)
        t |= static_cast<uint64_t>(frame[pos + i]) << (8 * i);
    return t;
}

// Afstand ud fra seks tidsstempler (double-sided two-way ranging).
// Formlen tåler, at de to chips' ure går lidt forskelligt, og at svartiderne er lange.
double computeDistance(uint64_t t1, uint64_t t2, uint64_t t3,
                       uint64_t t4, uint64_t t5, uint64_t t6)
{
    constexpr uint64_t MASK_40BIT = 0xFFFFFFFFFFULL;
    constexpr double TICK_SECONDS = 1.0 / (128.0 * 499.2e6); // ca. 15,65 ps
    constexpr double SPEED_OF_LIGHT = 299'792'458.0;         // m/s

    // Differencer i chippens tidsenheder; masken håndterer, at tælleren løber rundt
    const double roundA = static_cast<double>((t4 - t1) & MASK_40BIT); // Gemmer: poll -> response
    const double replyA = static_cast<double>((t5 - t4) & MASK_40BIT); // Gemmer: response -> final
    const double roundB = static_cast<double>((t6 - t3) & MASK_40BIT); // Søger: response -> final
    const double replyB = static_cast<double>((t3 - t2) & MASK_40BIT); // Søger: poll -> response

    const double tof = (roundA * roundB - replyA * replyB) /
                       (roundA + roundB + replyA + replyB);

    return tof * TICK_SECONDS * SPEED_OF_LIGHT;
}

// =====================================================================
// Gemmer (initiator)
// =====================================================================
bool measureOnce(UWBDriver &uwb, uint8_t seq, double &distance)
{
    Frame tx{}, rx{};
    size_t rxLen = 0;
    uint64_t t1 = 0, t4 = 0, t5 = 0, ignored = 0;

    // 1. Send POLL, og vent på RESPONSE
    size_t txLen = buildHeader(tx, seq, ADDR_SOEGER, ADDR_GEMMER, MSG_POLL);
    if (!uwb.sendFrame(tx.data(), txLen, true, t1))
        return false;
    if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, t4, REPLY_TIMEOUT) ||
        !isExpected(rx, rxLen, ADDR_GEMMER, ADDR_SOEGER, MSG_RESPONSE) || rx[2] != seq)
        return false;

    // 2. Send FINAL, og vent på REPORT med Søgerens tre tidsstempler
    txLen = buildHeader(tx, seq, ADDR_SOEGER, ADDR_GEMMER, MSG_FINAL);
    if (!uwb.sendFrame(tx.data(), txLen, true, t5))
        return false;
    if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, ignored, REPLY_TIMEOUT) ||
        !isExpected(rx, rxLen, ADDR_GEMMER, ADDR_SOEGER, MSG_REPORT) || rx[2] != seq ||
        rxLen < HEADER_LEN + 3 * TIMESTAMP_LEN)
        return false;

    const uint64_t t2 = getTimestamp(rx, HEADER_LEN);
    const uint64_t t3 = getTimestamp(rx, HEADER_LEN + TIMESTAMP_LEN);
    const uint64_t t6 = getTimestamp(rx, HEADER_LEN + 2 * TIMESTAMP_LEN);

    distance = computeDistance(t1, t2, t3, t4, t5, t6);
    return true;
}

void runGemmer(UWBDriver &uwb, int count)
{
    uint8_t seq = 0;
    for (int i = 0; count < 0 || i < count; ++i)
    {
        double distance = 0.0;
        if (measureOnce(uwb, seq, distance))
            std::cout << "Afstand: " << std::fixed << std::setprecision(2) << distance << " m" << std::endl;
        else
            std::cout << "Ingen maaling (intet svar fra Soeger)" << std::endl;

        ++seq;
        std::this_thread::sleep_for(MEASURE_INTERVAL);
    }
}

// =====================================================================
// Søger (responder)
// =====================================================================
void runSoeger(UWBDriver &uwb)
{
    Frame tx{}, rx{};
    size_t rxLen = 0;
    uint64_t t2 = 0, t3 = 0, t6 = 0, ignored = 0;

    while (true)
    {
        // 1. Vent på POLL
        if (!uwb.startReceive() ||
            !uwb.receiveFrame(rx.data(), rx.size(), rxLen, t2, 1000ms) ||
            !isExpected(rx, rxLen, ADDR_SOEGER, ADDR_GEMMER, MSG_POLL))
            continue;

        const uint8_t seq = rx[2];

        // 2. Send RESPONSE, og vent på FINAL
        size_t txLen = buildHeader(tx, seq, ADDR_GEMMER, ADDR_SOEGER, MSG_RESPONSE);
        if (!uwb.sendFrame(tx.data(), txLen, true, t3))
            continue;
        if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, t6, REPLY_TIMEOUT) ||
            !isExpected(rx, rxLen, ADDR_SOEGER, ADDR_GEMMER, MSG_FINAL) || rx[2] != seq)
            continue;

        // 3. Send REPORT med de tre tidsstempler, Gemmeren mangler
        txLen = buildHeader(tx, seq, ADDR_GEMMER, ADDR_SOEGER, MSG_REPORT);
        putTimestamp(tx, txLen, t2);
        putTimestamp(tx, txLen + TIMESTAMP_LEN, t3);
        putTimestamp(tx, txLen + 2 * TIMESTAMP_LEN, t6);
        uwb.sendFrame(tx.data(), txLen + 3 * TIMESTAMP_LEN, false, ignored);
    }
}

int main(int argc, char *argv[])
{
    const std::string role = argc >= 2 ? argv[1] : "";
    if (role != "gemmer" && role != "soeger")
    {
        std::cerr << "Brug: " << argv[0] << " gemmer|soeger [antal maalinger]" << std::endl;
        return 1;
    }
    const int count = argc >= 3 ? std::atoi(argv[2]) : -1; // -1 = kør for evigt

    SPIDriver spi(1000000, 0, 8); // 1 MHz, mode 0, 8 bit
    UWBDriver uwb(spi);

    if (!uwb.init(role == "gemmer" ? ADDR_GEMMER : ADDR_SOEGER))
    {
        std::cerr << "init() fejlede." << std::endl;
        return 1;
    }
    std::cout << "UWB klar som " << role << std::endl;

    if (role == "gemmer")
        runGemmer(uwb, count);
    else
        runSoeger(uwb);

    return 0;
}