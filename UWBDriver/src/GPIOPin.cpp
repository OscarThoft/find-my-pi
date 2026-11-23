#include "GPIOPin.h"
#include <iostream>
#include <stdexcept>

GPIOPin::GPIOPin(unsigned int pin, const std::string &dir, const std::string &consumer)
    : pin_number(pin), direction(dir), consumer_name(consumer)
{
    if (!requestPin(direction))
        throw std::runtime_error("Kunne ikke requeste GPIO " + std::to_string(pin_number));
}

GPIOPin::~GPIOPin()
{
    // line_request opgiver automatisk kontrol over pinnen
}

bool GPIOPin::requestPin(const std::string &dir)
{
    try
    {
        // 2. Definer indstillinger for pinnen
        auto settings = ::gpiod::line_settings();

        if (dir == "out")
        {
            settings.set_direction(::gpiod::line::direction::OUTPUT);
            settings.set_output_value(::gpiod::line::value::INACTIVE);
        }
        else
        {
            settings.set_direction(::gpiod::line::direction::INPUT);
            settings.set_bias(::gpiod::line::bias::DISABLED);
            settings.set_edge_detection(::gpiod::line::edge::RISING);
        }

        if (request)
        {
            // 2a. Pinnen er allerede vores: skift retning uden at slippe den
            ::gpiod::line_config config;
            config.add_line_settings(pin_number, settings);
            request->reconfigure_lines(config);
        }
        else
        {
            // 2b. Første gang: åbn chippen og lav en ny request
            request = ::gpiod::chip("/dev/gpiochip0")
                          .prepare_request()
                          .set_consumer(consumer_name)
                          .add_line_settings(pin_number, settings)
                          .do_request();
        }

        direction = dir;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "gpiod fejl på pin " << pin_number << ": " << e.what() << std::endl;
        return false;
    }
}

void GPIOPin::setDirection(const std::string &dir)
{
    if (direction != dir)
    {
        requestPin(dir); // skift retning i kernen ved at lave en ny anmodning
    }
}

void GPIOPin::write(int value)
{
    if (!request || direction != "out")
        return;

    // indstil værdien på pinnen (0 eller 1)
    request->set_value(pin_number, value == 1 ? ::gpiod::line::value::ACTIVE : ::gpiod::line::value::INACTIVE);
}

int GPIOPin::read()
{
    if (!request)
        return 0;

    // læs den rå spændingsværdi på pinnen
    auto val = request->get_value(pin_number);
    return (val == ::gpiod::line::value::ACTIVE) ? 1 : 0;
}

bool GPIOPin::waitForInterrupt(std::chrono::milliseconds timeout)
{
    if (!request || direction != "in")
        return false;

    try
    {
        // Blokerer, indtil der kommer en kant, eller timeout udløber
        if (!request->wait_edge_events(timeout))
            return false; // timeout, ingen interrupt

        // Tøm hændelserne, ellers returnerer næste kald med det samme
        ::gpiod::edge_event_buffer buffer;
        request->read_edge_events(buffer);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "gpiod fejl ved vent på pin " << pin_number << ": " << e.what() << std::endl;
        return false;
    }
}
