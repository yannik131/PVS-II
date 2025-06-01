#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define RECURSIVE

/** 
 * 3.1 a)
 * Bei der Verwendung des voreingestellten Mutex-Typen würde das mehrfache Sperren der Mutexvariable
 * vom selben Thread zu undefiniertem Verhalten führen. Das liegt daran, dass der Thread beim rekursiven
 * Aufruf der Methode bereits Eigentümer der Mutexvariable sein kann, aber beim Ausführen der Methode
 * die Mutexvariable ebenfalls noch einmal für sich beanspruchen will. Eine reguläre Mutex-Variable würde
 * bei dem rekursiven Anfordern der Variable durch denselben Thread undefiniertes Verhalten hervorrufen,
 * während man mit dem RECURSIVE-Type einen internen Zähler führt.
 * Quelle: https://linux.die.net/man/3/pthread_mutex_lock#:~:text=If%20the%20mutex%20type%20is%20PTHREAD_MUTEX_DEFAULT%2C%20attempting%20to%20recursively%20lock%20the%20mutex%20results%20in%20undefined%20behavior
 * 
 * 3.1 c)
 * Beschreibung für n threads = 1: Ein Thread mit .me = .key = 0 und Startknoten A wird erzeugt, welcher
 * über search thread func versucht, den key = 0 im Baum von Startknoten A aus zu finden. Beim ersten
 * Aufruf von search tree würde der Thread die Mutex sperren (Counter wird von 0 auf 1 inkrementiert)
 * und feststellen, dass der key von A nicht mit 0 übereinstimmt. Er würde die Suche in Left (Knoten B)
 * fortsetzen. Hier würde er die Mutexvariable ein zweites mal sperren (counter = 2), wobei aber auch
 * hier der key nicht übereinstimmt. Da sowohl B.left als auch B.right = NULL ist, würde der Funktionsaufruf
 * mit dem dekrementieren des Counters (counter = 1) enden und man kehrt wieder in den initialen Aufruf
 * zurück. Hier würde man nun weitergehen und A.right (Knoten C) durchsuchen, mit demselben Effekt
 * wie in A.left (counter von 1 auf 2, key != 0, left == right == NULL, unlock führt zu counter von 2
 * auf 1). Abschließend würde man in den initialen Aufruf von search tree zurückkehren, wobei hier die
 * Mutex-Variable von 1 auf 0 dekrementiert wird und somit wieder komplett freigegeben wird. Danach
 * gibt die Funktion den Wert 0 zurück und die Ausgabe, dass der Key nicht gefunden wurde. Ermöglicht
 * korrekte Verwendung einer Mutex Variable in einer rekursiven Methode ohne undefiniertes Verhalten
 * hervorzurufen
 * 
 * 3.1 d)
 * Da jeder Thread die Mutex-Variable beim Beginn von search_tree bereits aufruft und somit den internen
 * Zähler auf 1 setzt, kann kein anderer Thread solange in der Funktion weiterarbeiten, bis der
 * initiale Thread entweder das Element gefunden oder den Baum komplett durchsucht hat. Das ist auch
 * unabhängig von den zwischenzeitlichen ”unlock“-Operationen des initialen Threads, da diese zwar den
 * Counter dekrementieren, jedoch nie auf 0 setzen. Effektiv kann so dauerhaft nur ein Thread den Baum
 * durchsuchen, obwohl mehrere Threads bereit sein können, wodurch ein Flaschenhals entsteht.
 * Vernachlässigbar wäre das Sperren unter der Bedingung, dass keine Schreiboperationen auf den Baum
 * durchgeführt werden, wodurch beim Lesen keine relevanten Race Conditions entstehen könnten, da der
 * Inhalt des Baumes unverändert bleibt. Man könnte auch nur den jeweiligen Teilbaum sperren, in welchem
 * Modifikationen durchgeführt werden. Dadurch könnte ein anderer Threads parallel in anderen
 * Teilbäumen ggf. sein Element suchen und finden.
*/

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
    printf("Thread %d: Search unsuccessful\n", targ->me);

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

  printf("%s", omp_num_threads_as_char);

  if (omp_num_threads_as_char != NULL)
    if (strlen(omp_num_threads_as_char) > 0)
      n_threads = atoi(omp_num_threads_as_char);

  thread = (pthread_t *) malloc(n_threads * sizeof(pthread_t));
  arg = (thread_arg_t *) malloc(n_threads * sizeof(thread_arg_t));

  pthread_mutexattr_init(&attr);

#ifdef RECURSIVE
  printf("PTHREAD_MUTEX_RECURSIVE\n");
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