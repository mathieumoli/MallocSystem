#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include "my-malloc.h"

#define MOST_RESTRICTING_TYPE double
#define SIZE_FOR_SBRK 800 

// la taille est 16
#define HEADER_SIZE sizeof(union header)

static int nb_alloc   = 0;              /* Nombre de fois où on alloué     */
static int nb_dealloc = 0;              /* Nombre de fois où on désalloué  */
static int nb_sbrk    = 0;              /* nombre de fois où a appelé sbrk */

typedef union header {
	struct {
		unsigned int size;
		union header *next;
	} info;
	MOST_RESTRICTING_TYPE dummy; //pour l'alignement car double est obligatoirement sur un multiple de 8
} *Header;

static void*freelist = NULL;



void creationHeader(void* adresse,size_t size){
	Header newBlock=adresse;
	newBlock->info.size=size;
	newBlock->info.next=NULL;
	fprintf(stderr, "Block Alloué @0x%d (size=%d, next 0x%d)\n", newBlock, newBlock->info.size, newBlock->info.next);

}

void fusionBlocs(){
	Header headerCourant=freelist;

	Header headerSuivant=headerCourant->info.next;
	void * adresseCourant;
	void * adFreelist=freelist;
	 			void * next;

	do{
                adresseCourant = headerCourant;
                while((adresseCourant+HEADER_SIZE+headerCourant->info.size)==headerSuivant){
                        headerCourant->info.size+=HEADER_SIZE+headerSuivant->info.size;
                        headerCourant->info.next=headerSuivant->info.next;
                        headerSuivant=headerSuivant->info.next;
                        //printf("into the while %d",headerCourant->info.size);
                }
                headerCourant=headerSuivant;
          //fprintf(stderr, "Block @0x%d (size=%d, next 0x%d)\n", headerCourant, headerCourant->info.size, headerCourant->info.next);
          //fprintf(stderr, "Freelist @0x%d\n", freelist);

                headerSuivant=headerSuivant->info.next;
                next=headerCourant->info.next;
                
        }while(next!=adFreelist);
}

void * couper(Header theheader,size_t size){

	unsigned int tailleBloc=theheader->info.size;
	int tailleNouveauBlocVide=tailleBloc-(size+HEADER_SIZE);
	theheader->info.size=tailleNouveauBlocVide;
	fprintf(stderr, "Block @0x%d (Nouvelle Size=%d , Ancienne Size=%d, next 0x%d)\n", theheader, theheader->info.size,tailleBloc, theheader->info.next);

	//pointeur vers la nouvelle source avec la metadonnée
	void * adresseHeader=theheader;
	void * pointeur=adresseHeader+HEADER_SIZE+tailleNouveauBlocVide;
	creationHeader(pointeur,size);

	return pointeur;

}
//searchinfreelist renvoie le pointeur de l'emplacement avec le header modifié ou null
void* searchinfreelist(size_t size){

	void * freeAssezGrand=NULL;
	Header headerPrecedent=freelist;
	Header headerCourant=freelist;
	void * next= headerPrecedent->info.next;
		//je me positionne : precedent est le dernier, courant est le premier
	while(next!=freelist){
		headerPrecedent=headerPrecedent->info.next;
		next=headerPrecedent->info.next;
			//printf("je suis passé une fois");
	}
	do{
		if(headerCourant->info.size>=size){
			printf("taille du bloc : %d\n",headerCourant->info.size );
			printf("taille demandée : %d\n",size );

			// je considere que si la taille n'est pas superieur à la taille demandée
			// + 2 header je lui donne car sinon le bloc ne sera jamais utilisé et donc inutile
			if(headerCourant->info.size>=(size+HEADER_SIZE+16)){
				printf("je decoupe la case est assez grande\n");
				freeAssezGrand=couper(headerCourant,size);
				return freeAssezGrand;
			}
			else{
				//cas particulier un seul element juste de la taille 
				//la freeliste passe a NULL
				if(headerCourant==freelist && headerCourant->info.next==freelist)
				{
					freelist=NULL;
				}
				//printf("je suis de la bonne taille");
				//je change freelist
				if(freelist==headerCourant){
					freelist=headerCourant->info.next;
				}
				freeAssezGrand=headerCourant;
				headerPrecedent->info.next=headerCourant->info.next;
				return freeAssezGrand;
			}
		}
		//printf("courant trop petit");
		headerPrecedent=headerCourant;
		headerCourant=headerCourant->info.next;
		//tant que j'ai pas fait un tour complet
	}while (headerCourant!=freelist);
	
	
	return freeAssezGrand;
}

