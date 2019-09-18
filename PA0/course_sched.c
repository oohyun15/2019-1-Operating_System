#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 4096
#define MAX_BUF 256
#define MAX_COURSE 10
#define MAX_INFO 10

typedef struct CourseEntry
{
	char name[MAX_LENGTH];			   // 과목의 이름
	struct CourseEntry *prerequisites; // 선수 과목들
	int n_prerequisites;			   // 선수 과목의 수
	float difficulty;				   // 과목의 난이도
} CourseEntry;

typedef struct Node
{
	int data;
	struct Node *next;
} Node;

typedef struct Queue
{
	Node *front;
	Node *end;
	int count;
} Queue;

// Queue Method
void InitQueue(Queue *);
int IsEmpty(Queue *);
void Enqueue(Queue *, int);
int Dequeue(Queue *);

// File Functions
void FileRead(char *, FILE *, char *info[MAX_COURSE]);

// Topological Sorting Function
void TopoSorting(int n, CourseEntry *ce);




int main(int argc, char **argv)
{
	CourseEntry ce[MAX_COURSE]; 								// CourseEntry
	FILE *fddb;													// File Discriptor of Database
	FILE *fdet;													// File Discriptor of Everytime

	char bufdb[MAX_BUF], bufet[MAX_BUF]; 						// buffer of DB, ET
	int cn;														// CourseNumber

	// printf("%d\n", argc);

	if (argc == 1) 
	{
		printf("Hello world!\n");

		return 0;
	}
	
	// File Open
	if ((fddb = fopen("./in/database.csv", "r")) < 0)
	{
		printf("Database file open error.");

		return -1;
	}


	// if ((fdet = fopen(argv[1], "r")) < 0)
	if ((fdet = fopen(argv[1], "r")) < 0)
	{
		printf("Everytime file open error.");

		return -1;
	}

	// Database
	while (fgets(bufdb, MAX_BUF, fddb))
	{
		// Database Infomation
		char *Info[MAX_COURSE] = { NULL, };					// 0: name, 1:n_prerequisites 2..: prerequisites name

		// File Read
		FileRead(bufdb, fddb, Info);

		// Assign Database Informations
		strcpy(ce[cn].name, Info[0]);
		ce[cn].n_prerequisites = atoi(Info[1]);
		ce[cn].difficulty = 5.0f;							// Default value

		if (ce[cn].n_prerequisites > 0)
		{
			// Dynamic Allocation
			ce[cn].prerequisites = malloc(sizeof(CourseEntry) * ce[cn].n_prerequisites);

			// Assign Prerequisites Courses
			for (int j = 0; j < ce[cn].n_prerequisites; j++) for (int k = 0; k < cn; k++) if (strcmp(Info[j + 2], ce[k].name) == 0) ce[cn].prerequisites[j] = ce[k];	
		}
		// Count Course Number
		cn++;
	}

	// Everytime
	while (fgets(bufet, MAX_BUF, fdet))
	{
		// Everytime Infomations
		char *Info[MAX_COURSE] = { NULL, };			       // 0: name, 1: difficulty

		// File Read
		FileRead(bufet, fdet, Info);

		// Updating Course Difficulty
		for (int i = 0; i < cn; i++) if (strcmp(ce[i].name, Info[0]) == 0) ce[i].difficulty = atof(Info[1]);
	}
	// File Close
	fclose(fddb);
	fddb = NULL;
	fclose(fdet);

	// Topological Sorting
	TopoSorting(cn, ce);

	// Free
	for (int i = 0; i < cn; i++) free(ce[i].prerequisites);
	
	return 0;
}

/* Queue's Method Definitions */
// Initialize Queue
void InitQueue(Queue *queue)
{
	queue->front = queue->end = NULL;
	queue->count = 0;
}

// Check if count is 0
int IsEmpty(Queue *queue)
{
	return queue->count == 0;
}

