#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string> 
 
 // 1. Opret en TCP-socket til IPv4
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("socket");
        return 1;
    }

    // 2. Angiv serverens adresse og port
    sockaddr_in serverAddress{}; // {} nulstiller structen
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(9000);

    // 3. Knyt socketen til porten
    if (bind(serverSocket, (sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        perror("bind");
        close(serverSocket);
        return 1;
    }

    // 4. Gør serveren klar til at modtage forbindelser
    if (listen(serverSocket, 5) < 0)
    {
        perror("listen");
        close(serverSocket);
        return 1;
    }

    std::cout << "Venter på en klient..." << std::endl;

    // 5. Vent på en klient. Returnerer en NY socket til klienten
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0)
    {
        perror("accept");
        close(serverSocket);
        return 1;
    }

    std::cout << "Klient forbundet!" << std::endl;

    // HER kan vi senere sende og modtage via clientSocket

    // eksempel:

    // Vent på besked fra søgeren
    char besked[16]{};
    int antal = recv(clientSocket, besked, sizeof(besked) - 1, 0);

    if (antal > 0 && std::string(besked) == "START")
    {
        // Her starter gemmeren spillet
        bool spilStartet = true;

        std::cout << "Spillet er startet!" << std::endl;

        // Bekræft til søgeren
        send(clientSocket, "OK", 2, 0);
    }

    close(clientSocket);
    close(serverSocket);
    return 0;
}


// Bemærk: Dette er et lille demonstrationseksempel. 
// TCP kan dele en besked over flere recv()-kald, 
// så den endelige version skal samle hele beskeden, før den behandles.