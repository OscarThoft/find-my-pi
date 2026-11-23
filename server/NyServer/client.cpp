#include <iostream>
#include <cstdio>
#include <exception>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <json_dto/pub.hpp>
#include "client.h"

// SØGER! med HTTP

// Kør PÅ SERVER "ip -br addr" for at se wifi ip adresser
//  ret IP-adressen 3 stedet til servers IP
// ret i inet_pton() og i begge host
// byg igen ( cmake --build build --target client )

int Client::connectToServer() 
{
    // 1. Opret en TCP-socket til IPv4
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("socket");
        return -1;
    }

    // 2. Angiv den server, vi vil forbinde til
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);

    // Omdan IP-adressen fra tekst til binær form,
    // og tjek, om adressens format er gyldigt
    if (inet_pton(AF_INET, "172.16.15.2",  // RET IP ADRESSE HER
                  &serverAddress.sin_addr) != 1)
    {
        std::cerr << "Ugyldig IP adresse\n";
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

bool Client::startGame(int clientSocket)
{
    std::string request =
        "POST /gamestart HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n"  // ret ip adresse her 
        "Content-Length: 0\r\n"
        "\r\n";

    // Send hele requesten
    std::size_t sendt = 0;
    while (sendt < request.size())
    {
        auto antal = send(clientSocket, request.c_str() + sendt,
                request.size() - sendt, MSG_NOSIGNAL);

        if (antal <= 0)
        {
            std::cerr << "Kommunikationsfejl ved spilstart\n";
            return false;
        }
        sendt += antal;
    }

    // Modtag headers. Det forventede 204-svar har ingen body
    std::string svar;
    char buffer[1024];

    while (svar.find("\r\n\r\n") == std::string::npos)
    {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
        {
            std::cerr << "Kommunikationsfejl: fejl i svar fra server\n";
            return false;
        }

        svar.append(buffer, antal);
    }

    // Tjek HTTP-statuskoden i svarets første linje
    auto mellemrum = svar.find(' ');
    if (mellemrum == std::string::npos ||
        svar.substr(mellemrum + 1, 4) != "204 ")
    {
        std::cerr << "Serveren bekræftede ikke spilstart med 204\n";
        return false;
    }

    return true;
}

double Client::getData(int clientSocket)
{
    // Lav en string, som vi sender til serveren
    std::string request =
        "GET /gamestatus HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n" // ret ip adresse her 
        "\r\n";

    // Send hele requesten
    std::size_t sendt = 0;
    while (sendt < request.size())
    {
        auto antal = send(clientSocket,
                          request.c_str() + sendt,
                          request.size() - sendt, MSG_NOSIGNAL);

        if (antal <= 0) 
        { 
            return -3.0; 
        }

        sendt += antal;
    }

    // Modtag svaret
    std::string svar;
    char buffer[1024];

    while (true)
    {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
            return -3.0; // Fejl eller lukket forbindelse

        svar.append(buffer, antal);

        auto slut = svar.find("\r\n\r\n");
        if (slut == std::string::npos)
            continue; // Headers er endnu ikke modtaget færdig

        // Tjek, at serveren svarer 200 OK
        auto mellemrum = svar.find(' ');
        if (mellemrum == std::string::npos ||
            svar.substr(mellemrum + 1, 4) != "200 ")
            return -3.0;

        auto pos = svar.find("Content-Length:");
        if (pos == std::string::npos || pos > slut)
            return -3.0;

        try
        {
            auto length = std::stoul(svar.substr(pos + 15));

            // Vent, indtil hele bodyen er modtaget
            if (svar.size() - (slut + 4) < length)
                continue;

            auto data = json_dto::from_json<GameStateAndData>(
                svar.substr(slut + 4, length));

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

            return -3.0; // Ukendt status
        }
        catch (const std::exception &)
        {
            return -3.0; // Ugyldig længde eller JSON
        }
    }
}

int main()
{
    Client client;

    int clientSocket = client.connectToServer();

    if (clientSocket < 0)
    {
        return 1;
    }

    if (!client.startGame(clientSocket))
    {
        close(clientSocket);
        return 1;
    }

    // Et loop, der:
    double afstand = client.getData(clientSocket);

    if (afstand == -1.0)
    {
        // Kald funktionen, der sætter søgerens status til FOUND
    }
    else if (afstand == -2.0)
    {
        std::cout << "Spillet er ikke startet\n";
    }
    else if (afstand == -3.0)
    {
        std::cerr << "Kommunikationsfejl eller ugyldigt svar\n";
        close(clientSocket);
        return 1;
    }
    else
    {
        std::cout << "Afstand: " << afstand << '\n';
    }

    // Uden for loopet, når status er FOUND:
    // Print, at spillet er færdigt, og hvor lang tid det tog

    close(clientSocket);
    return 0;
}