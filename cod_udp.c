/*
 *  === cod_udp.c ===
 *
    Legge/Scrive le variabili dichiarate in Codesys come Globals
    su UDP verso la memoria Condivisa EPI

    utilizza la libreria "libmemory.so"
*/

#include <stdio.h>      //printf
#include <string.h>     //memset
#include <stdlib.h>     //exit(0);
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


//typedef unsigned int DWORD;


// ----------------------------------------------------------
struct sockaddr_in sock_srv, sock_sender, sock_cli;
int udp_server = 0;
int udp_client = 0;
int ServerPort;
int ClientPort;


char UdpErrors[512];

//
// ======================================================================
//

// ------------------------------------
// Rende Buffer con eventuale Errore
//
char *GetUdpErrors()
{return UdpErrors;}

// ----------------------------------------
// Set della comunicazione Udp :
// - Utilizza 'Port' come porta del Server (ricezione)
// - il valore 'Port+1' come porta del Client
// - 'pCliIP' e' l'indirizzo del destinatario (trasmissione)
//
int SetCodUdp(int Port, char *pCliIP)
{
    int err = 0;
    // Clear segnalazione errori
    UdpErrors[0] = '\0';

    // -----------------------------------------------------------------
    // Crea un Server UDP
    //
    ServerPort = Port;
    if ((udp_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror(UdpErrors);
        return -1;
    }

    // zero out the structure
    memset((char *) &sock_srv, 0, sizeof(sock_srv));

    // set the structure
    sock_srv.sin_family = AF_INET;
    sock_srv.sin_port = htons(ServerPort);
    sock_srv.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(udp_server , (struct sockaddr*)&sock_srv, sizeof(sock_srv) ) == -1)
    {
        perror(UdpErrors);
        err = 1;
    }

    // -----------------------------------------------------------------
    // Crea un Client UDP
    //
    ClientPort = Port + 1;
    if ( (udp_client=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror(UdpErrors);
        err |= 2;
    }

    memset((char *) &sock_cli, 0, sizeof(sock_cli));
    sock_cli.sin_family = AF_INET;
    sock_cli.sin_port = htons(ClientPort);

    if (inet_aton(pCliIP , &sock_cli.sin_addr) == 0)
    {
        perror(UdpErrors);
        err |= 2;
    }

    return err;
}


// --------------------------------------
// Chiude socket Server o Client
// se 'srvflag' o 'cliflag' > 0
//
void CloseUdpComm(int srvflag, int cliflag)
{
    if  ((srvflag > 0) && (udp_server > 0))
        close(udp_server);
    if  ((cliflag > 0) && (udp_client > 0))
        close(udp_client);
}

//--------------------------------------------------
// try to receive some data, this is a blocking call
//
int UdpReceive(int BufLen, char *pRcvBuf)
{
    int slen = sizeof(sock_sender);
    int recv_len;

    /* Dal manuale :
    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                     struct sockaddr *src_addr, socklen_t *addrlen);
    */
    if ((recv_len = recvfrom(udp_server, pRcvBuf, BufLen, 0,
                             (struct sockaddr *) &sock_sender, (socklen_t *)&slen)) == -1)
    {
        perror(UdpErrors);
        return -1;
    }
    return recv_len;
}

//--------------------------------------------------
// try to send some data
//
int UdpSend(int BufLen, char *pSndBuf)
{
    int x;
    int slen = sizeof(sock_cli);
    x = sendto(udp_client, pSndBuf, BufLen, 0, (struct sockaddr*) &sock_cli, slen);
    if (x < 0)
    {
        perror(UdpErrors);
        return -1;
    }
    return BufLen;
}
