/*
 * test-malloc.c 	-- Un programme simple de test de malloc
 *
 *           Author: Erick Gallesio [eg@unice.fr]
 *    Creation date: 11-May-2015 17:47 (eg)
 * Last file update: 11-May-2015 21:17 (eg)
 */

#include <stdio.h>
#include <strings.h>
#include "my-malloc.h"


#define N     10	/* nombre d'allocations par défaut */
#define SZMAX 200	/* taille maximum à allouer par défaut */



/* Boucle d'allocation n fois max (au maximum) caractères puis désallocation */
void do_alloc_free(int n, int max) {
  void *tab[n];
  int i;

  /* boucle d'allocation */
  for (i = 0; i < n; i ++) {
    int size = rand() % max;	/* tirer un nombre dans l'intervalle [0, max-1] */
    tab[i] = mymalloc(size);
    if (tab[i] == NULL)
      printf("**NOT OK @%d (size = %d)**\n", i, size);

    /* remplir la zone de zeros pour tester en écriture */
    bzero(tab[i], size);
  }
  mymalloc_infos("AFTER ALLOC");

  
  /* boucle de libération de la mémoire:  */
  /*      - Etape 1: on libère un élément sur deux, histoire de morceler la mémoire */
  for (i = 0; i <n; i +=2) {
    if (tab[i] != NULL) {
myfree(tab[i]);
} }
  mymalloc_infos("AFTER DESALLOC 1/2");

  /*      - Etape 2: on alloue dans les nouveaux trous */
  for (i = 0; i < n; i += 2) {
    int size = rand() % max;	/* tirer un nombre dans l'intervalle [0, max-1] */
    tab[i] = mymalloc(size);
	
	mymalloc_infos("liste");
    if (tab[i] == NULL)
      printf("**NOT OK (2nd allocation loop) @%d (size = %d)**\n", i, size);
  }
  mymalloc_infos("MIDDLE");

  /*       - Etape 3: On libère le tout */
  for (i = 0; i < n; i++) {
    if (tab[i] != NULL) {//fprintf(stderr,"%d",tab[i]);
myfree(tab[i]);
mymalloc_infos(" nouvelle liste");
}
  }
}


int main(int argc, char *argv[]) {
  int n = N;
  int szmax = SZMAX;

  switch (argc) {
    case 1:
      break;
    case 2:
      n = atoi(argv[1]);
      break;
    case 3:
      n     = atoi(argv[1]);
      szmax = atoi(argv[2]);
      break;
    default:
      fprintf(stderr,"Usage: %s [nloop [szmax]]\n", *argv);
      exit(1);
  }
  mymalloc_infos("INITIAL");
  do_alloc_free(n, szmax);
  mymalloc_infos("END");
  return 0;
}
