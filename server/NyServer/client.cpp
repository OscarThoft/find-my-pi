#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <json_dto/pub.hpp>
#include <exception>

// SØGER !! 

struct GameStateOgData
{
    std::string state;
    double distance;

    template <typename JSON_IO>
    void json_io(JSON_IO &io)
    {
        io &json_dto::mandatory("state", state);
        io &json_dto::mandatory("distance", distance);
    }
};

int connectToServer()
{
    // 1. Opret en TCP-socket til IPv4
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("socket");
        return -1;
    }
    // 2. Angiv den server, vi vil forbinde til
    sockaddr_in serverAddress{}; // opretter/nulstiller struct som holder serverens adresse
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);

    // omdanner IP-adressen fra tekst til binær form, som socketen kan bruge.
    // og tjekker om ip adressen er gyldig
    if (inet_pton(AF_INET, "172.16.15.2",
                  &serverAddress.sin_addr) != 1)
    {
        std::cerr << "Ugyldig IP-adresse\n";
        close(clientSocket);
        return -1;
    }
    // 3. Opret forbindelse til serveren
    if (connect(clientSocket, (sockaddr *)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("connect");
        close(clientSocket);
        return -1;
    }
    return clientSocket;
}

void startGame(int clientSocket)
{
    std::string request =
        "POST /gamestart HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    send(clientSocket, request.c_str(), request.size(), 0);

    // modtager svaret
    std::string svar;
    char buffer[1024];

    while (true)
    {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
            retur - 3; // Fejl eller lukket forbindelse

        svar.append(buffer, antal);

        auto slut = svar.find("\r\n\r\n");
        if (slut == std::string::npos)
            continue; // Headers er endnu ikke modtaget færdig

        auto pos = svar.find("Content-Length:");
        if (pos == std::string::npos || pos > slut)
            return -3; // fejl

        auto length = std::stoul(svar.substr(pos + 15));
        
        // venter til hele body er modtaget
        
        if (svar.size() - (slut + 4) < length)
         continue;

    }

}

double getData(int clientSocket)
{
    // laver en string som vi sender til server
    std::string request =
        "GET /gamestatus HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n"
        "\r\n";

    // sender requesten
    std::size_t sendt = 0;
    while (sendt < request.size())
    {
        auto antal = send(clientSocket,
                          request.c_str() + sendt,
                          request.size() - sendt, 0);
        if (antal <= 0)
            return;
        sendt += antal;
    }

    // modtager svaret
    std::string svar;
    char buffer[1024];

    while (true)
    {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
            retur - 3; // Fejl eller lukket forbindelse

        svar.append(buffer, antal);

        auto slut = svar.find("\r\n\r\n");
        if (slut == std::string::npos)
            continue; // Headers er endnu ikke modtaget færdig

        auto pos = svar.find("Content-Length:");
        if (pos == std::string::npos || pos > slut)
            return -3; // fejl

        try
        {
            auto length = std::stoul(svar.substr(pos + 15));

            // venter til hele body er modtaget
            if (svar.size() - (slut + 4) < length)
                continue;

            auto data = json_dto::from_json<GameStateOgData>(svar.substr(slut + 4, length));

            if (data.state == "Running")
            {
                return data.distance;
            }

            if (data.state == "Found")
            {
                return -1.0;
            }

            if (data.state == "Inactive")
            {
                return -2.0;
            }

            return -3; // ukendt status
        }
        catch (const std::exception &)
        {
            return -3; // uguldig længde eller JSON
        }
    }
}

int main()
{
    int clientSocket = connectToServer();
    if (clientSocket < 0)
    {
        return 1;
    }

    startGame(clientSocket);

    // et loop der:
    double afstand = getData(clientSocket);
    if (afstand = -0, 1)
    {
        // kald function der sættes status til found?
    }

    // printer afstand i terminalen

    // uden for loope (når status = found)
    // print spilet er færdig gemmere blev fundet på xx tid osv....

    close(clientSocket);
    return 0;
}
