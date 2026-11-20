#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    // 1. Opret en TCP-socket til IPv4
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (clientSocket < 0) {
        perror("socket");
        return 1;
    }

    // her oprettes socket,
    //  AF_INET=vi bruger alm IPv4 adresser (som 10.0.0.1)
	// SOCK_STREAM = vi vil have en stream forbindelse aka en TCP. 
    // 0 til sidst er bare "lad styresystemet vaelge den rigtige protokol"
	
    // returvaerdien gemmes i clientSocket
    // Det er en socket file descriptor: et heltal, som bruges
    // til at referere til denne socket i resten af programmet.


    // 2. Angiv den server, vi vil forbinde til
    sockaddr_in serverAddress{};  // opretter/nulstiller struct som holder serverens adresse
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9000);

    if (inet_pton(AF_INET, "172.16.15.2",
                  &serverAddress.sin_addr) != 1) {
        std::cerr << "Ugyldig IP-adresse\n";
        close(clientSocket);
        return 1;
    }
    // omndanner IP-adressen fra tekst til binær form, som socketen kan bruge.

    // 3. Opret forbindelse til serveren
    if (connect(clientSocket, (sockaddr*)&serverAddress,
                sizeof(serverAddress)) < 0) {
        perror("connect");
        close(clientSocket);
        return 1;
    }

    std::cout << "Forbundet til serveren!" << std::endl;

    // HER kan vi senere sende og modtage via clientSocket

     // eksemple:
    
    // Send besked om at starte spillet

}