#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http.h"

#define API_KEY ""

#define WOLFRAM "api.wolframalpha.com"
#define BASE_QUERY "v2/query?input=&appid=&format=plaintext"


void printWolframOutput(char *data);

int main(int argc, char *argv[]){


	if(argc <= 1){
		printf("WolfFram Alpha!\n");
		return 1;
	}

	int i;
	int strsize = 0;
	char *wolfram_func;
	

	for(i = 1; i < argc; i++){
		strsize += strlen(argv[i]);
		
		if(i < argc-1){
			strsize += 3;
		}
	}
	
	
	wolfram_func = malloc(strsize);
	wolfram_func[0] = '\0';
	
	for(i = 1; i < argc; i++){
		strcat(wolfram_func, argv[i]);
		
		if(i < argc-1){
			strcat(wolfram_func,"%20");
		}
	}
	

	char *encoded_func = urlEncode( wolfram_func );	
	free(wolfram_func);

	int base_length = sizeof(BASE_QUERY);
	int key_length = sizeof(API_KEY);
		
	char *query = (char *)malloc((base_length + key_length + strlen(encoded_func)+1)*sizeof(char));
	
	if(sprintf( query, "v2/query?input=%s&appid=%s&format=plaintext", encoded_func, API_KEY ) < 0){
		perror("Error formatting query\n");
		return 0;
	}	
	
	
	openHttpSocket();
	char *content = httpGetH(WOLFRAM, query,"User-Agent: HTMLGET 1.0");
	closeHttpSocket();

//	printf(query);
//	printf(content);	
	printWolframOutput(content);


	free(encoded_func);
	free(query);
	free(content);


	return 0;
}



void printWolframOutput(char *data){


	int length = strlen(data);
	int inside = 0;
	int diff = 0;
	int isempty = 1;
	int j = 0;
	int buflen = 0;
	int bufmax = 256;
	int lastSpace = 0;

	char *cbuffer = (char *)malloc(bufmax);
	char *title = (char *)malloc(5);
	char *xml = (char *)malloc(16);

	int tlen = 0;
	int hastitle = 0;	

	int plen = 0;
	int getPodTitle = 0;
	int ignorePod = 0;

	for(j = 0; j < length; j++){
						
	// Print everything outside <> and title	

		char c = data[j];

		if(hastitle != 0 && c == '\''){
			hastitle++;
			
			if(hastitle == 2){
				cbuffer[buflen++] = '\x1B';
				cbuffer[buflen++] = '[';
				cbuffer[buflen++] = '3';
				cbuffer[buflen++] = '6';
				cbuffer[buflen++] = 'm';
			}else if(hastitle == 3){
				cbuffer[buflen++] = '\033';
				cbuffer[buflen++] = '[';
				cbuffer[buflen++] = '0';
				cbuffer[buflen++] = 'm';
			}
			
			continue;
		}		

		if(getPodTitle == 1 && plen < 16){
		
			if(c != ' '){
				xml[plen++] = c;
			}else{
				if(strncmp(xml,"link",plen) == 0){
					ignorePod = 1;
				}
				
				getPodTitle = 0;
			}
				
		}


		if( c == '<'){
			
			if(diff > 0 && isempty == 0){
				cbuffer[buflen++] = '\n';
			}
			
			isempty = 1;
			inside = 1;
			diff = 0;
			tlen = 0;
			hastitle = 0;
			plen = 0;
			getPodTitle = 1;	
			ignorePod = 0;

			continue;

		}else if(inside == 1 && c == '>'){

			inside = 0;
			continue;
		}
		
		if((inside == 0 || hastitle == 2)  && c != '\n'){

			if(lastSpace == 0 || c !=' ' && c != '\t'){
				cbuffer[buflen++] = c;
				diff++;
			}

			if(c!=' ' && c != '\t'){
				lastSpace = 0;
				isempty = 0;
			}else{
				lastSpace = 1;
			}
		}else if(inside == 1 && hastitle == 0 && ignorePod == 0){

			if(c == ' '){
				tlen = 0;	
			}else if( tlen < 5){
				
				title[tlen++] = c;
				if(strncmp(title, "title",5) == 0){
					hastitle = 1;
					tlen = 0;
					title[0] = '\0';
				}
			}			
		}
		
		

		// Resize buffer
		if(buflen >= bufmax - 8){
			bufmax = bufmax*2;			
			char *nbuffer =(char *)malloc(bufmax);
			memcpy(&nbuffer[0], &cbuffer[0],( buflen )* sizeof(char));
			free(cbuffer);
			cbuffer = (char *)nbuffer;
		}
	}

	printf("buflen=%d\n\n", buflen);
	printf(cbuffer);


	free(cbuffer);
	free(title);
	free(xml);
}



