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
	GString* body;
	in_addr_t ip;
	int port;
	GHashTable* headers;
};

//reading the request
struct request read_request(gchar* message)
{
	gchar **request_lines = g_strsplit(message,"\n",0);
	gchar **request_string = g_strsplit(request_lines[0]," ", 3);

	struct request new_request;
	
	// For the first line
	new_request.type = request_string[0];
	new_request.URI = request_string[1];
	new_request.version = request_string[2];

	new_request.headers = g_hash_table_new(g_str_hash, g_str_equal);
	int head_count = 0;
	// For the rest of the lines
	for(size_t i = 1; i < g_strv_length(request_lines); i++ )
	{
		printf("Parsing line: %s \n ", request_lines[i]);
		gchar **request_header = g_strsplit(request_lines[i], ":", 0);
		head_count++;
		if(g_strv_length(request_header) < 2)
		{
			break;
		}
		printf("Key: %s Value: %s \n",request_header[0], request_header[1]);
		g_hash_table_insert(new_request.headers, request_header[0], request_header[1]);
	}

	printf("After headers \n");
	if(g_strcmp0(new_request.type, "POST") == 0)
	{
		GString* body = g_string_new(NULL);
		for(size_t i = head_count + 1; i < g_strv_length(request_lines); i++)
		{
			g_string_append(body, request_lines[i]);
		}
		new_request.body = body;
	}
	return new_request;
}

//processing the request
GString* process_request(struct request my_request)
{
	GString* response = g_string_new(NULL);

	g_string_append_printf(response,"HTTP/1.1 %d %s \n", 200, "OK");

	GHashTableIter i;
	gpointer key, value;
	g_hash_table_iter_init(&i, my_request.headers);

	//Timestamp
	time_t the_time;
	the_time = time(NULL);

	while(g_hash_table_iter_next(&i, &key,&value))
	{
		printf("Key: %s Value: %s \n", (char*)key, (char*)value);
		g_string_append_printf(response, "%s: %s \n", (char*)key,(char*)value);
	}
	
	g_string_append_printf(response, "%s: %s \n", "Last-Modified", asctime(localtime(&the_time))); 
	
	// For the content length	
	int length; 
	length  = strlen(response->str);
 
	g_string_append_printf(response, "%s: %d \n", "Content-Length", length);
	g_string_append_printf(response, "%s: %s \n", "Content-Type", "text/plain");
	g_string_append(response, "\n");

	if(g_strcmp0(my_request.type, "GET") == 0)
	{
		char* ip = inet_ntoa(*(struct in_addr*)&my_request.ip);
		g_string_append_printf(response,"%s %s:%d \n", my_request.URI, ip, my_request.port);
	}

	if(g_strcmp0(my_request.type, "HEAD") == 0)
	{
	}

	if(g_strcmp0(my_request.type, "POST") == 0)
	{
		g_string_append_printf(response,"%s \n", my_request.body->str);
		// Free the body
		g_string_free(my_request.body, TRUE);

	}
	// Destroying headers
	g_hash_table_destroy(my_request.headers);
	return response;
} 

int main(int argc, char *argv[])
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
	server.sin_port = htons(atoi(argv[1]));
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
			 
		GString* response = process_request(first_request);
		
		// If persistent connection 
		if(strstr(response->str, "keep-alive")!= NULL)
		{
			time_t start, end;
			double elapsed;
		
			time(&start);
			time(&end);
			elapsed = difftime(end, start);	
			while(recv(connfd, message, sizeof(message) - 1, 0) && elapsed <= 30)
			{
				struct request new_request = read_request(message);
				GString* new_response = process_request(new_request);
				send(connfd, new_response->str, 100,0);
				time(&end);
				elapsed = difftime(end, start);
			}
			close(connfd);	
		}
				 
		printf("Response: %s \n",response->str);	
		send(connfd, response->str,100, 0);

		// Free the response
		g_string_free(response, TRUE);


		// Close the connection.
		shutdown(connfd, SHUT_RDWR);
		close(connfd);
	}
}
