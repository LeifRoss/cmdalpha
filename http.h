#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>


/*
	HTTP Library 
	Version 0.01
	Author - Leif Andreas Rudlang

*/


#ifndef HTTP_H
#define HTTP_H



// Open socket, init HTTP
int openHttpSocket();

// Close socket, free HTTP
int closeHttpSocket();

// Retrieve IP of remote host
char *getIP(char *host);


// Perform a complete HTTP GET request
char *httpGetH( char *host, char *page, char *headers);


char *httpGet( char *host, char *page);


// Performa a complete HTTP POST request
char *httpPostH(char *host, char *subpage, char *headers, char *data);

char *httpPost(char *host, char *subpage, char *data);


// URL encode a string
char *urlEncode( char *in );

char *xurlEncode( char *in );

char *xformEncode( char *in[], int params);


// Connect the HTTP socket to the remote host
int connectToHost( char *host );


// Send the http packet
void sendHttpPacket( int tmpres, char* packet );

// Recieve the http packet
char *recieveHttp();


// UTILITY METHODS //

// String append utility method
char *stringAppend( char *a, char *b);

// Add to char utility method
int addToChar(char *in, char *add, int len);

#endif


