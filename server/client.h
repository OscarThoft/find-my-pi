#pragma once
#include <string>
#include <json_dto/pub.hpp>

// søger h fil 

class Client {
public:
    int connectToServer();
    bool startGame(int clientSocket); 
    double getData(int clientSocket);

private:
    struct GameStateAndData
    {
        std::string state;
        double distance;

        template <typename JSON_IO>
        void json_io(JSON_IO &io)
        {
            io & json_dto::mandatory("state", state);
            io & json_dto::mandatory("distance", distance);
        }
    };


};