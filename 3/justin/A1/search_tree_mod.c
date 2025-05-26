/*
  a)
  Im Allgemeinen: Das rekusive Sperren einer Mutex Variablen mit Typ PTHREAD_MUTEX_DEFAULT erzeugt undefiniertes Verhalten.
  pthread_mutexattr_init erstellt ein mutex-attributes-object mit Default-Werten.
  Diese werden (wenn RECURSIVE nicht definiert ist) nicht verändert.
  Quellen:
  https://pubs.opengroup.org/onlinepubs/009604499/functions/pthread_mutex_lock.html
  https://pubs.opengroup.org/onlinepubs/009696899/functions/pthread_mutexattr_gettype.html
  https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutexattr_init.html

  c)
  Immer wenn ein search-thread die Suche beginnt, versucht er den Mutex zu sperren.
  Wenn das Sperren erfolgreich war, ist der Thread im Besitz des Mutex und ein Mutex interner Zähler wird inkrementiert.
  Jedes Mal, wenn ein neuer Knoten des Baumes von diesem Thread betrachtet wird (d.h. search_tree aufgerufen wird)
  wird der Mutex ein weiteres Mal gesperrt, d.h. der Zähler inkrementiert.
  Vor dem Verlassen von search_tree wird jedes Mal der Mutex unlocked, d.h. der Zähler dekrementiert.
  Nachdem der ursprüngliche Aufruf von search_tree beendet ist, wurde jede Sperre wieder freigegeben, d.h. der Zähler ist auf 0,
  d.h. der Mutex ist frei und kann (ggf.) von einem anderen Thread gesperrt werden.

  d)
  Wenn nur eine einzelne Mute-Variable für den ganzen Baum zuständig ist, kann immer nur ein Thread gleichzeitig auf dem Baum arbeiten.
  Das bedeutet, dass die Threads alle sequentiell arbeiten müssen.
  (Je größer der Baum ist, desto länger dauert jeder Suchvorgang.)

  Das Sperren ist vernachlässigbar, wenn:
    - der Baum sehr klein ist oder
    - es nur einen (oder wenige) Threads gibt
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// b) 
// Wir definieren RECURSIVE, wodurch der Typ des Mutex im Programm auf PTHREAD_MUTEX_RECURSIVE gesetzt wird
#ifndef RECURSIVE
  #define RECURSIVE
#endif

typedef struct node_
{
  int key;
  int value;
  struct node_ *left, *right;
} node_t;

pthread_mutex_t tree_lock;

typedef struct
{
  int me;
  int key;
  node_t *node;
} thread_arg_t;

int search_tree(node_t * node, int key)
{
  pthread_mutex_lock(&tree_lock);

  if (node->key == key)
  {
    /* solution is found here */
    printf("key = %d, value = %d\n", key, node->value);
    pthread_mutex_unlock(&tree_lock);
    return 1;
  }

  if (node->left != NULL)
    if (search_tree(node->left, key))
    {
      pthread_mutex_unlock(&tree_lock);
      return 1;
    }

  if (node->right != NULL)
    if (search_tree(node->right, key))
    {
      pthread_mutex_unlock(&tree_lock);
      return 1;
    }

  pthread_mutex_unlock(&tree_lock);
  return 0;
}

void *search_thread_func(void *arg)
{
  thread_arg_t *targ = (thread_arg_t *) arg;

  if (!search_tree(targ->node, targ->key))
    printf("Thread %d: Search unsuccessfull\n", targ->me);

  return NULL;
}

int main(int argc, char *argv[])
{
  node_t A, B, C;
  pthread_mutexattr_t attr;
  char *omp_num_threads_as_char;
  int n_threads = 1;
  pthread_t *thread;
  thread_arg_t *arg;
  int i;

  omp_num_threads_as_char = getenv("OMP_NUM_THREADS");

  if (omp_num_threads_as_char != NULL)
    if (strlen(omp_num_threads_as_char) > 0)
      n_threads = atoi(omp_num_threads_as_char);

  thread = (pthread_t *) malloc(n_threads * sizeof(pthread_t));
  arg = (thread_arg_t *) malloc(n_threads * sizeof(thread_arg_t));

  pthread_mutexattr_init(&attr);

#ifdef RECURSIVE
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif

  pthread_mutex_init(&tree_lock, &attr);

  A.key = 3;
  A.value = 1;
  B.key = 2;
  B.value = 2;
  C.key = 1;
  C.value = 3;

  A.left = &B;
  A.right = &C;
  B.left = NULL;
  B.right = NULL;
  C.left = NULL;
  C.right = NULL;

  printf("Starting %d threads...\n", n_threads);

  for (i = 0; i < n_threads; i++)
  {
    arg[i].me = i;
    arg[i].key = i;
    arg[i].node = &A;
    pthread_create(&thread[i], NULL, search_thread_func, &arg[i]);
  }

  printf("Joining threads...\n");

  for (i = 0; i < n_threads; i++)
    pthread_join(thread[i], NULL);

  pthread_mutex_destroy(&tree_lock);
  pthread_mutexattr_destroy(&attr);

  free(thread);
  free(arg);

  return EXIT_SUCCESS;
}


