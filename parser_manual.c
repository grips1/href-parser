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
	struct addrinfo* 		hints = malloc(sizeof(struct addrinfo));
    struct addrinfo* 		result;
	struct sockaddr_in* 	address_struct;
	struct in_addr			temp_struct;
	//char 					header[100];
	char*					header;
	//sprintf(header, "GET %s HTTP/1.1\nHostname: %s\n", directory, hostname); //note:HTTP3 isn't backwards compatible, so this wouldn't work.
	header = "GET / HTTP/1.1\nHost: httpforever.com\n";
	printf("header constructed:\n%s\n", header);
	printf("voodoo size of header:%d\n", strlen(header));
	ip = malloc(100 * sizeof(char));
	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_protocol = 6; //(tcp)
	error = getaddrinfo(hostname, "https", NULL, &result);
	if(error != 0)// gonna try gethostbyname() before getaddrinfo
    {
		printf("getaddr failure!\nError code:%d\nQuitting...\n", error);
		free(hints);
		return -1;
	}
	free(hints);
	while(result != NULL)
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
	//int inet_aton(const char *cp, struct in_addr *inp);
	//printf("ipv4:%s\n", result->ai_addr->sa_data);
	//const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
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
		printf("Error converting address\n");
		return -1;
	}

	//connect to server socket
	if(!(sockfd = socket(AF_INET, SOCK_STREAM, 6)))
    {
        printf("Socket File Descriptor error! Quitting..\n");
        close(sockfd);
        return -1;
    }
	error = connect(sockfd, result->ai_addr, sizeof(struct sockaddr));
	if(error != 0)
	{
		printf("Connection failure\n");
		close(sockfd);
		return -1;
	}
    //send HTTP packet, how to craft the header?
	//snprintf()?
	//create formatted GET request format. What're the requirements for an HTTP GET request?
	if(!(buffer = send(sockfd, header, strlen(header), 0)) )
	{
		printf("send() failed, error number: %d\n", errno);
		close(sockfd);
		return -1;
	}
	printf("buffer printed:%d\n", buffer);//buffer is being printed, but no data is received...?
    close(sockfd);
	//channel response from the server to local directory
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