// Enqueue data
void Enqueue(Queue *queue, int data)
{
	Node *now = malloc(sizeof(Node));
	now->data = data;
	now->next = NULL;

	if (IsEmpty(queue)) queue->front = now;
	else queue->end->next = now;

	queue->end = now;
	queue->count++;
}

// Dequeue data
int Dequeue(Queue *queue)
{
	int data = 0;
	Node *now;
	if (IsEmpty(queue))	return data;
	now = queue->front;
	data = now->data;
	queue->front = now->next;
	free(now);
	queue->count--;
	return data;
}

// FileRead Function
void FileRead(char *buf, FILE *fd, char *info[MAX_COURSE])
{
	buf[strlen(buf) - 1] = '\0'; 				// remove CR
	char *sep = strtok(buf, ",");				// Separator ","
	int i = 0;									// Information index
	while (sep != NULL)
	{
		// Delete space when first char is space
		while (sep[0] == ' ')
		{
			for (int k = 0; k < strlen(sep); k++)
			{
				if (sep[k + 1] != '\0')	sep[k] = sep[k + 1];
				
				else sep[k] = '\0';
			}
		}
		// Assign data
		info[i] = sep;

		// Count index
		i++;

		// Change separator char into NULL
		sep = strtok(NULL, ",");
	}
}

// Topological Sorting Function
void TopoSorting(int n, CourseEntry *ce)
{
	// Init Queues
	Queue queue, temp, temp2;
	InitQueue(&queue);
	InitQueue(&temp);
	InitQueue(&temp2);

	// First condition: n_prerequisites
	for (int i = 0; i < n; i++) if (ce[i].n_prerequisites == 0) Enqueue(&temp, i);

	// ERROR: Can't be existed
	if (temp.count == 0) exit(1);
	
	// count -> 1
	if (temp.count == 1) Enqueue(&queue, Dequeue(&temp));
	
	// Second condition: difficulty
	else
	{
		int count = temp.count;						// when dequeue queue, queue.count--. so, save the count value
		float diff = 0.0f;							// init value

		for (int j = 0; j < count; j++)
		{
			// Dequeue value
			int index = Dequeue(&temp);

			// index.diff > diff
			if (ce[index].difficulty > diff)
			{
				diff = ce[index].difficulty;
				InitQueue(&temp2);
				Enqueue(&temp2, index);
			}
			// index.diff == diff
			else if (ce[index].difficulty == diff) Enqueue(&temp2, index);
		}
		// count -> 1
		if (temp2.count == 1) Enqueue(&queue, Dequeue(&temp2));

		// Third condition: Alphabet Sort
		else
		{
			count = temp2.count;					// when dequeue queue, queue.count--. so, save the count value
			char name[MAX_LENGTH] = {'\0'};			// init value

			for (int j = 0; j < count; j++)
			{
				// Dequeue value
				int index = Dequeue(&temp2);

				// first value must be enqueued.
				if (j == 0)
				{
					strcpy(name, ce[index].name);
					Enqueue(&queue, index);
				}
				// Compare 2 names
				else if (strcmp(ce[index].name, name) < 0)
				{
					strcpy(name, ce[index].name);
					InitQueue(&queue);
					Enqueue(&queue, index);
				}
			}
		}
	}
	// Final value 
	int index = Dequeue(&queue);

	// output
	printf("%s\n", ce[index].name);

	// Delete ce[index] prerequisites
	for (int i = 0; i < n; i++)
	{
		if (ce[i].n_prerequisites != 0)
		{
			for (int j = 0; j < ce[i].n_prerequisites; j++)
			{
				if (strcmp(ce[i].prerequisites[j].name, ce[index].name) == 0)
				{
					for (int k = j; k < ce[i].n_prerequisites; k++)	ce[i].prerequisites[k] = ce[i].prerequisites[k + 1];

					ce[i].n_prerequisites--;
				}
			}
		}
	}
	if (n > 1)
	{
		// Delete ce[index]
		for (int i = index; i < n; i++)	ce[i] = ce[i + 1];
		
		// Recursive
		TopoSorting(--n, ce);
	}
	else return;	
}