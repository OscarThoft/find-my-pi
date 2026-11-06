
#include <iostream>       // Bruges til input og output (cout og cerr)
#include <string>         // Gør det muligt at arbejde med strings
#include <cstring>        // Indeholder funktioner til håndtering af data
#include <thread>         // Gør det muligt at arbejde med tråde og sleep
#include <chrono>         // Bruges til at håndtere tid og tidsintervaller
#include <cstdlib>        // Indeholder generelle hjælpefunktioner
#include <unistd.h>       // Indeholder close() til at lukke sockets
#include <arpa/inet.h>    // Indeholder funktioner til IP-adresser og porte
#include <sys/socket.h>   // Indeholder funktioner til TCP sockets
#include "config.h"       // Henter vores fælles konfiguration, fx serverport

int main(int argc, char* argv[]) {

    // Kontrollerer, at brugeren har angivet serverens IP-adresse.
    // argc fortæller, hvor mange argumenter programmet er startet med.
    // argv indeholder selve argumenterne.
    // Vi forventer ét argument: serverens IP-adresse.
    if (argc != 2) {
        std::cerr << "Usage: ./client <server-ip>\n";
        return 1;
    }

    // Opretter en TCP-socket.
    // AF_INET betyder IPv4.
    // SOCK_STREAM betyder TCP-forbindelse.
    // 0 vælger automatisk den korrekte TCP-protokol.
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Kontrollerer, om socketen blev oprettet korrekt.
    if (clientSocket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Opretter og konfigurerer serverens netværksadresse.
    sockaddr_in serverAddress{};

    // Angiver, at vi bruger IPv4.
    serverAddress.sin_family = AF_INET;

    // Angiver serverens portnummer.
    // Porten hentes fra vores fælles config.h.
    // htons() konverterer portnummeret til network byte order.
    serverAddress.sin_port = htons(Config::SERVER_PORT);

    // Konverterer serverens IP-adresse fra tekstformat
    // til det binære format, som socket-funktionerne bruger.
    // argv[1] indeholder IP-adressen fra terminalen.
    if (inet_pton(AF_INET, argv[1],
                  &serverAddress.sin_addr) <= 0) {

        std::cerr << "Invalid server IP address\n";

        // Lukker socketen, hvis IP-adressen er ugyldig.
        close(clientSocket);
        return 1;
    }

    // Forsøger at oprette forbindelse til serveren.
    // Vi bruger serverens IP-adresse og portnummer.
    // connect() opretter TCP-forbindelsen mellem client og server.
    if (connect(clientSocket,
                (sockaddr*)&serverAddress,
                sizeof(serverAddress)) < 0) {

        std::cerr << "Connection failed\n";

        // Lukker socketen, hvis forbindelsen fejler.
        close(clientSocket);
        return 1;
    }

    // Forbindelsen til serveren er nu oprettet.
    std::cout << "[CLIENT] Connected to server!\n";

    // Simuleret afstand til gemmeren.
    // Senere erstatter vi denne værdi med rigtige målinger
    // fra vores UWB-modul (DWM1000).
    double distance = 20.0;

    // Fortsætter med at sende afstande, så længe afstanden
    // er større end eller lig med 0 meter.
    while (distance >= 0.0) {

        // Opretter en besked med afstandsværdien.
        // Beskeden starter med DISTANCE, efterfulgt af afstanden.
        // \n markerer slutningen på beskeden.
        std::string message =
            "DISTANCE " + std::to_string(distance) + "\n";

        // Sender beskeden til serveren via TCP.
        // clientSocket identificerer forbindelsen.
        // message.c_str() giver adgang til beskedens data.
        // message.size() angiver antallet af bytes.
        // MSG_NOSIGNAL forhindrer programmet i at få SIGPIPE,
        // hvis serveren har lukket forbindelsen.
        ssize_t sent = send(
            clientSocket,
            message.c_str(),
            message.size(),
            MSG_NOSIGNAL
        );

        // Kontrollerer, om afsendelsen fejlede.
        if (sent < 0) {
            std::cerr << "[CLIENT] Send failed\n";
            break;
        }

        // Udskriver den besked, som clienten har sendt.
        std::cout << "[CLIENT] Sent: "
                  << message;

        // Reducerer den simulerede afstand med 0,2 meter.
        // Dette får det til at se ud, som om søgeren
        // bevæger sig tættere på gemmeren.
        distance -= 0.2;

        // Sætter programmet på pause i 200 millisekunder.
        // 200 ms svarer til 5 opdateringer i sekundet (5 Hz).
        // Senere bruger vi samme interval til UWB-målingerne.
        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                Config::UPDATE_INTERVAL_MS
            )
        );
    }

    // Lukker TCP-forbindelsen til serveren,
    // når clienten er færdig med at sende data.
    close(clientSocket);

    // Udskriver, at clienten er blevet afbrudt.
    std::cout << "[CLIENT] Disconnected\n";

    return 0;
}
