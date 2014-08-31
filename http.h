




int openHttpSocket();


int closeHttpSocket();


char *getIP(char *host);


char *build_get_query(char *host, char *page);


char *stringAppend( char *a, char *b);

int addToChar(char *in, char *add, int len);

char *httpGet( char *host, char *page);

char *urlEncode( char *in );
