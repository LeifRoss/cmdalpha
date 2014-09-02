#include "http.h"


#define HOST "localhost"
#define PAGE "/"
#define PORT 80
#define USERAGENT "Mozilla/4.0"


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


char *httpGetH( char *host, char *page, char *headers ){

	int tmpres = connectToHost( host );

	
	char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.1\r\nHost: %s\r\n%s\r\n\r\n";

	if( getpage[0] == '/' ){
		getpage = getpage +1;
	}

	query = (char *)malloc(strlen(host) + strlen(getpage) + strlen(headers) + strlen(tpl)-5);
	sprintf(query, tpl, getpage, host, headers);

	sendHttpPacket(tmpres, query);

	char *httpresult = recieveHttp();

	free(query);

	return httpresult;
}



char *httpGet( char *host, char *page ){

	char *tpl = "User-Agent: %s\r\n\r\n";
	char *header = (char *)malloc( (strlen(tpl) + strlen(USERAGENT))*sizeof(char) );
	sprintf(header, tpl, USERAGENT);


	char *httpresult = httpGetH( host, page, header );
	
	free(header);
	
	return httpresult;
}



char *httpPostH(char *host, char *subpage, char *headers, char *data ){

	int tmpres = connectToHost(host);

	if( subpage[0] == '/'){
		subpage++;
	}

	
	
	char *tpl = "POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\n%s\r\n\r\n%s";
	
	int contentLength = strlen(data) * sizeof(char);
	char *query = (char *)malloc(( strlen(tpl) + strlen(headers) + strlen(host) + strlen(subpage) + strlen(data) - 5) * sizeof(char));
	sprintf(query, tpl, subpage, host, contentLength, headers, data);

	sendHttpPacket(tmpres, query);	

	char *httpresult = recieveHttp();
	
	free(query);	
	free(tpl);

	return httpresult;
}




char *httpPost(char *host, char *subpage, char *data ){

	char* tpl = "User-agent: %s\r\nContent-Type: application/x-www-form-urlencoded";
	char* header = (char *)malloc( (strlen(tpl) + strlen(USERAGENT)) * sizeof(char) );
	
	sprintf(header, tpl, USERAGENT);
	
	char *httpresult = httpPostH(host, subpage, header, data);

	free(tpl);
	free(header);

	return httpresult;
}


int connectToHost( char* host ){


	struct sockaddr_in *remote;	
	int tmpres;
	char *ip;

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

	free(remote);
	free(ip);

	return tmpres;
}


void sendHttpPacket(int tmpres, char *packet){


	// Send package
	int sent = 0;

	while(sent < strlen(packet)){
	
		tmpres = send(sock, packet+sent, strlen(packet)-sent, 0);
		
		if(tmpres == -1){
			perror("HTTPLIB: Can't send query\n");
			exit(1);
		}
		
		sent += tmpres;
	}

}


char* recieveHttp(){
	
	char buf[BUFSIZ+1];

	memset(buf, 0, sizeof(buf));
	int httpstart = 0;
	char *httpcontent;
	int len = 0;
	int tmpres = 0;

	//allocate

	int allocated = 512;
	char *httpresult = (char *)malloc(allocated);


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
			
			int contentlen = strlen(httpcontent);
			int nlen = len + contentlen;
						
			if(nlen >= allocated){	
				allocated = (nlen + len) * sizeof(char);
				char *nbuffer = (char *)malloc(allocated);
				memcpy(&nbuffer[0], &httpresult[0], (len)*sizeof(char));
				free(httpresult);
				httpresult = (char *)nbuffer;
			}			

			memcpy( &httpresult[len], &httpcontent[0], contentlen*sizeof(char));
			len = nlen;	
		}
		
		memset(buf, 0, tmpres);
	}	
	
	
	char *queryresult = (char *)malloc(len*sizeof(char));
	memcpy(&queryresult[0], &httpresult[0], (len)*sizeof(char));
	
	free(httpresult);


	return queryresult;
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


char *xurlEncode( char *in ){



	int length = strlen(in);
	int j;

	char *result = (char *)malloc( 3*length * sizeof(char) );
	int len = 0;
	

	for(j = 0; j < length; j++){

		char c = in[j];
		
		switch(c){

		case ' ':
			len += addToChar(result, "+", len);
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


char *xformEncode( char *in[], int params){

	int capacity = 128;
	char *buffer = (char *)malloc(capacity);
	int len = 0;

	int j;
	int elements = params*2;
	for(j = 0; j < elements; j += 2){

		char *key = in[j];
		char *val = in[j+1];
	
		char *keyEncoded = xurlEncode(key);
		char *valEncoded = xurlEncode(val);		
		
		int nlen = len + (strlen(keyEncoded) + strlen(valEncoded) + 4) * sizeof(char);
		
		if(nlen >= capacity) {
			capacity = nlen + capacity;
			char *nbuffer = (char *)malloc(capacity);
			memcpy(nbuffer, buffer, len);
			free(buffer);
			buffer = nbuffer;	
		}
		
		len += addToChar(buffer, keyEncoded, len);
		buffer[len++] = '=';
		len += addToChar(buffer, valEncoded, len);
		
		if(j < elements-1){
			buffer[len++] = '&';
		}

	}

	char *encoded = (char *)malloc(len*sizeof(char));
	strncpy(encoded, buffer, len);
	
	free(buffer);

	return encoded;
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





int addToChar(char *in, char *add, int len){
	
	int i;
	int length = strlen(add);

	for(i = 0; i < length; i++){

		in[len++] = add[i];
	}

	return length;
}


