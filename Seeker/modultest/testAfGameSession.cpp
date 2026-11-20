#include "game_session.h"
#include <iostream>
#include <chrono>

int main(){
    game_session g;
    //Test med sammenligning af chrono tid og tiden vores game_session måler
    //Start tid
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    g.setStatus(g.RUNNING);

    int a = 0;

    for(int i=0;i <100000000;i++) {
        a++;
    }

    //Slut tid
    g.setStatus(g.FOUND);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    //Beregn tid brugt:
    std::chrono::duration<double> res_c = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    double res_g = g.getTimeRunning();

    std::cout<<"Time running (game_session): " << res_g << "\n";
    std::cout<<"Time running (chrono): " << res_c.count() << "\n";


    //Test af andre funktionaliteter og edge cases:
    g.setStatus(g.INACTIVE);
    std::cout << "Time running (should return fail message): " << g.getTimeRunning() << "\n";

    g.setStatus(g.RUNNING);

    for(int i=0;i <10000000;i++) {
        a++;
    }

    std::cout << "Halfway time running: " << g.getTimeRunning() << "\n";
    g.setStatus(g.RUNNING);
    std::cout << "Test time is not reset: " << g.getTimeRunning() << "\n";

    for(int i=0;i <10000000;i++) {
        a++;
    }

    g.setStatus(g.FOUND);
    std::cout << "Full time running: " << g.getTimeRunning() << "\n";

}