#pragma once
#include <cstdint>

// Registre, sub-adresser og bit i DW1000 (UWB Click).
// Kilde: DW1000 User Manual. REG_ = registernummer, SUB_ = offset inde i registret.

// ---------- Registre ----------
constexpr uint8_t REG_DEV_ID = 0x00;
constexpr uint8_t REG_PANADR = 0x03;
constexpr uint8_t REG_TX_FCTRL = 0x08;
constexpr uint8_t REG_TX_BUFFER = 0x09;
constexpr uint8_t REG_SYS_CTRL = 0x0D;
constexpr uint8_t REG_SYS_STATUS = 0x0F;
constexpr uint8_t REG_RX_FINFO = 0x10;
constexpr uint8_t REG_RX_BUFFER = 0x11;
constexpr uint8_t REG_RX_TIME = 0x15;
constexpr uint8_t REG_TX_TIME = 0x17;
constexpr uint8_t REG_TX_ANTD = 0x18;
constexpr uint8_t REG_TX_POWER = 0x1E;

constexpr uint8_t REG_AGC_CTRL = 0x23;
constexpr uint16_t SUB_AGC_TUNE1 = 0x04;
constexpr uint16_t SUB_AGC_TUNE2 = 0x0C;

constexpr uint8_t REG_DRX_CONF = 0x27;
constexpr uint16_t SUB_DRX_TUNE2 = 0x08;

constexpr uint8_t REG_RF_CONF = 0x28;
constexpr uint16_t SUB_RF_TXCTRL = 0x0C;

constexpr uint8_t REG_TX_CAL = 0x2A;
constexpr uint16_t SUB_TC_PGDELAY = 0x0B;

constexpr uint8_t REG_FS_CTRL = 0x2B;
constexpr uint16_t SUB_FS_PLLTUNE = 0x0B;

constexpr uint8_t REG_OTP_IF = 0x2D;
constexpr uint16_t SUB_OTP_CTRL = 0x06;

constexpr uint8_t REG_LDE_CTRL = 0x2E;
constexpr uint16_t SUB_LDE_CFG1 = 0x0806;
constexpr uint16_t SUB_LDE_RXANTD = 0x1804;
constexpr uint16_t SUB_LDE_CFG2 = 0x1806;
constexpr uint16_t SUB_LDE_REPC = 0x2804; // tærskel for første signal; skal passe til præambelkoden

constexpr uint8_t REG_PMSC = 0x36;
constexpr uint16_t SUB_PMSC_CTRL0 = 0x00;
constexpr uint16_t SUB_PMSC_SOFTRESET = 0x03; // øverste byte af PMSC_CTRL0

// ---------- SYS_CTRL (0x0D): kommandoer til sender og modtager ----------
constexpr uint8_t SYS_CTRL_TXSTRT = 0x02;    // bit 1: start sending
constexpr uint8_t SYS_CTRL_TRXOFF = 0x40;    // bit 6: sluk sender og modtager
constexpr uint8_t SYS_CTRL_WAIT4RESP = 0x80; // bit 7: tænd modtageren efter sending
constexpr uint16_t SYS_CTRL_RXENAB = 0x0100; // bit 8: tænd modtageren

// ---------- SYS_STATUS (0x0F): hvad der er sket ----------
constexpr uint32_t SYS_STATUS_TXFRS = 0x00000080;       // bit 7:  frame sendt
constexpr uint32_t SYS_STATUS_LDEDONE = 0x00000400;     // bit 10: modtagetidsstemplet er klar
constexpr uint32_t SYS_STATUS_RXFCG = 0x00004000;       // bit 14: frame modtaget, CRC ok
constexpr uint32_t SYS_STATUS_ALL_TX = 0x000000F0;      // alle sende-flag (bit 4-7)
constexpr uint32_t SYS_STATUS_ALL_RX_GOOD = 0x00006F00; // alle flag ved en god modtagelse
constexpr uint32_t SYS_STATUS_ALL_RX_ERR = 0x24059000;  // alle modtagefejl (CRC, header, SFD ...)
constexpr uint32_t SYS_STATUS_ALL_RX_TO = 0x00220000;   // alle modtage-timeouts
constexpr uint32_t SYS_STATUS_ALL_RX = SYS_STATUS_ALL_RX_GOOD | SYS_STATUS_ALL_RX_ERR |
                                       SYS_STATUS_ALL_RX_TO;