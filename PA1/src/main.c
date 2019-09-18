#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#define MAX_LEN 256

///* Functions declaration *///
int parsing(char *, char **, char *);
void del_char(char *, char, int, int);
char* change_env(char *);
void mycd(int argn, char **);
void myecho(int type, char *opr, char *opn, char *filename);
void myready(int type, char **, char *filename);
void myauto(int type, char **, char *filename);
void myreport(int type, char **, char *filename);
void builtin(int type, char **, char *filename);

///* main function *///
int main()
{
	// Local variables
	char r_buf[4096];
	char *buf[MAX_LEN];
	char opn[MAX_LEN]; 								// echo 구현 시 사용

	while (fgets(r_buf, 4096, stdin) != NULL)
	{
		// Delete '\n'
		del_char(r_buf, '\n', 0, 1);
		
		// Check if r_buf is empty
		if (strlen(r_buf) == 0)	continue;

		// Check redirection
		int type = 0;
		if (strchr(r_buf, '>')) type = 1;
		else if (strchr(r_buf, '<')) type = 2;
		
		// Parse r_buf to buf
		int num = parsing(r_buf, buf, "<|>");

		// Delete space
		for (int i = 0; i < num; i++) del_char(buf[i], ' ', 1, 1);

		// Initialization
		opn[0] = '\0';
		char *args[MAX_LEN] = {	NULL, }; 
		
		// Find opn
		char *ptr = strchr(buf[0], ' ');			// 0번째 buf에 한정되어 있음!! 추후에 수정 필요
		if (ptr != NULL) strcpy(opn, ptr);

		// Delete space
		del_char(opn, ' ', 1, 1);

		// Parse buf[0] to args
		int argn = parsing(buf[0], args, " ");

		// for args[i], change ~ or $ into path
		for (int i = 0; i < argn; i++) args[i] = change_env(args[i]);

		// for opn, change ~ or $ into path
		strcpy(opn,change_env(opn));

		// Command "exit"
		if (strcmp(args[0], "exit") == 0) exit(EXIT_SUCCESS);

		// Command "cd"
		else if (strcmp(args[0], "cd") == 0) mycd(argn, args);

		// Command "echo"
		else if (strcmp(args[0], "echo") == 0) myecho(type, args[0], opn, buf[1]);

		// Command "ready-to-score"
		else if (strcmp(args[0], "ready-to-score") == 0) myready(type, args, buf[1]);

		// Command "auto-grade-pa0"
		else if (strcmp(args[0], "auto-grade-pa0") == 0) myauto(type, args, buf[1]);

		// Command "report-grade"
		else if (strcmp(args[0], "report-grade") == 0) myreport(type, args, buf[1]);
		
		// Built-in command
		else builtin(type, args, buf[1]);
	}
	return 0;
}

///* Function definition *///
int parsing(char *opn, char **args, char *op)
{
	char *sep = strtok(opn, op);

	int n = 0;

	while (sep != NULL)
	{
		args[n++] = sep;

		sep = strtok(NULL, op);
	}
	return n;
}

void del_char(char *str, char sep, int first, int end)
{
	// first char
	if (first == 1)
	{
		while (str[0] == sep)
		{
			for (int i = 0; i < strlen(str); i++)
			{
				if (str[i + 1] != '\0') str[i] = str[i + 1];

				else str[i] = '\0';
			}
		}
	}
	// end char
	if (end == 1)
	{
		while (str[strlen(str) - 1] == sep) str[strlen(str) - 1] = '\0';
	} 
}

char *change_env(char *path)
{
	if (path[0] == '~')
	{
		char temp[MAX_LEN];

		temp[0] = '\0';
		
		del_char(path, '~', 1, 0);

		strcpy(temp, getenv("HOME"));

		return strcat(temp, path);
	}
	else if (path[0] == '$')
	{
		del_char(path, '$', 1, 0);

		if ((getenv(path)) == NULL) return "";
		
		return getenv(path);
	}
	else return path;
}

void mycd(int argn, char **args)
{
	if (argn == 1) chdir(getenv("HOME"));

	else if (argn == 2)
	{
		if (chdir(args[1]) == -1)
		{
			fprintf(stderr, "There is no file or directory.\n");

			return;
		}
	}
	else
	{
		fprintf(stderr, "Too many arguments.\n");

		return;
	}
}

