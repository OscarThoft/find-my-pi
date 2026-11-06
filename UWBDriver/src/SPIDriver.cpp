#include "SPIDriver.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <iostream>

SPIDriver::SPIDriver(uint32_t speed, uint8_t mode) 
    : speed_hz(speed), mode(mode), bits_per_word(8) {
    openSPI()
}

SPIDriver::~SPIDriver() {
    closeSPI();
}

bool SPIDriver::openSPI() {
    // Åben SPI enheden (bruger /dev/spidevX.Y)
    spi_fd = open("/dev/spidev0.0", O_RDWR); // Tilpas enhed efter behov
    if (spi_fd < 0) {
        std::cerr << "Fejl: Kunne ikke åbne SPI enhed." << std::endl;
        return false;
    }

    // Indstil SPI Mode (0, 1, 2 eller 3)
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) return false;
    
    // Indstil Bits per word (typisk 8 bit)
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) return false;
    
    // Indstil Max Speed (Hz)
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) return false;

    return true;
}

void SPIDriver::closeSPI() {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
}

bool SPIDriver::transfer(uint8_t* tx_buf, uint8_t* rx_buf, size_t length) {
    if (spi_fd < 0) return false;

    // spidev_ioc_transfer er den struktur kernelen forventer
    struct spi_ioc_transfer spi;
    std::memset(&spi, 0, sizeof(spi));

    spi.tx_buf = (unsigned long)tx_buf;
    spi.rx_buf = (unsigned long)rx_buf;
    spi.len = length;
    spi.speed_hz = speed_hz;
    spi.bits_per_word = bits_per_word;
    //spi.delay_usecs = 0;

    // Udfør selve overførslen i hardwaren
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi) < 0) {
        std::cerr << "Fejl under SPI transfer ioctl." << std::endl;
        return false;
    }
    return true;
}
