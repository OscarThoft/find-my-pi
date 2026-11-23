#include <iostream>
#include <string>
#include <cstdio>
#include <exception>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <json_dto/pub.hpp>

// SOEGER: 
// Kør PÅ SERVER "ip -br addr" for at se wifi ip adresser
//  ret IP-adressen 3 stedet til servers IP
// ret i inet_pton() og i begge host
// byg igen ( cmake --build build --target client )


struct GameStateOgData {
    std::string state;
    double distance;

    template<typename JSON_IO>
    void json_io(JSON_IO& io)
    {
        io & json_dto::mandatory("state", state);
        io & json_dto::mandatory("distance", distance);
    }
};

int connectToServer()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket");
        return -1;
    }
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);

   if (inet_pton(AF_INET, "172.16.15.2", &serverAddress.sin_addr) != 1) {
        std::cerr << "Ugyldig IP-adresse\n";
        close(clientSocket);
        return -1;
    }
    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("connect");
        close(clientSocket);
        return -1;
    }
    return clientSocket;
}

bool startGame(int clientSocket)
{
    std::string request =
        "POST /gamestart HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n"
        "Content-Length: 0\r\n"
        "\r\n";

    std::size_t sendt = 0;
    while (sendt < request.size()) {
        auto antal = send(clientSocket, request.c_str() + sendt,
                          request.size() - sendt, MSG_NOSIGNAL);
        if (antal <= 0)
            return false;
        sendt += antal;
    }

    // 204 har ingen body: modtag alle headers og tjek statuskoden.
    std::string svar;
    char buffer[1024];
    while (svar.find("\r\n\r\n") == std::string::npos) {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
            return false;
        svar.append(buffer, antal);
    }
    auto pos = svar.find(' ');
    return pos != std::string::npos && svar.substr(pos + 1, 4) == "204 ";
}

double getData(int clientSocket)
{
    std::string request =
        "GET /gamestatus HTTP/1.1\r\n"
        "Host: 172.16.15.2:9000\r\n"
        "\r\n";

    std::size_t sendt = 0;
    while (sendt < request.size()) {
        auto antal = send(clientSocket, request.c_str() + sendt,
                          request.size() - sendt, MSG_NOSIGNAL);
        if (antal <= 0)
            return -3.0;
        sendt += antal;
    }

    std::string svar;
    char buffer[1024];
    while (true) {
        auto antal = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (antal <= 0)
            return -3.0;
        svar.append(buffer, antal);

        auto slut = svar.find("\r\n\r\n");
        if (slut == std::string::npos)
            continue;

        auto statusPos = svar.find(' ');
        if (statusPos == std::string::npos || svar.substr(statusPos + 1, 4) != "200 ")
            return -3.0;

        auto pos = svar.find("Content-Length:");
        if (pos == std::string::npos || pos > slut)
            return -3.0;

        try {
            auto length = std::stoul(svar.substr(pos + 15));
            if (svar.size() - (slut + 4) < length)
                continue;

            auto data = json_dto::from_json<GameStateOgData>(
                svar.substr(slut + 4, length));
            if (data.state == "Running") return data.distance;
            if (data.state == "Found") return -1.0;
            if (data.state == "Inactive") return -2.0;
            return -3.0;
        }
        catch (const std::exception&) {
            return -3.0;
        }
    }
}

int main()
{
    int clientSocket = connectToServer();
    if (clientSocket < 0)
        return 1;

    if (!startGame(clientSocket)) {
        std::cerr << "Kommunikationsfejl: spilstart blev ikke bekraeftet.\n";
        close(clientSocket);
        return 1;
    }
    std::cout << "Serveren har bekraeftet spilstart.\n";


    double afstand = getData(clientSocket);
    if (afstand == -1.0)
        std::cout << "Gemmeren er fundet!\n";
    else if (afstand == -2.0)
        std::cout << "Spillet er inaktivt.\n";
    else if (afstand == -3.0)
        std::cerr << "Kommunikationsfejl eller ugyldigt svar.\n";
    else
        std::cout << "Afstand: " << afstand << " meter\n";

    close(clientSocket);
    return afstand == -3.0 ? 1 : 0;
}
