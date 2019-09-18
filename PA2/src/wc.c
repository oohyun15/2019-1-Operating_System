/*********************************************************
 * PA2: Word Counting

   [Index]
 * 1. Preprocess                - Parallelization
 * 2. Tokenization              - Serialization
 * 3. Word counting             - Parallelization
 * 4. Find max frequency        - Parallelization
 * 5. Print the counting result - Serialization
 * 6. Release                   - Parallelization
 
 * 1~3 단계: 파일의 각 라인별로 실행 
 * 4~6 단계: 한번에 실행
 * 처음 1~3단계의 병렬화 작업은 fgets()으로 얻은 각 라인마다 순차적으로 진행했음
 * 한번에 단계별 병렬화 진행 시 파일의 크기가 클 경우 메모리 덤프 발생으로 인해 하지 못했음
 
 * Makefile 커맨드 추가
 * make test1: diff 커맨드를 통해 owl.txt 파일을 비교
 * make test2: diff 커맨드를 통해 therepublic.txt 파일을 비교

*********************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/queue.h>
#include <pthread.h>
#include <unistd.h>

#define MTX_NUM 8
#define THR_NUM 4

static LIST_HEAD(listhead, entry) head;
struct listhead *headp = NULL;
pthread_t *p_thread;
int num_entries = 0;
int counter;
int tok_len;
int max_frequency = 0;
char new_buf[4096];
static char *tok_for_parallel[4096] = {0, };
const char *WHITE_SPACE;
pthread_mutex_t mutex_lock[MTX_NUM];

struct entry
{
  char name[BUFSIZ];
  int frequency;
  LIST_ENTRY(entry)
  entries;
};
static struct entry *word_entries;

void *Preprocess(void *);
void *Word_counting(void *);
void *Find_max(void *);

int main(int argc, char **argv)
{
  for (int i = 0; i < MTX_NUM; i++)
  {
    pthread_mutex_init(&mutex_lock[i], NULL);
  }

  if (argc != 2)
  {
    fprintf(stderr, "%s: not enough input\n", argv[0]);
    exit(1);
  }
  int status;
  FILE *fp = fopen(argv[1], "r");
  WHITE_SPACE = " \t\n";
  char buf[4096];
  p_thread = (pthread_t *)malloc(THR_NUM * sizeof(pthread_t));
  LIST_INIT(&head);
  
  while (fgets(buf, 4096, fp) != NULL)
  {
    /* 1. Preprocess for parallelization */
    counter = 0;
    for (int i = 0; i < THR_NUM; i++)
    {
      status = pthread_create(&p_thread[i], NULL, Preprocess, (void *)buf);
    if (status != 0)
      {
        fprintf(stderr, "thr %d create error.\n", i);
        exit(1);
      }
    }
   
    for (int i = 0; i < THR_NUM; i++)
    {
      status = pthread_join(p_thread[i], NULL);
    if (status != 0)
      {
        fprintf(stderr, "thr %d join error.\n", i);
        exit(1);
      }      
    }

    for (int i = strlen(buf); i< strlen(new_buf); i++)
    {
      new_buf[i] = '\0';
    }

    /* 2. Tokenization for serialization */
    char *tok = strtok(new_buf, WHITE_SPACE);
    if (tok == NULL || strcmp(tok, "") == 0) continue;
    counter = 0;
    tok_for_parallel[counter++] = tok;

    // loop until NULL
    for (;;)
    {
      tok = strtok(NULL, WHITE_SPACE);
      if (tok == NULL)
        break;
      tok_for_parallel[counter++] = tok;
    }

    // total token number
    tok_len = counter;

    /* 3. Word Counting for parallelization */
    counter = 0;
    for (int i = 0; i < THR_NUM; i++)
    {
      status = pthread_create(&p_thread[i], NULL, Word_counting, NULL);
    if (status != 0)
      {
        fprintf(stderr, "thr %d create error.\n", i);
        exit(1);
      }
    }

    for (int i = 0; i < THR_NUM; i++)
    {
      status = pthread_join(p_thread[i], NULL);
    if (status != 0)
      {
       fprintf(stderr, "thr %d join error.\n", i);
       exit(1);
      }      
    }

    // Initialize Preprocess buf memory 
    new_buf[0] = '\0';
  }
  
  /* 4. Find Max frequency for parallelization */
  counter = 0;
  word_entries = head.lh_first;

  for (int i = 0; i < THR_NUM; i++)
  {
    status = pthread_create(&p_thread[i], NULL, Find_max, NULL);
    if (status != 0)
    {
      fprintf(stderr, "thr %d create error.\n", i);
      exit(1);
    }
  }

  for (int i = 0; i < THR_NUM; i++)
  {
    status = pthread_join(p_thread[i], NULL);
    if (status != 0)
    {
      fprintf(stderr, "thr %d join error.\n", i);
      exit(1);
    }
  }

  /* 5. Print the counting result very very slow way. */
  for (int it = max_frequency; it > 0; --it)
  {
    for (struct entry *np = head.lh_first; np != NULL; np = np->entries.le_next)
    {
      if (np->frequency == it)
      {
        printf("%s %d\n", np->name, np->frequency);
      }
    }
  }

  /* 6. Release */
  while (head.lh_first != NULL)
  {
    LIST_REMOVE(head.lh_first, entries);
  }

  for (int i = 0; i < MTX_NUM; i++)
  {
    status = pthread_mutex_destroy(&mutex_lock[i]);
    if (status != 0)
    {
      fprintf(stderr, "MUTEX %d DESTROY ERROR\n", i);
      exit(1);
    }
  }

  return 0;
}

