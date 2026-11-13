#include "game_session.h"
#include <iostream>

int main(){
    game_session session;

    session.setStatus(game_session::LOST);
    std::cout << "Status er: " << session.getStatus()<<std::endl;
    std::cout << "Tid for nuværende session: " << session.getTimeRunning()<<std::endl;

}