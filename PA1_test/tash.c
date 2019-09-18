#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#define MAX_PATH		100
#define MAX_LENGTH		1024
#define SPACE			" \n\t"
#define SLASH			"/"


int sep(char*, char**, char*);

int main(void)
{


	for ( ; ; )
	{
		char buf[MAX_PATH];

		char *args[MAX_PATH] = {NULL, };

		printf("$ ");

		fgets(buf, MAX_PATH, stdin);
	
		buf[strlen(buf) - 1] = '\0'; 

		if (strlen(buf) == 0) continue;

		int n_args = sep(buf, args, SPACE);
		




		/* Command */
		// pwd
		if (strcmp(args[0], "pwd") == 0)
		{
			if (n_args != 1)
			{
				printf("usage: a,out <descriptor #>\n");
			
				continue;
			}
			char *temp = malloc(sizeof(char *));

			if ((getcwd(temp, MAX_PATH)) != 0) printf("%s\n", temp);

			free(temp);
		}

		// exit
		else if(strcmp(args[0], "exit") == 0)
		{
			break;
		}

		// cd
		else if(strcmp(args[0], "cd") == 0)
		{
			if (n_args > 2)
			{
				printf("usage: a.out <descriptor #>\n");

				continue;
			}

			else if ( n_args == 1) chdir(getenv("HOME"));
/*		env value		
		char *path = args[1];

		if ((path = (char *)getenv("HOME")) == NULL) path = ".";
*/
			else
			{
				if (chdir(args[1]) == -1)
				{
					for (int i = 0; i < n_args; i++) printf("%s: ", args[i]);

					printf("There is no file or directory.\n");
				}
			}
		}

		// echo
		else if(strcmp(args[0], "echo") == 0)
		{
			if (n_args < 2)
			{
				printf("usage: a.out <descriptor #>\n");

				continue;
			}

			for (int i = 1 ; i < n_args ; i++)
			{
				printf("%s ", args[i]);
			}
			printf("\n");
		}

		// Unknown
		else
		{
			printf("%s: Can't find a command.\n", args[0]);
		}
		


		
		
		
	}

	return 0;
}

int sep(char* buf, char** args, char* separator)
{
	int n = 0;

	char* sep = strtok(buf, separator);


	while(sep != NULL)
	{
		
		args[n++] = sep;

	//	printf("%s ", args[n-1]);	
		sep = strtok(NULL, separator);
	}
	//	printf("\targ num: %d \n", n);


	return n;
	
}
