
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

int main() {

    // Opretter en TCP-socket.
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Tillader genbrug af porten efter nedlukning.
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // Konfigurerer serverens netværksadresse.
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Config::SERVER_PORT);

    // Lytter på alle netværksinterfaces (Wi-Fi og Ethernet).
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binder socketen til porten.
    if (bind(serverSocket,
             (sockaddr*)&serverAddress,
             sizeof(serverAddress)) < 0) {

        std::cerr << "Error binding socket\n";
        close(serverSocket);
        return 1;
    }

    // Sætter serveren i lyttetilstand.
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening\n";
        close(serverSocket);
        return 1;
    }

    std::cout << "[SERVER] Waiting for client on port "
              << Config::SERVER_PORT << "...\n";

    // Venter på, at clienten forbinder.
    sockaddr_in clientAddress{};
    socklen_t clientLength = sizeof(clientAddress);

    int clientSocket = accept(
        serverSocket,
        (sockaddr*)&clientAddress,
        &clientLength
    );

    if (clientSocket < 0) {
        std::cerr << "Error accepting client\n";
        close(serverSocket);
        return 1;
    }

    std::cout << "[SERVER] Client connected!\n";

    // Simuleret afstand til søgeren.
    // Senere kan denne værdi erstattes med en rigtig UWB-måling.
    double distance = 20.0;

    // Sender afstandsdata, indtil afstanden når 0 meter.
    while (distance >= 0.0) {

        // Opretter beskeden, som skal sendes til clienten.
        std::string message =
            "DISTANCE " + std::to_string(distance) + "\n";

        // Sender beskeden til clienten.
        // Vi bruger en løkke, så alle bytes bliver sendt.
        size_t totalSent = 0;

        while (totalSent < message.size()) {

            ssize_t sent = send(
                clientSocket,
                message.c_str() + totalSent,
                message.size() - totalSent,
                MSG_NOSIGNAL
            );

            if (sent <= 0) {
                std::cerr << "[SERVER] Send failed\n";
                close(clientSocket);
                close(serverSocket);
                return 1;
            }

            totalSent += sent;
        }

        // Udskriver den sendte besked i serverens terminal.
        std::cout << "[SERVER] Sent: " << message;

        // Reducerer den simulerede afstand med 0,2 meter.
        distance -= 0.2;

        // Venter 200 ms mellem hver besked (5 gange i sekundet).
        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                Config::UPDATE_INTERVAL_MS
            )
        );
    }

    // Lukker forbindelsen til clienten.
    close(clientSocket);
    close(serverSocket);

    std::cout << "[SERVER] Disconnected\n";

    return 0;
}
