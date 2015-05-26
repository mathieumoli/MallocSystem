/*
 * fake-malloc.c 	-- Implementation de malloc, free, calloc, realloc
 *
 * Implémentation first-fit pour malloc
 *
 *           Author: Erick Gallesio [eg@unice.fr]
 *    Creation date: 11-May-2015 17:26 (eg)
 * Last file update: 11-May-2015 20:55 (eg)
 */

#include <stdio.h>
#include "my-malloc.h"


/* Une version de my-malloc.c qu'il faudra vite remplacer par quelque chose de
 * plus "raisonnable". Ce code n'est juste là que pour pouvoir lancer
 * les programmes de test.
 */
static int nb_alloc   = 0;		/* Nombre de fois où on alloué     */
static int nb_dealloc = 0;		/* Nombre de fois où on désalloué  */
static int nb_sbrk    = 0;		/* nombre de fois où a appelé sbrk */


void *mymalloc(size_t size) {
  nb_alloc += 1;
  return malloc(size);
}


void myfree(void *ptr) {
  nb_dealloc += 1;
  free(ptr);
}

void *mycalloc(size_t nmemb, size_t size) {
  nb_alloc += 1;
  return calloc(nmemb, size);
}

void *myrealloc(void *ptr, size_t size) {
  /* il faudrait probablement changer les valeur de nballoc et
   * nb_dealloc dans une véritable implémentation 
   */
  return realloc(ptr, size);
}

#ifdef MALLOC_DBG
void mymalloc_infos(char *msg) {
  if (msg) fprintf(stderr, "**********\n*** %s\n", msg);

  fprintf(stderr, "# allocs = %3d - # deallocs = %3d - # sbrk = %3d\n",
	  nb_alloc, nb_dealloc, nb_sbrk);
  /* Ca pourrait être pas mal d'afficher ici les blocs dans la liste libre */

  if (msg) fprintf(stderr, "**********\n\n");
}
#endif