void myecho(int type, char *opr, char *opn, char *filename)
{
	pid_t pid;

	int status = 0;

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "FORK ERROR\n");

		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (opn[0] == '"' || opn[0] == '\'')
		{
			// first char
			for (int i = 0; i < strlen(opn); i++)
			{
				if (opn[i + 1] != '\0') opn[i] = opn[i + 1];

				else opn[i] = '\0';
			}
		}

		if (opn[strlen(opn) - 1] == '"' || opn[strlen(opn) - 1] == '\'') opn[strlen(opn) - 1] = '\0';
		
		if (type == 1)
		{
			int fd;

			if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}

			if ((dup2(fd, STDOUT_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		else if (type == 2)
		{
			int fd;

			if ((fd = open(filename, O_RDONLY | O_CREAT, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}

			if ((dup2(fd, STDIN_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}

		if ((execlp(opr, opr, opn, (char *)0)) == -1)
		{
			fprintf(stderr, "ERROR!\n");

			exit(EXIT_FAILURE);
		}
	}
	else waitpid(pid, &status, 0);
	
}

void myready(int type, char **args, char *filename)
{
	pid_t pid;

	int status = 0;

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "FORK ERROR\n");

		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (type == 1)
		{
			int fd;

			if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}

			if ((dup2(fd, STDOUT_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		else if (type == 2)
		{
			int fd;

			if ((fd = open(filename, O_RDONLY | O_CREAT, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}
			if ((dup2(fd, STDIN_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		if ((execlp("python3", "python3", "scripts/ready-to-score.py", (char *)0)) == -1)
		{
			fprintf(stderr, "ERROR!\n");

			exit(EXIT_FAILURE);
		}
	}
	else waitpid(pid, &status, 0);
}

void myauto(int type, char **args, char *filename)
{
	pid_t pid;

	int status = 0;

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "FORK ERROR\n");

		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (type == 1)
		{
			int fd;

			if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}

			if ((dup2(fd, STDOUT_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		else if (type == 2)
		{
			int fd;

			if ((fd = open(filename, O_RDONLY | O_CREAT, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}
			if ((dup2(fd, STDIN_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		if ((execlp("python3", "python3", "scripts/auto-grade-pa0.py", (char *)0)) == -1)
		{
			fprintf(stderr, "ERROR!\n");

			exit(EXIT_FAILURE);
		}
	}
	else waitpid(pid, &status, 0);
}

void myreport(int type, char **args, char *filename)
{
	pid_t pid;

	int status = 0;

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "FORK ERROR\n");

		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (type == 1)
		{
			int fd;
			
			if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}

			if ((dup2(fd, STDOUT_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}

			close(fd);
		}
		else if (type == 2)
		{
			int fd;
			if ((fd = open(filename, O_RDONLY | O_CREAT, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}
			if ((dup2(fd, STDIN_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}

		if ((execlp("python3", "python3", "scripts/report-grade.py", (char *)0)) == -1)
		{
			fprintf(stderr, "ERROR!\n");

			exit(EXIT_FAILURE);
		}
	}
	else waitpid(pid, &status, 0);
}

void builtin(int type, char **args, char *filename)
{
	pid_t pid;

	int status = 0;

	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "FORK ERROR\n");

		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (type == 1)
		{
			int fd;

			if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}
			if ((dup2(fd, STDOUT_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}

			close(fd);
		}
		else if (type == 2)
		{
			int fd;

			if ((fd = open(filename, O_RDONLY | O_CREAT, 0644)) == -1)
			{
				fprintf(stderr, "FILEOPEN ERROR\n");

				exit(EXIT_FAILURE);
			}
			if ((dup2(fd, STDIN_FILENO)) == -1)
			{
				fprintf(stderr, "DUP ERROR\n");

				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		if ((execvp(args[0], args)) == -1)
		{
			fprintf(stderr, "ERROR!\n");

			exit(EXIT_FAILURE);
		}
	}
	else waitpid(pid, &status, 0);
}
