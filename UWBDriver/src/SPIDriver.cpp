#include "SPIDriver.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <iostream>

SPIDriver::SPIDriver() 
    : speed_hz(2000000), mode(0), bits_per_word(8) { //2MHz datatransferspeed, SPImode 0, bits_per_word 8
    openSPI()
}

SPIDriver::~SPIDriver() {
    closeSPI();
}

bool SPIDriver::openSPI() {
    // åbn SPI enheden. hardcodet til "/dev/spidev0.0" 
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "Fejl: Kunne ikke åbne SPI enhed." << std::endl;
        return false;
    }

    // indstil SPI Mode
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) return false;
    
    // indstil Bits per word
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) return false;
    
    // indstil Max Speed
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