/**
 * A first fit malloc
 * @see malloc(3)
 * 
 * La fonction mymalloc retourne un pointeur 
 * sur un bloc assez grand pour contenir un objet de taille size caracteres
 * (size_t un entier long non signé). En cas d'échec, 
 * mymalloc retourne le pointeur NULL
 * 
 * Lorsque l'utilisateur demande n octets de mémore, l'algorithme parcourt 
 * la liste des blocs libres à la recherche d'un bloc assez grand pour 
 * satisfaire la demande. Si un tel bloc existe, les n premiers octets de ce 
 * bloc sont retournés à l'utilisateur, tandis que le reste du bloc est laissé
 * dans la liste des blocs libres. Dans le cas contraire, la fonction sbrk est 
 * appelée pour faire grossir le segment de données du processus
 *  
 */
 void *mymalloc(size_t size)
 {
 	static int placeReserve=0;
	static void*adressePlaceRes=NULL;

 	void *  nouvelleAdresse=NULL;
 	void * adresseSansMeta=NULL;
 	nb_alloc += 1;
	//blocs potentiels
 	if(freelist!=NULL){
		//printf("freelist vaut %s\n", (char *)freelist);
		//searchinfreelist renvoie le pointeur de l'emplacement avec le header modifié ou null
 		void *adresseRecyclage=searchinfreelist(size);
		//si un emplacement est trouvé
 		if(adresseRecyclage){
 			adresseSansMeta=HEADER_SIZE+adresseRecyclage;
 			return adresseSansMeta;
 		}
 		
 	}
	//a voir avec Mr Tigli*************************
 	if(adressePlaceRes==NULL){
 	//	printf("dans AdressePlaceRes\n" );
 		adressePlaceRes=sbrk(0);
 		if(sbrk(SIZE_FOR_SBRK) ==((void*)-1)){
		// on renvoit un NULL
 			printf("premier sbrk mort");
 			return NULL;
 		}
 		
 		nb_sbrk++;
 		placeReserve=SIZE_FOR_SBRK;
 	}
 //	printf("je passe dans un second implement\n" );
		// pas assez de place un sbrk a faire
 	if(placeReserve<(HEADER_SIZE+size)){
 		//printf("pas assez de place\n" );

 		if(sbrk(SIZE_FOR_SBRK) ==((void*)-1)){
 			 printf("second sbrk mort");

		// on renvoit un NULL
 			return NULL;
 		}
 		nb_sbrk++;
 		placeReserve+=SIZE_FOR_SBRK;
 	// 	nouvelleAdresse=adressePlaceRes;
 	// 	creationHeader(nouvelleAdresse,size);
		// //je me place après tout
 	// 	adressePlaceRes=sbrk(0);
 	// 	adresseSansMeta=nouvelleAdresse+HEADER_SIZE;
 	// 	return adresseSansMeta;
		//assez de place 
 	}
 		//printf("assez de place\n" );

 		placeReserve-=(HEADER_SIZE+size);
 		nouvelleAdresse=adressePlaceRes;
 		creationHeader(nouvelleAdresse,size);
 		adressePlaceRes+=(HEADER_SIZE+size);
 		adresseSansMeta=nouvelleAdresse+HEADER_SIZE;
 		return adresseSansMeta;

 	
 }

