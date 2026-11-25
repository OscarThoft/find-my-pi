#ifndef TIMESTAMPS_H
#define TIMESTAMPS_H

#include <cstdint>

struct Timestamps{
    uint64_t t1 = 0; // POLL sendt         (Gemmer)
    uint64_t t2 = 0; // POLL modtaget      (Søger)
    uint64_t t3 = 0; // RESPONSE sendt     (Søger)
    uint64_t t4 = 0; // RESPONSE modtaget  (Gemmer)
    uint64_t t5 = 0; // FINAL sendt        (Gemmer)
    uint64_t t6 = 0; // FINAL modtaget     (Søger)
};

#endif