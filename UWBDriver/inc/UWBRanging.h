#pragma once
#include "UWBDriver.h"
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

// Afstandsmåling mellem to UWB-moduler med double-sided two-way ranging (DS-TWR).
// Én måling er fire beskeder:
//   Gemmer -> Søger : POLL                  t1 sendt,    t2 modtaget
//   Søger  -> Gemmer: RESPONSE              t3 sendt,    t4 modtaget
//   Gemmer -> Søger : FINAL                 t5 sendt,    t6 modtaget
//   Søger  -> Gemmer: REPORT (t2, t3, t6)   så Gemmeren har alle seks tidsstempler

// Modulernes adresser (skrives i chippen med UWBDriver::init)
constexpr uint16_t ADDR_GEMMER = 0x0001;
constexpr uint16_t ADDR_SOEGER = 0x0002;

// De seks tidsstempler fra én måling.
// Rå 40-bit værdier fra chippen: ét trin er ca. 15,65 ps (1 / (128 * 499,2 MHz)).
// Tælleren løber rundt ca. hvert 17. sekund, så differencer skal regnes modulo 2^40.
struct Timestamps
{
    uint64_t t1 = 0; // POLL sendt         (Gemmer)
    uint64_t t2 = 0; // POLL modtaget      (Søger)
    uint64_t t3 = 0; // RESPONSE sendt     (Søger)
    uint64_t t4 = 0; // RESPONSE modtaget  (Gemmer)
    uint64_t t5 = 0; // FINAL sendt        (Gemmer)
    uint64_t t6 = 0; // FINAL modtaget     (Søger)
};

class UWBRanging
{
private:
    using Frame = std::array<uint8_t, UWBDriver::MAX_FRAME_LEN>;

    UWBDriver &uwb;
    uint16_t myAddr;   // egen adresse
    uint16_t peerAddr; // modpartens adresse
    uint8_t seq = 0;   // sekvensnummer; tælles op for hver måling

    // Skriver de 10 header-bytes og returnerer længden
    size_t buildFrameHeader(Frame &frame, uint8_t type, uint8_t frameSeq) const;

    // Er beskeden til mig, fra modparten og af den forventede type?
    bool isExpected(const Frame &frame, size_t len, uint8_t type) const;

    // Pakker et 40-bit tidsstempel ind i / ud af 5 bytes i en besked
    static void putTimestamp(Frame &frame, size_t pos, uint64_t t);
    static uint64_t getTimestamp(const Frame &frame, size_t pos);

public:
    // driver: et UWBDriver, hvor init() allerede er kaldt
    UWBRanging(UWBDriver &driver, uint16_t ownAddress, uint16_t peerAddress);

    // Gemmer (initiator): kører én hel måling.
    // true og alle seks tidsstempler i ts, eller false hvis et trin fejlede.
    bool measure(Timestamps &ts);

    // Søger (responder): venter højst timeout på en POLL og gennemfører sin halvdel.
    // true hvis REPORT blev sendt, false ved timeout eller fejl.
    bool respond(std::chrono::milliseconds timeout);
};
