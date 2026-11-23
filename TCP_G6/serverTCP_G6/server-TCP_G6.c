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

#if defined _WIN32     //Ramo valido e utilizzabile solo per windows
#include <winsock.h> 
#else
#define closesocket close  // Ramo utilizzabile su Unix
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 5194
#define QLEN 6
#define BUFFERSIZE 512

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

int main ()
{
   //Per Windows
   #if defined _WIN32  //perchè solo per windows va inizializzata la socket 

   //Inizializzazione 
   //Struct con i campi versione usata e la massima utilizzabile
   WSADATA wsadata;
   int iResult=WSAStartup(MAKEWORD(2,2), &wsadata);

   //Verifico se 0 è stato accettato / altrimenti da un codice di errore
   if(iResult!=0)
   {
    printf("Errore della WSAStartup() \n");
    return -1;
   } 
   #endif


   //--- CREAZIONE DI UNA SOCKET ---
   int Mysocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

   if(Mysocket<0)  //Valuto eventuali errori in fase di creazione
   {
      printf("Creazione della socket fallita. \n");
      ClearWinSock();
      return -1;
   }

   //- Assegno solo al server un indirizzo alla socket
   struct sockaddr_in ind;
   memset(&ind, 0, sizeof(ind));
   ind.sin_family = AF_INET;
   ind.sin_addr.s_addr = htonl(INADDR_ANY); // Ascolta su tutte le interfacce di rete
   ind.sin_port = htons(PORT);

   //- Verifico la presenza di errori 
   if(bind(Mysocket, (struct sockaddr*) &ind, sizeof(ind))<0)
   {
      printf("Bind() fallita. \n");
      closesocket(Mysocket);
      ClearWinSock();
      return -1;
   }
   printf("bind() riuscita sulla porta %d\n", PORT);
   

   //--- MANDARE LA SOCKET IN ASCOLTO ---
    if (listen(Mysocket, QLEN) < 0) {
        printf("listen() failed.\n");
        closesocket(Mysocket);
        ClearWinSock();
        return -1;
    }
    printf("listen() riuscita: socket in ascolto (coda = %d)\n", QLEN);

    // ACCETTARE UNA NUOVA CONNESSIONE (si esce con Ctrl+C)
    struct sockaddr_in cad;
    int clientSocket;
    #if defined _WIN32
    int clientLen;
    #else
    socklen_t clientLen;
    #endif

    printf("Waiting for a client to connect...\n");
    while (1) {
        clientLen = sizeof(cad);
        if ((clientSocket = accept(Mysocket, (struct sockaddr*)&cad, &clientLen)) < 0) {
            printf("accept() failed.\n");
            closesocket(Mysocket);
            ClearWinSock();
            return -1;
        }
        // Stampa IP del client connesso
        printf("Connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));
        

        // INVIARE DATI AL CLIENT ("connessione avvenuta")
         char* inputString = "connessione avvenuta";
         int stringLen = strlen(inputString);
         if (send(clientSocket, inputString, stringLen, 0) != stringLen) 
         {
           printf("Invio fallito.\n");
           closesocket(clientSocket);
           continue;
         }
         printf("Inviati %d byte: %s\n", stringLen, inputString);

        // Ciclo di ricezione numeri e invio risultato o QUIT
        int numeri[3];
        char outBuf[BUFFERSIZE];

        while (1) {
            // Ricevo i 3 interi inviati dal client
            int bytesRcvd = recv(clientSocket, (char*)numeri, sizeof(numeri), 0);
            if (bytesRcvd <= 0) {
                printf("Client disconnesso o errore di ricezione.\n");
                break; // Esce dal ciclo interno e torna in ascolto (Punto 11)
            }

            // Converto da network byte order a host byte order per portabilità
            int n1 = ntohl(numeri[0]);
            int n2 = ntohl(numeri[1]);
            int n3 = ntohl(numeri[2]);
            int res = n1 + n2 - n3;

            // Visualizzo sullo std output la formula e il risultato
            printf("la somma di %d+%d-%d=%d\n", n1, n2, n3, res);

            // Se res = 100 preparo "QUIT", altrimenti preparo il risultato res
            if (res == 100) {
                strcpy(outBuf, "QUIT");
            } else {
                sprintf(outBuf, "%d", res);
            }

            // Invio la stringa al client
            int outLen = strlen(outBuf);
            if (send(clientSocket, outBuf, outLen, 0) != outLen) {
                printf("Invio risposta fallito.\n");
                break;
            }

            // Se ho inviato QUIT, il client chiuderà: chiudo anch'io questa connessione
            if (res == 100) {
                printf("Inviato QUIT. Chiusura connessione con il client.\n");
                break;
            }
        }

        //  Chiudo il socket del client corrente e resto in ascolto per il prossimo
        closesocket(clientSocket);
        printf("\nWaiting for a new client to connect...\n");
    }

    closesocket(Mysocket);
    ClearWinSock();
    return 0;
}