/**
 * @see free(3)
 * myfree libere la zone pointée par p afin qu'elle soit réutilisable 
 * par un futur mymalloc dans le programmme; 
 * après cet appel p est invalide (mais pas null 
 * en fait sa valeur n'est pas modifiée).
 * Pour pouvoir appeler myfree, p doit avoir une valeur qui est le resultat 
 * d'un précédent mymalloc
 * 
 * Lorsque la mémoire est rendue au systeme, 
 * on se contente de remettre le bloc dans la liste des blocs libres.
 * Bien sûr, si ce bloc est contigu avec d'anciens blocs libre, il sera fusionné
 * 
 */
 void  myfree(void *ptr)
 {
 	nb_dealloc += 1;
 	Header aLiberer = ptr-HEADER_SIZE;
	//premier free
 	if(freelist==NULL){
 		freelist=aLiberer;
		//liste circulaire j'affecte au premier et dernier element l'adresse du premier donc lui meme
 		aLiberer->info.next=aLiberer;
					//printf("je suis le seul element de la freelist");

 	}else{
		//chercher le bloc inferieur le plus proche
		//initialisation
 		Header headerCourant=freelist;
 		//cas premier dans la liste
 		if(headerCourant>aLiberer){
 			freelist=aLiberer;
 			aLiberer->info.next=headerCourant;
 			if(headerCourant->info.next==headerCourant){
 				headerCourant->info.next=freelist;
 			}else{
 			Header headerPrecedent=headerCourant;
 			void * next= headerPrecedent->info.next;
 			
 		while(next!=headerCourant){
		headerPrecedent=headerPrecedent->info.next;
		next=headerPrecedent->info.next;
		}

		headerPrecedent->info.next=freelist;
		}

 		}else{
 		Header headerSuivant=headerCourant->info.next;
 		do{
			//si on est au bon endroit
 			if(headerCourant<aLiberer && headerSuivant>aLiberer){
 				aLiberer->info.next=headerSuivant;
 				headerCourant->info.next=aLiberer;
				//printf("je suis entre deux blocs\n");
 				break;
			}else//on avance dans la liste
			{
				headerCourant=headerSuivant;
				headerSuivant=headerSuivant->info.next;
			}
			
		}while(headerSuivant!=freelist);

		//si c'est a NULL c'est qu'on a fait le while 
		//jusqu'a la fin et que notre nouveau bloc free doit être ajouté à la fin de la liste
		if(aLiberer->info.next==NULL){
			headerCourant->info.next=aLiberer;
			aLiberer->info.next=freelist;
			//printf("je suis le dernier donc je passe pas par la double condition\n");

		}
}
		//verifier pour fusionner les blocs
		fusionBlocs();
		Header courant=freelist;
		//printf("taille du premier bloc:%d\n",courant->info.size);

	}
	//if(aLiberer->info.next==freelist)
			//printf("j'ai la freelist en next");
}

/**
 * @see calloc(3)
 */
 void *mycalloc(size_t nmemb, size_t size)
 {
 	size_t tailleTotale=nmemb*size;
 	void * pointeurZone=mymalloc(tailleTotale);
 	
 	for(int i=0;i<tailleTotale;i++){
 		int *zoneaZero;
 		zoneaZero=pointeurZone+i;
 		*zoneaZero=0;
 		printf("%d",*zoneaZero);


 	}
 	return pointeurZone;

 }

/**
* @see realloc(3)
* realloc() modifie la taille du bloc de mémoire pointé par ptr 
* pour l'amener à une taille de size octets. realloc() conserve le 
* contenu de la zone mémoire minimum entre la nouvelle et l'ancienne taille. 
* Le contenu de la zone de mémoire nouvellement allouée n'est pas initialisé. 
* Si ptr est NULL, l'appel est équivalent à malloc(size), pour toute valeur de size. 				FAIT 
* Si size vaut zéro, et ptr n'est pas NULL, l'appel est équivalent à free(ptr). 					FAIT
* Si ptr n'est pas NULL, il doit avoir été obtenu par un appel antérieur à malloc(), calloc() ou realloc(). 
* Si la zone pointée était déplacée, un free(ptr) est effectué.   
*/
void *myrealloc(void *ptr, size_t size)
{
	if (!ptr) {
		return (mymalloc(size));
	}
	else {
		if(size==0)
		{
			myfree(ptr);
		}
	}
	return NULL;
}

#ifdef MALLOC_DBG
void mymalloc_infos(char *str){
	// if(str) printf("**************** %s ****************\n",str);

	// printf( "# allocs = %3d - # deallocs = %3d - # sbrk = %3d\n",nb_alloc, nb_dealloc, nb_sbrk);

	// if (str) printf(stderr, "****************\n\n");
	 if (str) fprintf(stderr, "**********\n*** %s\n", str);

    fprintf(stderr, "# allocs = %3d - # deallocs = %3d - # sbrk = %3d\n",
        nb_alloc, nb_dealloc, nb_sbrk);
    /* Ca pourrait être pas mal d'afficher ici les blocs dans la liste libre */
    if (freelist)
    {Header iBlock;
        for (iBlock = freelist; iBlock->info.next!= freelist; iBlock=iBlock->info.next )
        {
            fprintf(stderr, "Block @0x%d (size=%d, next 0x%d)\n", iBlock, iBlock->info.size, iBlock->info.next);
        }
            fprintf(stderr, "Block @0x%d (size=%d, next 0x%d)\n", iBlock, iBlock->info.size, iBlock->info.next);

    }
}
#endif
