## Contenu de la distribution

Ce répertoire contient les fichiers suivants :

- Makefile 
- README.md
- fake-malloc.c: une version de la bibliothèque d'allocation qui utilise les vraies  versions de `malloc`, free, ... 
- my-malloc.h: le header de la librarie à construire
- test-fake: une version de test-malloc liée avec `fake-malloc.c`
- test-malloc-eg: une version exécutable GNU/Linux x86_64 du programme de test 

## Exemple d'exécution du fichier de test

Un exemple d'exécution est donné ci-dessous:

	$ test-malloc 10 100
	**********
	*** INITIAL
	# allocs =   0 - # deallocs =   0 - # sbrk =   0
	Block @ 0x601570 (size=   0, next 0x601570)
	**********

	mymalloc(83) [7 blocks]
	--> sbrk(800) [50 blocks]
	mymalloc(86) [7 blocks]
	mymalloc(77) [6 blocks]
	mymalloc(15) [2 blocks]
	mymalloc(93) [7 blocks]
	mymalloc(35) [4 blocks]
	mymalloc(86) [7 blocks]
	mymalloc(92) [7 blocks]
	mymalloc(49) [5 blocks]
	--> sbrk(800) [50 blocks]
	mymalloc(21) [3 blocks]
	**********
	*** AFTER ALLOC
	# allocs =  10 - # deallocs =   0 - # sbrk =   2
	Block @ 0x601570 (size=   0, next 0x1327320)
	Block @ 0x1327320 (size=  45, next 0x601570)
	**********

	mymalloc(62) [5 blocks]
	mymalloc(27) [3 blocks]
	mymalloc(90) [7 blocks]
	mymalloc(59) [5 blocks]
	mymalloc(63) [5 blocks]
	**********
	*** MIDDLE
	# allocs =  15 - # deallocs =   5 - # sbrk =   2
	Block @ 0x601570 (size=   0, next 0x13270a0)
	Block @ 0x13270a0 (size=   2, next 0x1327150)
	Block @ 0x1327150 (size=   4, next 0x13271e0)
	Block @ 0x13271e0 (size=   1, next 0x13272b0)
	Block @ 0x13272b0 (size=  45, next 0x601570)
	**********

	**********
	*** END
	# allocs =  15 - # deallocs =  15 - # sbrk =   2
	Block @ 0x601570 (size=   0, next 0x1327000)
	Block @ 0x1327000 (size= 100, next 0x601570)
	**********

Ici on exécute notre programme de test avec 10 allocation d'au plus 100 caractères

- Pour commencer on voit que l'on a:

		**********
		*** INITIAL
		# allocs =   0 - # deallocs =   0 - # sbrk =   0
		Block @ 0x601570 (size=   0, next 0x601570)
        **********
 c'est à dire une liste vide contenant juste un bloc de taille nulle.

- Ensuite, on commence par allouer 10 blocs. On obtient:

		mymalloc(83) [7 blocks]
		--> sbrk(800) [50 blocks]
		mymalloc(86) [7 blocks]
		mymalloc(77) [6 blocks]
		mymalloc(15) [2 blocks]
		mymalloc(93) [7 blocks]
		mymalloc(35) [4 blocks]
		mymalloc(86) [7 blocks]
		mymalloc(92) [7 blocks]
		mymalloc(49) [5 blocks]
		--> sbrk(800) [50 blocks]
		mymalloc(21) [3 blocks]
		**********
		*** AFTER ALLOC
		# allocs =  10 - # deallocs =   0 - # sbrk =   2
		Block @ 0x601570 (size=   0, next 0x1327320)
		Block @ 0x1327320 (size=  45, next 0x601570)
		**********
  on voit que certaines allocations provoquent l'appel à la fonction `sbrk` (la
  première bien-sur, et l'avant dernière).
  \
  A la fin de la phase d'allocations, notre liste libre contient deux éléments
  (le bloc de taille 0 que l'on avait au début et un bloc de taille 45).

- Ensuite le programme fait 5 appels à la fonction `myfree`, puis 5 appels à
  `mymalloc`. On obtient:

		mymalloc(62) [5 blocks]
		mymalloc(27) [3 blocks]
		mymalloc(90) [7 blocks]
		mymalloc(59) [5 blocks]
		mymalloc(63) [5 blocks]
		**********
		*** MIDDLE
		# allocs =  15 - # deallocs =   5 - # sbrk =   2
		Block @ 0x601570 (size=   0, next 0x13270a0)
		Block @ 0x13270a0 (size=   2, next 0x1327150)
		Block @ 0x1327150 (size=   4, next 0x13271e0)
		Block @ 0x13271e0 (size=   1, next 0x13272b0)
		Block @ 0x13272b0 (size=  45, next 0x601570)
		**********
  Cela a conduit au morcellement de la mémoire, et maintenant, notre liste de blocs
  libres contient 4 (petits) blocs.

- Enfin, on termine par séallouer tous les blocs que l'on avait alloué et on
  obtient:

		**********
		*** END
		# allocs =  15 - # deallocs =  15 - # sbrk =   2
		Block @ 0x601570 (size=   0, next 0x1327000)
		Block @ 0x1327000 (size= 100, next 0x601570)
		**********
  ce qui veut dire que tous les blocs alloués se sont finalement réunis en un seul 
  de 100 unités (et que par conséquent notre fonction `myfree` a bien récupéré et
  concaténé tous les blocs contigus dans la liste des blocs libres).


