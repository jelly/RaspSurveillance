/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <abort@digitalise.net> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return J. Dijkstra (04/29/2010)
 * ----------------------------------------------------------------------------
 *  Modified by Adriano Maia (adriano@usk.bz)
 */
#include "nma.h"


static int nma_get_response_code(char* response);
static nma_connection* nma_ssl_connect();
static SOCKET nma_tcp_connect();
static char* nma_ssl_read(nma_connection* c);
static void nma_ssl_disconnect(nma_connection* c);
static char nma_int_to_hex(char code);
static char* nma_url_encode(char* str);

int nma_push_msg(char* api_key, int priority, char* application_name, char* event_name, char* description)
{
	nma_connection* c;
	char buffer[MESSAGESIZE];
	char* response = NULL;
#ifdef _WINDOWS
	static int wsa_init = 0;
#endif
	
	application_name = nma_url_encode(application_name);
	event_name = nma_url_encode(event_name);
	description = nma_url_encode(description);
	
#ifdef _WINDOWS
	if (wsa_init == 0)
	{
		WSAData wsad;
		
		if (WSAStartup(MAKEWORD(2, 2), &wsad) != 0)
		{
			fprintf(stderr, "Failed to initialize winsock (%d)\n", GetLastError());
			goto end;
		}
		
#ifdef _DEBUG
		printf("NotifyMyAndroid [debug]: Initialized Winsock\n");
#endif
		
		wsa_init = 1;
	}
#endif
	
	
	if ((c = nma_ssl_connect()) == NULL) goto end;
	
#ifdef _DEBUG
	printf("NotifyMyAndroid [debug]: Connected\n");
#endif
	
	sprintf(buffer, "GET /publicapi/notify?apikey=%s&priority=%d&application=%s&event=%s&description=%s\r\nHost: %s\r\n\r\n",
			api_key, priority, application_name, event_name, description, HOSTNAME);
	if (SSL_write(c->ssl_handle, buffer, strlen(buffer)) <= 0)
	{
		fprintf(stderr, "Failed to write buffer to SSL connection\n");
		ERR_print_errors_fp(stderr);
		goto end;
	}
	
#ifdef _DEBUG
	printf("NotifyMyAndroid [debug]: Written buffer: %s\n", buffer);
#endif
	
	response = nma_ssl_read(c);
	
#ifdef _DEBUG
	OutputDebugString("NotifyMyAndroid [debug]: server response:");
	if (response != NULL) printf("%s\n", response);
#endif
	
	nma_ssl_disconnect(c);
	
end:
	free(application_name);
	free(event_name);
	free(description);
	
	return (response == NULL ? -1 : nma_get_response_code(response));
}

static int nma_get_response_code(char* response)
{
	int code;
	char* start = strstr(response, "<nma>");
	
	/* invalid response */
	if (start == NULL) return -1; 
	
	start = strstr(start, "code");
	
	/* invalid response */
	if (start == NULL) return -1;
	
	/* failed to parse code */
	if (sscanf(start, "code=\"%d\"", &code) != 1) return -1;
	
	free(response);
	
	return code;
}

static nma_connection* nma_ssl_connect()
{
	nma_connection* c = (nma_connection*)malloc(sizeof(nma_connection));
	
	c->socket = nma_tcp_connect();
	if (c->socket != SOCKET_ERROR)
	{
		SSL_library_init();
		SSL_load_error_strings();
		
		c->ssl_context = SSL_CTX_new(SSLv23_client_method());
		if (c->ssl_context == NULL) ERR_print_errors_fp(stderr);
		
		c->ssl_handle = SSL_new(c->ssl_context);
		if (c->ssl_handle == NULL) ERR_print_errors_fp(stderr);
		
		SSL_CTX_set_verify(c->ssl_context, SSL_VERIFY_PEER, SSL_VERIFY_NONE);
		
		if (!SSL_set_fd(c->ssl_handle, c->socket)) ERR_print_errors_fp(stderr);
		
		if (SSL_connect(c->ssl_handle) != 1) ERR_print_errors_fp(stderr);
		
#ifdef _DEBUG
		printf("NotifyMyAndroid [debug]: SSL Handshake successful\n");
#endif
	}
	else
	{
#ifdef _WINDOWS
		fprintf(stderr, "Failed to retrieve a valid connected socket\n");
		return NULL;
#else
		perror("NotifyMyAndroid: Failed to retrieve connected socket");
#endif
	}
	
	return c;
}

static SOCKET nma_tcp_connect()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in server;
	struct hostent* host = gethostbyname(HOSTNAME);
	
	if (s == SOCKET_ERROR)
	{
#ifdef _WINDOWS
		fprintf(stderr, "Could not create socket (%d)\n", WSAGetLastError());
		return NULL;
#else
		perror("NotifyMyAndroid: Could not create socket");
#endif
	}
	
	if (host == NULL)
	{
#ifdef _WINDOWS		
		fprintf(stderr, "Could not retrieve host by name (%d)\n", WSAGetLastError());
		return NULL;
#else
		perror("NotifyMyAndroid: Could not retrieve host by name");
#endif
	}
	
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(SSL_PORT);
	server.sin_addr = *(struct in_addr*)host->h_addr;
	
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
#ifdef _WINDOWS
		fprintf(stderr, "NotifyMyAndroid connect error (%d)\n", WSAGetLastError());
		return NULL;
#else
		perror("NotifyMyAndroid: connect error");
#endif
	}
	
	return s;
}

static char* nma_ssl_read(nma_connection* c)
{
	int r = 1;
	char buffer[BUFFERSIZE];
	int size = BUFFERSIZE + 1;
	char* retval = (char*)malloc(size);
	
	memset(retval, 0, size);
	
	while (r > 0)
	{
		r = SSL_read(c->ssl_handle, buffer, BUFFERSIZE);
		if (r > 0)
		{
			buffer[r] = '\0';
			
			retval = realloc(retval, size + r);
			strcat(retval, buffer);
		}
	}
	
	return retval;
}

static void nma_ssl_disconnect(nma_connection* c)
{
	if (c->socket) closesocket(c->socket);
	if (c->ssl_handle)
	{
		SSL_shutdown(c->ssl_handle);
		SSL_free(c->ssl_handle);
	}
	if (c->ssl_context) SSL_CTX_free(c->ssl_context);
	
	free(c);
}

static char nma_int_to_hex(char code) 
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

static char* nma_url_encode(char* str) 
{
	char* pstr = str;
	char* buf = (char*)malloc(strlen(str) * 3 + 1);
	char* pbuf = buf;
	
	while (*pstr)
	{
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
		{
			*pbuf++ = *pstr;
		}
		else
		{
			*pbuf++ = '%';
			*pbuf++ = nma_int_to_hex(*pstr >> 4);
			*pbuf++ = nma_int_to_hex(*pstr & 15);
		}
		
		pstr++;
	}
	*pbuf = '\0';
	
	return buf;
}
