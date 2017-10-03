#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <arpa/inet.h>
#include <time.h>

//struct for request
struct request
{
    gchar* type;
    gchar* URI;
    gchar* version;
    gchar* body;
    in_addr_t ip;
    int port;
    //Todo: add headers
    //reaserch glist (google)
    GList* headers;
	
};

//struct for headers
struct header
{
    gchar* key;
    gchar* value;
};

//reading the request
struct request read_request(gchar* message)
{
    gchar **request_lines = g_strsplit(message,"\n",0);
    gchar **request_string = g_strsplit(request_lines[0]," ", 3);

    struct request new_request;

    new_request.type = request_string[0];
    new_request.URI = request_string[1];
    new_request.version = request_string[2];

    struct header new_header;
    int b = 0;
    //forlykkju í gengum rest af línum
    for(size_t i = 1; i < g_strv_length(request_lines); i++ )
    {
	gchar **request_header = g_strsplit(request_lines[i], ":", 0);
	new_header.key = request_header[0];
	new_header.value = request_header[1];
	//appenda fyrst key og svo : og svo value
	//g_list_append(new_request.headers, new_header);
	b++;
    }
    //g_list_append(new_request.headers, new_header
    //read headers, create a new function that reads the headers
    //Todo: read body if Post request
    if(g_strcmp0(new_request.type, "POST") == 0)
    {
	// ef á eftir headerum kemur ein tóm lína
	// og svo texti þá er það bodyið	
    }
    return new_request;
}

//processing the request
GString* process_request(struct request my_request)
{
    GString* response = g_string_new(NULL);
    
    if(g_strcmp0(my_request.type, "GET") == 0)
    {
	//my_request.headers, loopa í gegn
	g_string_append_printf(response,"HTTP/1.1 %d %s \n", 200, "OK");
	printf("Response after append1: %s \n", response->str);
	//add headers to response 
	response = g_string_append(response, "\n");
	char* ip = inet_ntoa(*(struct in_addr*)&my_request.ip);
	g_string_append_printf(response,"%s %s:%d \n", my_request.URI, ip, my_request.port);
    }
    
    if(g_strcmp0(my_request.type, "HEAD") == 0)
    {
	//blabla
    }
   
    if(g_strcmp0(my_request.type, "POST") == 0)
    {
	//blabla
    
    }
    return response;
} 

int main(/*int argc, char *argv[]*/)
{
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];
    //int port = strol(argv[1]
    // Create and bind a TCP socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Network functions need arguments in network byte order instead of
    // host byte order. The macros htonl, htons convert the values.
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(32000);
    bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    // Before the server can accept messages, it has to listen to the
    // welcome port. A backlog of one connection is allowed.
    listen(sockfd, 1);

    for (;;) {
        // We first have to accept a TCP connection, connfd is a fresh
        // handle dedicated to this connection.
        socklen_t len = (socklen_t) sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr *) &client, &len);

        // Receive from connfd, not sockfd.
        ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
	
	//Timestamp
	time_t the_time;
	the_time = time(NULL);	

	printf("Recived message\n");
	
        message[n] = '\0';
        fprintf(stdout, "Received:%s\n", message);

	struct request first_request = read_request(message);
	first_request.ip = client.sin_addr.s_addr;
	first_request.port = htons(client.sin_port);
	char* ip = inet_ntoa(*(struct in_addr*)&client.sin_addr.s_addr);
	printf("Log request: ");
	printf("%s : %s:%d %s %s : %d\n", asctime(localtime(&the_time)) , ip, first_request.port, first_request.type, first_request.URI, 200);
	//Todo: call read_request
	//log request timestamp : <client ip>:<client port> <request method>
	//            <requested URL> : <response code>
	//Process request , create process function
	GString* response = process_request(first_request);
	//Send response back, body of responce is on the form 
	//http://foo.com/page 123.123.123.123:45678
        // Send the message back.
	//gchar* r = g_string_free(response, FALSE);
	printf("Response: %s \n",response->str);	
        send(connfd, response->str,100, 0);

        // Close the connection.
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }
}
