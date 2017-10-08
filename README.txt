HTTP Server

Written by Harpa Guðrún Hreinsdóttir and Sóley Ásgeirsdóttir



The program generates HTML pages and serves them to a server. 




We have struct for the request so that when we can break down message we can see the type, URI, version, body, in_addr_t and the port.

We have the following methods outside the main. 



Read request: Has one parameter, the message. It reads the message and puts it into the request struct. We are also using a hash table so we have to insert the information into the hash table. We do that in the read request method. 



Process_request: Has one parameter and that is a request struct. In this method we process the request. In here we make sure that for a GET requests, a HTML 5 page is generated in memory. Make sure that its actual content includes the URL of the requested page and the IP address and port number of the requesting client. 
For a HEAD request we just generate the header of the requested page. For POST requests, the page has to be generated in memory and should be a HTML 5 page. Its actual content should include the URL of the requested page, the IP address and port number of the requesting client, and the data in the body of the POST request.


