/*
 *  nma.h
 *  nma 
 *
 *  Created by J. Dijkstra (abort@digitalise.net) on 29-04-10.
 *  Copyright 2010 Digitalise.NET. All rights reserved.
 *  Modified by Adriano Maia (adriano@usk.bz) on 4/17/2011.
 *
 */

#ifndef NMA_H_
#define NMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#define STRICT

#define SSL_PORT 443
#define HOSTNAME "nma.usk.bz"
#define MESSAGESIZE 11400
#define BUFFERSIZE 512

/* priorities */
#define NMA_PRIORITY_VERY_LOW -2
#define NMA_PRIORITY_MODERATE -1
#define NMA_PRIORITY_NORMAL 0
#define NMA_PRIORITY_HIGH 1
#define NMA_PRIORITY_EMERGENCY 2

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ssleay32MT.lib")
#pragma comment(lib, "libeay32MT.lib")
#else
#define SOCKET int
#define SOCKET_ERROR -1
#define closesocket(socket) close(socket)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* openssl headers */
#include <openssl/ssl.h>
#include <openssl/err.h>

/* ssl/connection structure */
typedef struct {
    SOCKET socket;
    SSL* ssl_handle;
    SSL_CTX* ssl_context;
} nma_connection;

int nma_push_msg(char* api_key, int priority, char* application_name, char* event_name, char* description);

#ifdef __cplusplus
}
#endif
#endif
