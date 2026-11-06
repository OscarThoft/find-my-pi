
#ifndef CONFIG_H
#define CONFIG_H

namespace Config {

    // Network configuration
    constexpr int SERVER_PORT = 9000;

    // Update frequency: 5 times per second
    constexpr int UPDATE_INTERVAL_MS = 200;

    // UWB configuration
    constexpr int SPI_BUS = 0;
    constexpr int SPI_DEVICE = 0;

    // Server (Gemmer)
    constexpr int SERVER_UWB_RST_PIN = -1;
    constexpr int SERVER_UWB_IRQ_PIN = -1;
    constexpr int BUTTON_PIN = -1;

    // Client (Søger)
    constexpr int CLIENT_UWB_RST_PIN = -1;
    constexpr int CLIENT_UWB_IRQ_PIN = -1;

}

#endif
