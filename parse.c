#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int obtain_page(const char* url, const char* filename)
{
	CURL* handle = curl_easy_init();
	if(handle)
	{	
		curl_easy_setopt(handle, CURLOPT_URL, url);
		FILE* newfile = fopen(filename, "w");
		if(newfile)
		{
			curl_easy_setopt(handle, CURLOPT_WRITEDATA, newfile);
			printf("Fetching site source...\n");
			curl_easy_perform(handle);
			fclose(newfile);
			curl_easy_cleanup(handle);
			return 1;
		}
		else 
		{
			printf("Failure opening file!\n");
			fclose(newfile);
			curl_easy_cleanup(handle);
			return 0;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("Please insert URL and filename as parameter!\n"); //Note:if target URL has a scheme specified at the end of the domain, specify the full URL, otherwise insert the domain name purely.\nExample:https:/example.ftp/\n");
		return -1;
	}
	int z;
	char* str;
	char* filename = argv[2];
	if((str = (char*) malloc(201)) == NULL)
	{
		printf("malloc() error!\n");
		return -1;
	}
	
	FILE* target_src;
	if(!obtain_page(argv[1], filename))
	{
	 	perror("Error grabbing URL!\n");
		return -1;
	}
	if(!(target_src = fopen(filename, "r")) ) 
	{
		perror("Error opening file!\nQuitting!\n");
		fclose(target_src);
		return -1;
	}
	if(str)
	{
		printf("parsing for hrefs...\n");
		while(fgets(str, 200, target_src) != NULL)
		{	
			for(int i = 0; i < 200; i++)
			{
				if(*(str+i) == 'h' && *(str+i+5) == '"') //IF HREF DETECTED
				{
					for(z = 6;z < 200; z++) //start from the double quotes
					{
						if((*(str+i+z) != 0 && *(str+i+z) != '"')) 
							printf("%c", *(str+i+z));
						else break;
					}
					printf("\n");
				}
			}
		}
	}
	free(str);
	fclose(target_src);
	return 0;
}