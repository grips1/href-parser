#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
int obtain_src(char* hostname, char* directory)
{
    //implement DNS lookup to obtain IP of given URL - addr
    int 		error;
	int 		sockfd;
	short int	buffer;
	char* 		ip;
	struct addrinfo* 		sock_params = malloc(sizeof(struct addrinfo));
    struct addrinfo* 		result;
	struct sockaddr_in* 	address_struct;
	struct sockaddr*		address_struct_caster;
	struct in_addr			temp_struct;
	char*					response;
	char*					header;
	unsigned short int		port;
	port = 80;
	response = malloc(500);
	header = "GET / HTTP/1.1\r\nHost: httpforever.com\r\n\r\n";
	printf("header constructed:\n%s\n", header);
	printf("voodoo size of header:%d\nSize of the response buffer:%d\n", strlen(header), sizeof(response));
	ip = malloc(100 * sizeof(char));
	memset(sock_params, 0, sizeof(struct addrinfo));
	sock_params->ai_family = AF_INET;
    sock_params->ai_socktype = SOCK_STREAM;
    sock_params->ai_protocol = 6; //(tcp)
	error = getaddrinfo(hostname, "https", NULL, &result);
	if(error != 0)// gonna try gethostbyname() before getaddrinfo
    {
		printf("getaddr failure!\nError code:%d\nQuitting...\n", error);
		free(sock_params);
		return -1;
	}
	while(result != NULL) //setting result's(of type sockaddr) struct parameters to match those of the server.
	{
		if(result->ai_family == AF_INET6)
		{
			result = result->ai_next;
			printf("Received IPv6 address and skipping...\n");
			continue;
		}
		else if(result->ai_family == AF_INET)
		{
			//address_struct =  result->ai_addr; //no type casting needed according to man connect()...?
			printf("IPv4 address found for hostname.\n");
			break;
		}
		else
			result = result->ai_next;
	}
	address_struct = (struct sockaddr_in*) result->ai_addr;
	temp_struct = address_struct->sin_addr;
	if((ip = (inet_ntop(AF_INET, &temp_struct, ip, (2* sizeof(ip))))) == NULL)
    {
        printf("Error converting IP to string\n");
		printf("error number:%d\n", errno);
        free(ip);
		return -1;
    }
    printf("IPv4 address:%s\n", ip);
	error = inet_aton(ip, &temp_struct);
	if(error == 0)
	{
		printf("Error converting address back to binary, from string.\n");
		return -1;
	}
	//connect to server socket
	if(!(sockfd = socket(sock_params->ai_family,sock_params->ai_socktype,sock_params->ai_protocol)))
    {
        printf("Socket File Descriptor error! Quitting..\n");
        close(sockfd);
        return -1;
    }
	free(sock_params);
	address_struct = (struct sockaddr_in*) result->ai_addr;
	address_struct->sin_port = htons(port);
	address_struct->sin_family = AF_INET;
	//	address_struct->sin_addr = (unsigned int) result->ai_addr->sa_data;
	//address_struct_caster = (struct sockaddr*) address_struct;
	error = connect(sockfd, address_struct, sizeof(struct sockaddr));
	if(error != 0)
	{
		printf("Connection failure\n");
		close(sockfd);
		return -1;
	}
	if(!(buffer = send(sockfd, header, strlen(header), 0)) )
	{
		printf("send() failed, error number: %d\n", errno);
		close(sockfd);
		return -1;
	}
	printf("buffer printed:%d\n", buffer);//buffer is being printed, but no data is received...?
	//read the response, nothing is returned if GET isn't valid
	//channel response from the server to local directory
	//read(sockfd, response, strlen(response));
	close(sockfd);
	return 0;
}
int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Provide URL without the scheme\n");
        return -1;
    }
	//separate the hostname from the directory assuming no scheme specified
	char 		hostname[100];// = malloc(100 * sizeof(char));
	char* 		directory;// = malloc(200 * sizeof(char));
	int 		i;
	directory = strchr(argv[1], '/');
	printf("Directory:%s\n", directory);
	for(i = 0; i < 100 && (*(argv[1] + i) != '/' && *(argv[1] + i) != '\0'); i++)
	{
		hostname[i] = *(argv[1] + i);
	}
	hostname[i] = '\0';
	printf("hostname:%s\n", hostname);
	if(obtain_src(hostname, directory) != 0)
    {
		printf("Failure translating hostname, quitting.\n");
		return -1;
	}
	//parse data stored in obtain_src()
	//free(directory); //after call to obtain_src
    //free(hostname);
	return 0;
}
