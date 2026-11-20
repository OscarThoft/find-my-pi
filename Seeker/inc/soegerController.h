#pragma once
#include "game_session.h"
#include "UI.h"

class soegerController {
public:
    void startGame();

    void found();

    double calculateTime() const; 
    //Returnerer tiden for den nuværende session
    //Hvis spillet er RUNNING returnere den tiden indtil videre, FOUND returnere den tiden for spillet

private:
    double distance; //Gemmer distancen fra gameData
    void initSPI(); //Init af UWB og RPI kommunikation

    //Instanser af klasser:
    game_session session;
    UI ui;
};

