#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>






#define HOST "google.com"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"


int sock;


int openHttpSocket(){

	
	if(( sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("HTTPLIB: Can't create TCP socket\n");
		return -1;
	}
	
	return sock;
}

int closeHttpSocket(){

	close(sock);
	return 1;
}


char *getIP(char *host){

	struct hostent *hent;
	int iplen = 15;
	
	char *ip = (char *)malloc(iplen+1);
	memset(ip, 0, iplen+1);

	if(( hent = gethostbyname(host)) == NULL){
		herror("HTTPLIB: Can't get IP\n");
		exit(1);
	}

	if(inet_ntop( AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL){
		perror("HTTPLIB: Can't Resolve host\n");
		exit(1);
	}

	return ip;
}


char *build_get_query(char *host, char *page){

	char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";

	if( getpage[0] == '/' ){
		getpage = getpage +1;
		//fprintf(stderr, "Removing leading \"/\", converting %s to %s\n", page, getpage);
	}


	query = (char *)malloc(strlen(host) + strlen(getpage) + strlen(USERAGENT) + strlen(tpl)-5);

	sprintf(query, tpl, getpage, host, USERAGENT);

	return query;
}



char *stringAppend( char *a, char *b){

	char *result;

	if((result = malloc(strlen(a)+strlen(b)+1)) != NULL){
		result[0] = '\0';
		strcat(result, a);
		strcat(result, b);
	}else{
		perror("HTTPLIB: Error in stringAppend\n");
	}	

	return result;
}



char *httpGet( char *host, char *page ){


	struct sockaddr_in *remote;	
	int tmpres;
	char *ip;
	char *get;
	char buf[BUFSIZ+1];

	ip = getIP(host);

	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton( AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));

	if( tmpres < 0){
		perror("HTTPLIB: Can't set remote addr\n");
		exit(1);	
	}else if(tmpres == 0){
		fprintf(stderr, "HTTPLIB:  %s is not a valid IP Address\n", ip);
		exit(1);
	}

	remote->sin_port = htons(PORT);
	
	if( connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
		perror("HTTPLIB: Could not connect\n");
		exit(1);
	}

	get = build_get_query(host, page);

	// Send package
	int sent = 0;

	while(sent < strlen(get)){
	
		tmpres = send(sock, get+sent, strlen(get)-sent, 0);
		
		if(tmpres == -1){
			perror("HTTPLIB: Can't send query\n");
			exit(1);
		}
		
		sent += tmpres;
	}


	// Recieve package

	memset(buf, 0, sizeof(buf));
	int httpstart = 0;
	char *httpcontent;
	char *httpresult;

	while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
		
		if(httpstart == 0){

			httpcontent = strstr(buf, "\r\n\r\n");
			if(httpcontent != NULL){
				httpstart = 1;
				httpcontent += 4;
			}

		}else{
			httpcontent = buf;
		}

		if( httpstart ){
//			fprintf(stdout, httpcontent);
			httpresult = stringAppend(httpresult, httpcontent);			
		}
		
		memset(buf, 0, tmpres);
	}	
	

	free(get);
	free(remote);
	free(ip);

	return httpresult;
}




char *urlEncode( char *in ){



	int length = strlen(in);
	int j;

	char *result = (char *)malloc( 3*length * sizeof(char) );
	int len = 0;
	

	for(j = 0; j < length; j++){

		char c = in[j];
		
		switch(c){

		case ' ':
			len += addToChar(result, "%20", len);
		break;	
		case '!':
			len += addToChar(result, "%21", len);	
		break;
		case '#':
			len += addToChar(result, "%23", len);
		break;
		case '$':
			len += addToChar(result, "%24", len);
		break;
		case '&':
			len += addToChar(result, "%26", len);
		break;
		case '\'':
			len += addToChar(result, "%27", len);
		break;
		case '(':
			len += addToChar(result, "%28", len);
		break;
		case ')':
			len += addToChar(result, "%29", len);
		break;
		case '*':
			len += addToChar(result, "%2A", len);
		break;
		case '+':
			len += addToChar(result, "%2B", len);
		break;
		case ',':
			len += addToChar(result, "%2C", len);
		break;
		case '/':
			len += addToChar(result, "%2F", len);
		break;
		case ':':
			len += addToChar(result, "%3A", len);
		break;
		case ';':
			len += addToChar(result, "%3B", len);
		break;
		case '=':
			len += addToChar(result, "%3D", len);
		break;
		case '?':
			len += addToChar(result, "%3F", len);
		break;
		case '@':
			len += addToChar(result, "%40", len);
		break;
		case '[':
			len += addToChar(result, "%5B", len);
		break;
		case ']':
			len += addToChar(result, "%5D", len);
		break;
		default:
			result[len++] = c;
			
		}
	}
	
	char *encoded = (char *)malloc( len * sizeof(char) );
	encoded[0] = '\0';	
	
	strncpy(encoded, result, len);
	free(result);

	return encoded;
}


int addToChar(char *in, char *add, int len){
	
	int i;
	int length = strlen(add);

	for(i = 0; i < length; i++){

		in[len++] = add[i];
	}

	return length;
}


