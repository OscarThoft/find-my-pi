#include "GPIOPin.h"
#include <iostream>

GPIOPin::GPIOPin(unsigned int pin, const std::string& dir, const std::string& consumer)
    : pin_number(pin), direction(dir), consumer_name(consumer) {
        requestPin(direction);
    }

GPIOPin::~GPIOPin() {
    // line_request opgiver automatisk kontrol over pinnen
}

bool GPIOPin::requestPin(const std::string& dir) {
    try {
        // 1. Frigiv tidligere anmodning, hvis den eksisterer (vigtigt ved retningsskift)
        if (request) {
            request.release();
        }

        // 2. Definer indstillinger for pinnen
        auto settings = ::gpiod::line_settings();
        
        if (dir == "out") {
            settings.set_direction(::gpiod::line::direction::OUTPUT);
        } else {
            settings.set_direction(::gpiod::line::direction::INPUT);
        }

        // 3. Åbn GPIO-chippen
        request = ::gpiod::chip("/dev/gpiochip4")       // åbn
            .prepare_request()                          // lav en tom request
            .set_consumer(consumer_name)                // indsætter ejernavn over pinnen
            .add_line_settings(pin_number, settings)    // indsætter indstillinger
            .do_request();                              // afsender request

        direction = dir;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "gpiod fejl på pin " << pin_number << ": " << e.what() << std::endl;
        return false;
    }
}

void GPIOPin::setDirection(const std::string& dir) {
    if (direction != dir) {
        requestPin(dir); // skift retning i kernen ved at lave en ny anmodning
    }
}

void GPIOPin::write(int value) {
    if (!request || direction != "out") return;
    
    // indstil værdien på pinnen (0 eller 1)
    request.set_value(pin_number, value == 1 ? ::gpiod::line::value::ACTIVE : ::gpiod::line::value::INACTIVE);
}

int GPIOPin::read() {
    if (!request) return 0;
    
    // læs den rå spændingsværdi på pinnen
    auto val = request.get_value(pin_number);
    return (val == ::gpiod::line::value::ACTIVE) ? 1 : 0;
}