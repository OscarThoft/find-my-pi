#include "UWBRanging.h"

using namespace std::chrono_literals;

// Skal være den samme som PAN_ID i UWBDriver.cpp
constexpr uint16_t PAN_ID = 0xDECA;

// ---------- Beskedtyper (byte 9 i hver besked) ----------
constexpr uint8_t MSG_POLL = 0x61;
constexpr uint8_t MSG_RESPONSE = 0x50;
constexpr uint8_t MSG_FINAL = 0x69;
constexpr uint8_t MSG_REPORT = 0x2A;

// ---------- Beskedens opbygning ----------
// byte 0-1: frame control (0x41 0x88 = dataframe med korte adresser)
// byte 2:   sekvensnummer
// byte 3-4: PAN ID     byte 5-6: modtager     byte 7-8: afsender
// byte 9:   beskedtype byte 10..: payload
constexpr size_t HEADER_LEN = 10;
constexpr size_t TIMESTAMP_LEN = 5; // 40 bit

constexpr auto REPLY_TIMEOUT = 50ms; // hvor længe der ventes på et svar midt i en måling

UWBRanging::UWBRanging(UWBDriver &driver, uint16_t ownAddress, uint16_t peerAddress)
    : uwb(driver), myAddr(ownAddress), peerAddr(peerAddress)
{
}

// =====================================================================
// Hjælpefunktioner
// =====================================================================
size_t UWBRanging::buildFrameHeader(Frame &frame, uint8_t type, uint8_t frameSeq) const
{
    frame[0] = 0x41;
    frame[1] = 0x88;
    frame[2] = frameSeq;
    frame[3] = PAN_ID & 0xFF;
    frame[4] = PAN_ID >> 8;
    frame[5] = peerAddr & 0xFF; // modtager
    frame[6] = peerAddr >> 8;
    frame[7] = myAddr & 0xFF; // afsender
    frame[8] = myAddr >> 8;
    frame[9] = type;
    return HEADER_LEN;
}

bool UWBRanging::isExpected(const Frame &frame, size_t len, uint8_t type) const
{
    return len >= HEADER_LEN &&
           frame[0] == 0x41 && frame[1] == 0x88 &&
           frame[3] == (PAN_ID & 0xFF) && frame[4] == (PAN_ID >> 8) &&
           frame[5] == (myAddr & 0xFF) && frame[6] == (myAddr >> 8) &&     // til mig
           frame[7] == (peerAddr & 0xFF) && frame[8] == (peerAddr >> 8) && // fra modparten
           frame[9] == type;
}

void UWBRanging::putTimestamp(Frame &frame, size_t pos, uint64_t t)
{
    for (size_t i = 0; i < TIMESTAMP_LEN; ++i)
        frame[pos + i] = static_cast<uint8_t>(t >> (8 * i));
}

uint64_t UWBRanging::getTimestamp(const Frame &frame, size_t pos)
{
    uint64_t t = 0;
    for (size_t i = 0; i < TIMESTAMP_LEN; ++i)
        t |= static_cast<uint64_t>(frame[pos + i]) << (8 * i);
    return t;
}

// =====================================================================
// Gemmer (initiator): én måling
// =====================================================================
bool UWBRanging::measure(Timestamps &ts)
{
    const uint8_t s = seq++; // nyt sekvensnummer til denne måling
    Frame tx{}, rx{};
    size_t rxLen = 0;
    uint64_t ignored = 0;

    // 1. Send POLL, og vent på RESPONSE
    size_t txLen = buildFrameHeader(tx, MSG_POLL, s);
    if (!uwb.sendFrame(tx.data(), txLen, true, ts.t1))
        return false;
    if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, ts.t4, REPLY_TIMEOUT) ||
        !isExpected(rx, rxLen, MSG_RESPONSE) || rx[2] != s)
        return false;

    // 2. Send FINAL, og vent på REPORT med Søgerens tre tidsstempler
    txLen = buildFrameHeader(tx, MSG_FINAL, s);
    if (!uwb.sendFrame(tx.data(), txLen, true, ts.t5))
        return false;
    if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, ignored, REPLY_TIMEOUT) ||
        !isExpected(rx, rxLen, MSG_REPORT) || rx[2] != s ||
        rxLen < HEADER_LEN + 3 * TIMESTAMP_LEN)
        return false;

    // 3. Pak Søgerens tidsstempler ud
    ts.t2 = getTimestamp(rx, HEADER_LEN);
    ts.t3 = getTimestamp(rx, HEADER_LEN + TIMESTAMP_LEN);
    ts.t6 = getTimestamp(rx, HEADER_LEN + 2 * TIMESTAMP_LEN);
    return true;
}

// =====================================================================
// Søger (responder): én omgang
// =====================================================================
bool UWBRanging::respond(std::chrono::milliseconds timeout)
{
    Frame tx{}, rx{};
    size_t rxLen = 0;
    uint64_t t2 = 0, t3 = 0, t6 = 0, ignored = 0;

    // 1. Vent på POLL
    if (!uwb.startReceive() ||
        !uwb.receiveFrame(rx.data(), rx.size(), rxLen, t2, timeout) ||
        !isExpected(rx, rxLen, MSG_POLL))
        return false;

    const uint8_t s = rx[2]; // svar med samme sekvensnummer som Gemmeren

    // 2. Send RESPONSE, og vent på FINAL
    size_t txLen = buildFrameHeader(tx, MSG_RESPONSE, s);
    if (!uwb.sendFrame(tx.data(), txLen, true, t3))
        return false;
    if (!uwb.receiveFrame(rx.data(), rx.size(), rxLen, t6, REPLY_TIMEOUT) ||
        !isExpected(rx, rxLen, MSG_FINAL) || rx[2] != s)
        return false;

    // 3. Send REPORT med de tre tidsstempler, Gemmeren mangler
    txLen = buildFrameHeader(tx, MSG_REPORT, s);
    putTimestamp(tx, txLen, t2);
    putTimestamp(tx, txLen + TIMESTAMP_LEN, t3);
    putTimestamp(tx, txLen + 2 * TIMESTAMP_LEN, t6);
    return uwb.sendFrame(tx.data(), txLen + 3 * TIMESTAMP_LEN, false, ignored);
}