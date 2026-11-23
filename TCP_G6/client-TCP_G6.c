/*
Esonero Reti di Calcolatori a.a. 2026-27 - I Assegnazione di Laboratorio: socket TCP
Progettare ed implementare (scrivendo codice portabile) un’applicazione TCP client/server conforme al
seguente protocollo:
1) Il server è avviato su una porta a scelta (NB: Il server resta sempre in ascolto).
2) Il client legge da tastiera l’IP dell’host e il numero di porta da utilizzare per contattare il Server
3) Il client, richiede la connessione al server.
4) Stabilita la connessione, visualizza sullo std output un messaggio contenente l’IP dell’host del client con
cui ha stabilito la connessione. (Esempio: “connesso con il client il cui nome IP: xxxxx”)
5) Il server invia al client la stringa "connessione avvenuta".
6) Il client riceve la stringa e la stampa.
7) Il client legge una dallo std input tre interi n1 n2 ed n3, e li invia al server
8) Il server riceve i due numeri e visualizza sullo std output “la somma di n1+n2-n3=res”, dove res è il
risultato;
9) Se res = 100, il server invia al client la stringa “QUIT”, altrimenti invia il risultato res
10) Il client riceve la stringa dal server. La visualizza sullo STD output. Se la stringa corrisponde a “QUIT”
il client chiude la connessione e successivamente termina il processo. Altrimenti per il client stampa il
risultato res e torna al passo 7;
11) Il server rimane in ascolto. 

*/

#if defined _WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 512

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

int main() {
#if defined _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Error at WSAStartup()\n");
        return -1;
    }
#endif

    // CREAZIONE DELLA SOCKET
    int Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Csocket < 0) {
        printf("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    // CONNECT AL SERVER
    int port;
    char IP [16];
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;

    //CHIEDO DA TASTIERA DOVE COLLEGARMI
    // Lettura e validazione IP e Porta
    do {
        printf("Indica l'IP di dove vuoi collegarti: ");
        scanf("%15s", IP);
    } while (inet_addr(IP) == INADDR_NONE); // Ripete se l'IP non è valido (es. 256.x.x.x)

    sad.sin_addr.s_addr = inet_addr(IP);

    do {
        printf("Indica la porta di dove vuoi collegarti: ");
        scanf("%d", &port); 
    } while (port < 1 || port > 65535);

    sad.sin_port = htons(port);

    // CONNESSIONE AL SERVER
    if (connect(Csocket, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        printf("Connessione fallita.\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }
    printf("Connesso al server!\n");


    // RICEVERE DATI DAL SERVER (stringa "connessione avvenuta" )
    int bytesRcvd;
    int totalBytesRcvd = 0;
    char buf[BUFFERSIZE];
    printf("Messaggio ricevuto: ");
    bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
    if (bytesRcvd <= 0) {
        printf("\n Ricezione fallita.\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }
    totalBytesRcvd += bytesRcvd;
    buf[bytesRcvd] = '\0';
    printf("%s", buf);
    printf("\n");

    // CICLO INVIO 3 INTERI E RICEZIONE RISPOSTA 
    int n1, n2, n3;
    int numeri[3];

    while (1) {
        // Legge dallo std input tre interi n1 n2 ed n3, e li invia al server
        printf("Inserisci tre interi (n1 n2 n3): ");
        scanf("%d %d %d", &n1, &n2, &n3);

        // Converto in network byte order per mantenere la portabilità
        numeri[0] = htonl(n1);
        numeri[1] = htonl(n2);
        numeri[2] = htonl(n3);

        if (send(Csocket, (char*)numeri, sizeof(numeri), 0) != sizeof(numeri)) {
            printf("Invio dati fallito.\n");
            closesocket(Csocket);
            ClearWinSock();
            return -1;
        }

        //  Il client riceve la stringa dal server e la visualizza sullo STD output
        bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0);
        if (bytesRcvd <= 0) {
            printf("Ricezione risposta fallita.\n");
            closesocket(Csocket);
            ClearWinSock();
            return -1;
        }
        buf[bytesRcvd] = '\0';
        printf("Risposta dal server: %s\n", buf);

        // Se la stringa corrisponde a "QUIT" esce dal ciclo per chiudere e terminare
        if (strcmp(buf, "QUIT") == 0) {
            break;
        }
    }

    // CHIUSURA DELLA CONNESSIONE
    closesocket(Csocket);
    ClearWinSock();
    return 0;
}