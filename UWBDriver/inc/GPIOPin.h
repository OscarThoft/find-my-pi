#pragma once
#include <gpiod.hpp> //kræver at man installerer libgpiod: sudo apt install libgpiod-dev
#include <string>   //og ved compile, skal der linkes med -lgpiod flaget.

class GPIOPin {
private:
    unsigned int pin_number;
    std::string direction;
    std::string consumer_name;
    
    // Håndtaget til den aktive GPIO-anmodning i kernen
    ::gpiod::line_request request;

    // Intern hjælpefunktion til at genanmode om pinnen, når retningen ændres
    bool requestPin(const std::string& dir);

public:
    GPIOPin(unsigned int pin, const std::string& dir, const std::string& consumer = "UWB");
    ~GPIOPin();

    void setDirection(const std::string& dir);
    void write(int value); // 1 = Høj (3.3V), 0 = Lav (0V)
    int read();            // Returnerer 1 eller 0
};
