#include "../inc/soegerController.h"
#include <iostream>
#include <unistd.h>
#include <chrono>

int soegerController::startGame() 
{
    //Etabler forbindelse til serveren
    socket = client.connectToServer();
    if (socket < 0)
    {
        return -1;
    }

    if (!client.startGame(socket))
    {
        close(socket);
        return -1;
    }

    //Init UWB
    initSPI(); 

    return 1;
}

void soegerController::initSPI() {
    //Init for UWB (kommer senere)
}

void soegerController::found() {
    session.setStatus(game_session::FOUND);
    ui.showFound(calculateTime());
}

double soegerController::calculateTime() const {
    double startTime = session.getStartTime();
    double endTime = session.getEndTime();    

    if (startTime == 0.0) {
        std::cout << "Failed: Time not started \n";
        return 0.0;
    }


    if(session.getStatus() == game_session::FOUND) {
        return endTime - startTime; //Returnere tiden det tog i sekunder
    }

    auto duration = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()
        );

    double currentTime = duration.count();

    return currentTime - startTime; //Returnere tiden det har taget foreløbigt

}

void soegerController::run() 
{
    //Initialize client and UWB
    int s = startGame();

    while(s == -1) {
        ui.connectionFailed();
        s = startGame();
    }

    if(ui.startMenu()) {
        session.setStatus(game_session::RUNNING);
    } else {
        close(socket);
        return;
    }

    bool flag = 1;
    while(flag) 
    {
        while(session.getStatus() == game_session::RUNNING) {
            double data = client.getData(socket);
            
            if (data == -1.0){
                session.setStatus(game_session::FOUND);
                double time = calculateTime();
                ui.showFound(time);
            }
            else if (data == -2.0)
            {
                std::cout << "Spillet er ikke startet\n";  //Skal evt fjernes.
            }
            else if (data == -3.0)
            {
                std::cerr << "Kommunikationsfejl eller ugyldigt svar\n";
                close(socket);
            }
            else
            {
                std::cout << "Afstand: " << data << '\n';
            }
        }

        if(ui.playAgain()) 
        {
            session.setStatus(game_session::RUNNING);
        } else 
        {
            flag = 0;
        }
    }
    
}

