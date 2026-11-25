#pragma once
#include "game_session.h"
#include "UI.h"
#include "../../server/client.h"

class soegerController {
public:
    int startGame();

    void found();

    double calculateTime() const;  //Returnerer tiden for den nuværende session
    //Hvis spillet er RUNNING returnere den tiden indtil videre, ved FOUND returnere den tiden for spillet

    void run();

private:
    double distance; //Gemmer distancen fra gameData
    void initSPI(); //Init af UWB og RPI kommunikation
    int socket; //Client socket

    //Instanser af klasser:
    game_session session;
    UI ui;
    Client client;
};