void *Preprocess(void *my_buf)
{
  int index;
  char *buf = (char *)my_buf;
  int buf_len = strlen(buf);

  pthread_mutex_lock(&mutex_lock[0]);
  index = counter++;
  pthread_mutex_unlock(&mutex_lock[0]);

  for (int i = index; i < buf_len; i += THR_NUM)
  {
    if (!isalnum(buf[i]))
    {
      new_buf[i] = ' ';
    }
    else
    {
      new_buf[i] = tolower(buf[i]);
    }
  }
  pthread_exit(0);
}

void *Word_counting(void *arg)
{
  pthread_t tid = pthread_self();

  int index;
  pthread_mutex_lock(&mutex_lock[0]);
  index = counter++;
  pthread_mutex_unlock(&mutex_lock[0]);

  for (int i = index; i < tok_len; i += THR_NUM)
  {
    if (num_entries == 0)
    {
      struct entry *e = malloc(sizeof(struct entry));
      strcpy(e->name, tok_for_parallel[i]);
      e->frequency = 1;
      pthread_mutex_lock(&mutex_lock[1]);
      LIST_INSERT_HEAD(&head, e, entries);
      num_entries++;
      pthread_mutex_unlock(&mutex_lock[1]);

      continue;
    }

    else if (num_entries == 1)
    {
      int cmp = strcmp(tok_for_parallel[i], head.lh_first->name);
      if (cmp == 0)
      {
        pthread_mutex_lock(&mutex_lock[2]);
        head.lh_first->frequency++;
        pthread_mutex_unlock(&mutex_lock[2]);
      }
      else if (cmp > 0)
      {
        struct entry *e = malloc(sizeof(struct entry));

        strcpy(e->name, tok_for_parallel[i]);
        e->frequency = 1;

        pthread_mutex_lock(&mutex_lock[3]);
        LIST_INSERT_AFTER(head.lh_first, e, entries);
        num_entries++;
        pthread_mutex_unlock(&mutex_lock[3]);
      }
      else if (cmp < 0)
      {
        struct entry *e = malloc(sizeof(struct entry));

        strcpy(e->name, tok_for_parallel[i]);
        e->frequency = 1;

        pthread_mutex_lock(&mutex_lock[4]);
        LIST_INSERT_HEAD(&head, e, entries);
        num_entries++;
        pthread_mutex_unlock(&mutex_lock[4]);
      }
      continue;
    } 

    // Reduce: actual word-counting
    struct entry *np = head.lh_first;
    struct entry *final_np = NULL;

    int last_cmp = strcmp(tok_for_parallel[i], np->name);

    if (last_cmp < 0)
    {
      struct entry *e = malloc(sizeof(struct entry));

      strcpy(e->name, tok_for_parallel[i]);
      e->frequency = 1;

      pthread_mutex_lock(&mutex_lock[5]);
      LIST_INSERT_HEAD(&head, e, entries);
      num_entries++;
      pthread_mutex_unlock(&mutex_lock[5]);
      continue;
    }
    else if (last_cmp == 0)
    {
      pthread_mutex_lock(&mutex_lock[6]);
      np->frequency++;
      pthread_mutex_unlock(&mutex_lock[6]);
      continue;
    }

    // else last_cmp > 0
    pthread_mutex_lock(&mutex_lock[7]);
    for (np = np->entries.le_next; np != NULL; np = np->entries.le_next)
    {
      int cmp = strcmp(tok_for_parallel[i], np->name);

      if (cmp == 0)
      {
        np->frequency++;
        break;
      }
      else if (last_cmp * cmp < 0)
      { 
        // sign-crossing occurred
        struct entry *e = malloc(sizeof(struct entry));
        strcpy(e->name, tok_for_parallel[i]);
        e->frequency = 1;

        LIST_INSERT_BEFORE(np, e, entries);
        num_entries++;

        break;
      }

      if (np->entries.le_next == NULL)
      {
        final_np = np;
      }
      else
      {
        last_cmp = cmp;
      }
    }

    if (!np && final_np)
    {
      struct entry *e = malloc(sizeof(struct entry));

      strcpy(e->name, tok_for_parallel[i]);
      e->frequency = 1;

      LIST_INSERT_AFTER(final_np, e, entries);
      num_entries++;
    }
    pthread_mutex_unlock(&mutex_lock[7]);

    continue;
  }
}

void *Find_max(void *arg)
 {
  int index;
  pthread_mutex_lock(&mutex_lock[0]);
  index = counter++;
  pthread_mutex_unlock(&mutex_lock[0]);
  for (int i = index; i < num_entries && word_entries != NULL; i += THR_NUM, word_entries = word_entries->entries.le_next)
  {
    if (max_frequency < word_entries->frequency)
    {
      pthread_mutex_lock(&mutex_lock[1]);
      max_frequency = word_entries->frequency;
      pthread_mutex_unlock(&mutex_lock[1]);
    }
  }
}