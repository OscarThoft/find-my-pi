
#include <iostream>       // Bruges til input og output (cout og cerr)
#include <string>         // Gør det muligt at arbejde med strings
#include <cstring>        // Indeholder funktioner til håndtering af data
#include <unistd.h>       // Indeholder close() til at lukke sockets
#include <arpa/inet.h>    // Indeholder funktioner til IP-adresser og porte
#include <sys/socket.h>   // Indeholder funktioner til TCP sockets

#include "config.h"       // Henter vores fælles konfiguration, fx serverport

int main() {

    // Opretter en TCP-socket.
    // AF_INET betyder IPv4.
    // SOCK_STREAM betyder TCP-forbindelse.
    // 0 vælger automatisk den korrekte TCP-protokol.
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Kontrollerer, om socket blev oprettet korrekt.
    if (serverSocket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Tillader, at porten kan genbruges,
    // så vi lettere kan genstarte serveren efter nedlukning.
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // Opretter og konfigurerer serverens netværksadresse.
    sockaddr_in serverAddress{};

    // Angiver, at vi bruger IPv4.
    serverAddress.sin_family = AF_INET;

    // Angiver hvilken port serveren skal lytte på.
    // htons() konverterer portnummeret til network byte order.
    serverAddress.sin_port = htons(Config::SERVER_PORT);

    // INADDR_ANY betyder, at serveren accepterer
    // forbindelser på alle netværksinterfaces.
    // Det gør det muligt at forbinde via både Wi-Fi og Ethernet.
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binder socketen til den valgte IP-adresse og port.
    // Serveren reserverer dermed porten til kommunikation.
    if (bind(serverSocket,
             (sockaddr*)&serverAddress,
             sizeof(serverAddress)) < 0) {

        std::cerr << "Error binding socket\n";

        // Lukker socketen, hvis bindingen fejler.
        close(serverSocket);
        return 1;
    }

    // Sætter serveren i lyttetilstand.
    // Tallet 5 angiver størrelsen på køen af ventende forbindelser.
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening\n";
        close(serverSocket);
        return 1;
    }

    // Fortæller, at serveren er startet og venter på en client.
    std::cout << "[SERVER] Waiting for client on port "
              << Config::SERVER_PORT << "...\n";

    // Opretter en variabel til at gemme clientens adresse.
    sockaddr_in clientAddress{};

    // Angiver størrelsen på clientens adresseinformation.
    socklen_t clientLength = sizeof(clientAddress);

    // Venter på, at en client opretter forbindelse.
    // accept() blokerer programmet, indtil en client forbinder.
    // Der oprettes en ny socket til kommunikationen med clienten.
    int clientSocket = accept(
        serverSocket,
        (sockaddr*)&clientAddress,
        &clientLength
    );

    // Kontrollerer, om forbindelsen blev oprettet korrekt.
    if (clientSocket < 0) {
        std::cerr << "Error accepting client\n";
        close(serverSocket);
        return 1;
    }

    // Clienten er nu forbundet til serveren.
    std::cout << "[SERVER] Client connected!\n";

    // Opretter en buffer til midlertidigt at modtage data.
    // Bufferen kan indeholde op til 1024 bytes ad gangen.
    char buffer[1024];

    // Gemmer modtagne data, indtil vi har en komplet besked.
    std::string receivedData;

    // Fortsætter med at modtage data, så længe forbindelsen er aktiv.
    while (true) {

        // Modtager data fra clienten.
        // recv() returnerer antallet af bytes, der er modtaget.
        ssize_t bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        // Hvis recv() returnerer 0, har clienten lukket forbindelsen.
        // Hvis værdien er negativ, er der opstået en fejl.
        if (bytesReceived <= 0) {
            std::cout << "[SERVER] Client disconnected\n";
            break;
        }

        // Tilføjer de modtagne bytes til vores samlede besked.
        // Vi bruger bytesReceived, så vi kun tilføjer de bytes,
        // der faktisk blev modtaget.
        receivedData.append(buffer, bytesReceived);

        // TCP sender en datastrøm og garanterer ikke,
        // at hver send() svarer til én recv().
        // Derfor leder vi efter newline (\n), som markerer slutningen
        // på en besked.
        size_t newlinePosition;

        // Finder og behandler alle komplette beskeder i datastrømmen.
        while ((newlinePosition = receivedData.find('\n'))
               != std::string::npos) {

            // Udtrækker beskeden frem til newline.
            std::string message =
                receivedData.substr(0, newlinePosition);

            // Fjerner den behandlede besked og newline fra datastrømmen.
            // Eventuelle resterende data bevares til næste besked.
            receivedData.erase(0, newlinePosition + 1);

            // Udskriver den modtagne besked i terminalen.
            std::cout << "[SERVER] Received: "
                      << message << std::endl;
        }
    }

    // Lukker forbindelsen til clienten.
    close(clientSocket);

    // Lukker serverens lyttesocket.
    close(serverSocket);

    return 0;
}
