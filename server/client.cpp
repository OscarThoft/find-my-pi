
#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main(int argc, char* argv[]) {

    // Kontrollerer, at serverens IP-adresse er angivet.
    if (argc != 2) {
        std::cerr << "Usage: ./client <server-ip>\n";
        return 1;
    }

    // Opretter en TCP-socket.
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Konfigurerer serverens adresse.
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Config::SERVER_PORT);

    // Konverterer serverens IP-adresse til binært format.
    if (inet_pton(AF_INET, argv[1],
                  &serverAddress.sin_addr) <= 0) {

        std::cerr << "Invalid server IP address\n";
        close(clientSocket);
        return 1;
    }

    // Opretter forbindelse til serveren.
    if (connect(clientSocket,
                (sockaddr*)&serverAddress,
                sizeof(serverAddress)) < 0) {

        std::cerr << "Connection failed\n";
        close(clientSocket);
        return 1;
    }

    std::cout << "[CLIENT] Connected to server!\n";

    // Buffer til modtagelse af data.
    char buffer[1024];

    // Gemmer modtagne data, indtil en komplet besked er modtaget.
    std::string receivedData;

    // Fortsætter med at modtage data fra serveren.
    while (true) {

        // Modtager data fra serveren.
        ssize_t bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        // Serveren har lukket forbindelsen, eller der er opstået en fejl.
        if (bytesReceived <= 0) {
            std::cout << "[CLIENT] Server disconnected\n";
            break;
        }

        // Tilføjer de modtagne bytes til vores datastrøm.
        receivedData.append(buffer, bytesReceived);

        // Finder komplette beskeder ved hjælp af newline.
        size_t newlinePosition;

        while ((newlinePosition = receivedData.find('\n'))
               != std::string::npos) {

            // Udtrækker en komplet besked.
            std::string message =
                receivedData.substr(0, newlinePosition);

            // Fjerner den behandlede besked fra datastrømmen.
            receivedData.erase(0, newlinePosition + 1);

            // Udskriver den modtagne besked.
            std::cout << "[CLIENT] Received: "
                      << message << std::endl;
        }
    }

    // Lukker TCP-forbindelsen.
    close(clientSocket);

    std::cout << "[CLIENT] Disconnected\n";

    return 0;
}
