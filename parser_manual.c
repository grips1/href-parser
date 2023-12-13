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
#define PORT 443

FILE* obtain_src(char* hostname, char* directory)
{
    //implement DNS lookup to obtain IP of given URL - addr
    	FILE*		response_file;
	int 		error;
	int 		sockfd;
	int			buffer;
	char* 		ip;
	struct addrinfo* 		sock_params = malloc(sizeof(struct addrinfo));
    	struct addrinfo* 		result;
	struct sockaddr_in* 	address_struct;
	struct in_addr			temp_struct;
	char*					response;
	char					header[1000];
	response = malloc(100000);
	sprintf(header, "GET %s HTTP/1.1\r\nHost: %s\r\n", directory, hostname);
	ip = malloc(100 * sizeof(char));
	memset(sock_params, 0, sizeof(struct addrinfo));
	sock_params->ai_family = AF_INET;
    sock_params->ai_socktype = SOCK_STREAM;
    sock_params->ai_protocol = 6; //(tcp)
	error = getaddrinfo(hostname, "https", NULL, &result);
	if(error != 0)
    {
		printf("getaddr failure!\nError code:%d\nQuitting...\n", error);
		free(sock_params);
		return NULL;
	}
	while(result != NULL) 
	{
		if(result->ai_family == AF_INET6)
		{
			result = result->ai_next;
			printf("IPv6 skipped!\n");
			continue;
		}
		else if(result->ai_family == AF_INET)
		{
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
		return NULL;
    }
    printf("IPv4 address:%s\n", ip);
	error = inet_aton(ip, &temp_struct);
	if(error == 0)
	{
		printf("Error converting address back to binary, from string.\n");
		return NULL;
	}
	//connect to server socket
	if(!(sockfd = socket(sock_params->ai_family,sock_params->ai_socktype,sock_params->ai_protocol)))
    {
        printf("Socket File Descriptor error! Quitting..\n");
        close(sockfd);
        return NULL;
    }
	free(sock_params);
	address_struct = (struct sockaddr_in*) result->ai_addr;
	address_struct->sin_port = htons(PORT);
	address_struct->sin_family = AF_INET;
	error = connect(sockfd, address_struct, sizeof(struct sockaddr));
	if(error != 0)
	{
		printf("Connection failure\n");
		close(sockfd);
		return NULL;
	}
	if(!(buffer = send(sockfd, header, sizeof(header), 0)) )
	{
		printf("GET failure!\n");
		close(sockfd);
		return NULL;
	}
	printf("data sent:%d\n\n\n", buffer);
	buffer = read(sockfd, response, (100000 * sizeof(char)) );
	if(!buffer)
	{
		printf("read error! quitting\n");
		return NULL;
	}
	printf("size read:%d\nresponse header:\n\n%s\n", buffer, response);
	close(sockfd);
	response_file = fopen(hostname, "w+");
	fputs(response, response_file);
	return response_file;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Provide URL without the scheme\n");
        return -1;
    }
	char 		hostname[100];
	char* 		directory;
	int 		i, z;
	char* 		line_buffer;
	FILE*		header_file;
	line_buffer = (char*) malloc(5000);
	memset(line_buffer, 0, 5000);
	directory = strchr(argv[1], '/');
	printf("Directory:%s\n", directory);
	for(i = 0; i < 100 && (*(argv[1] + i) != '/' && *(argv[1] + i) != '\0'); i++)
	{
		hostname[i] = *(argv[1] + i);
	}
	hostname[i] = '\0';
	printf("hostname:%s\n", hostname);

	if((header_file = obtain_src(hostname, directory)) == NULL)
    {
		printf("Failure translating hostname, quitting.\n");
		return -1;
	}
	fclose(header_file);
	header_file = fopen(hostname, "r");
	while((fgets(line_buffer, 5000, header_file)) != NULL)
	{
		for(i = 0; i < 200; i++)
		{
			if(*(line_buffer+i) == 'h' && *(line_buffer+i+5) == '"') //IF HREF DETECTED
			{
				for(z = 6;z < 200; z++) //start from the double quotes
				{
					if((*(line_buffer+i+z) != 0 && *(line_buffer+i+z) != '"'))
						printf("%c", *(line_buffer+i+z));
					else break;
				}
				printf("\n");
			}
		}
	}
	return 0;
}
