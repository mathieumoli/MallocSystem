/*
 * my-malloc.h 	-- Implementation de malloc, free, calloc, realloc
 *
 * Implémentation first-fit pour malloc
 *
 *           Author: Erick Gallesio [eg@unice.fr]
 *    Creation date: 11-May-2015 17:26 (eg)
 * Last file update: 11-May-2015 20:58 (eg)
 */

#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_

#include <stdlib.h>

void *mymalloc(size_t size);
void  myfree(void *ptr);
void *mycalloc(size_t nmemb, size_t size);
void *myrealloc(void *ptr, size_t size);

/* Instrumentation */
#ifdef MALLOC_DBG
   void mymalloc_infos(char *str);
#else
#  define mymalloc_infos(str)	/* nothing */
#endif
#endif /* _MY_MALLOC_H_ */
