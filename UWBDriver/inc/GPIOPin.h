#pragma once
#include <gpiod.hpp> //kræver at man installerer libgpiod: sudo apt install libgpiod-dev
#include <string>   //og ved compile, skal der linkes med -lgpiod flaget.

//klassen styrer en enkelt pin
class GPIOPin {
private:
    unsigned int pin_number;
    std::string direction;      // in/out
    std::string consumer_name;  // gpiod.hpp skal bruge et navn, der ejer GPIO-pinnen
    
    // handle til den aktive GPIO-request i kernelen
    ::gpiod::line_request request;

    // intern hjælpefunktion til at genanmode om pinnen, når retningen ændres
    bool requestPin(const std::string& dir);

public:
    GPIOPin(unsigned int pin, const std::string& dir, const std::string& consumer = "UWB");
    ~GPIOPin();

    void setDirection(const std::string& dir);
    void write(int value); // 1 = Høj (3.3V), 0 = Lav (0V)
    int read();            // returnerer 1 eller 0
};