#include "GPIOPin.h"
#include <iostream>

GPIOPin::GPIOPin(unsigned int pin, const std::string& dir, const std::string& consumer)
    : pin_number(pin), direction(dir), consumer_name(consumer) {
        requestPin(direction);
    }

GPIOPin::~GPIOPin() {
    // Takket være RAII frigiver line_request automatisk pinnen her
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
            // Tilføj evt. pull-up hvis dit board kræver det (UWB har det ofte internt)
            // settings.set_bias(::gpiod::line::bias::PULL_UP); 
        }

        // 3. Åben GPIO-chippen (På RPi4 er hoved-GPIO'erne typisk på "gpiochip4")
        // Vi opbygger anmodningen med "Builder pattern"
        request = ::gpiod::chip("/dev/gpiochip4")
            .prepare_request()
            .set_consumer(consumer_name)
            .add_line_settings(pin_number, settings)
            .do_request();

        direction = dir;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "gpiod fejl på pin " << pin_number << ": " << e.what() << std::endl;
        return false;
    }
}

void GPIOPin::setDirection(const std::string& dir) {
    if (direction != dir) {
        requestPin(dir); // Skift retning i kernen ved at lave en ny anmodning
    }
}

void GPIOPin::write(int value) {
    if (!request || direction != "out") return;
    
    // Indstil værdien på pinnen (0 eller 1)
    request.set_value(pin_number, value == 1 ? ::gpiod::line::value::ACTIVE : ::gpiod::line::value::INACTIVE);
}

int GPIOPin::read() {
    if (!request) return 0;
    
    // Læs den rå spændingsværdi på pinnen
    auto val = request.get_value(pin_number);
    return (val == ::gpiod::line::value::ACTIVE) ? 1 : 0;
}
