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

/**
*Creer la metadonnee d'un block
* adresse, le pointeur vers l'adresse ou initialiser la metadonnée,
* size, la taille du block a allouer
*/
void creationHeader(void* adresse,size_t size){
	Header newBlock=adresse;
	newBlock->info.size=size;
	newBlock->info.next=NULL;
	fprintf(stderr, "Block Alloué @0x%d (size=%d, next 0x%d)\n", newBlock, newBlock->info.size, newBlock->info.next);

}

/**
* Recuperer un pointeur sur un block 
* à partir du pointeur sur sa metadonnée
*/
void* enleverMeta(void * pointeur){
	void * adresseSansMeta;
	adresseSansMeta=HEADER_SIZE+pointeur;
	return adresseSansMeta;

}

/**
* Recuperer la métadonnée d'un block 
* à partir de son pointeur
*/
void* recupererMeta(void*pointeur){
	void * adresseMeta;
	adresseMeta=pointeur-HEADER_SIZE;
	return adresseMeta;
}

/**
* Pour obtenir le block precedent à un bloc 
* adresseNext est l'adresse qui doit être 
* stocké dans la valeur next du suivant
*
*/
void * chercherBlocByNext(void * adresseNext){
	Header header=adresseNext;
	void * next= header->info.next;
	while(next!=adresseNext){
		header=header->info.next;
		next=header->info.next;
	}
	return header;
}

/** Permet de fusionner deux blocs 
* le premier parametre possède l'adresse la plus petite,
* le second l'adresse la plus grande
*/
bool fusionBlocs(Header premier,Header plusGrand){

	bool fusion=false;
	void * adressePremier=premier;
	void * adressePlusGrande=plusGrand;
	if((adressePremier+HEADER_SIZE+premier->info.size)==adressePlusGrande){
		premier->info.size+=HEADER_SIZE+plusGrand->info.size;
		if(plusGrand->info.next!=NULL){
			premier->info.next=plusGrand->info.next;

		}
		fusion=true;
		fprintf(stderr, "Block fusionné @0x%d (size=%d, next 0x%d) avec @0x%d \n", premier, premier->info.size, premier->info.next,plusGrand);
	}
	return fusion;
}

void couper(Header theheader,size_t size){

	void * adresseDuBloc=theheader;
	void * adresseNouveauBloc= adresseDuBloc+size+HEADER_SIZE;
	unsigned int sizeNewBloc=theheader->info.size-(HEADER_SIZE+size);
	creationHeader(adresseNouveauBloc,sizeNewBloc);

	theheader->info.size=size;
	Header headerPrecedent= chercherBlocByNext(adresseDuBloc);
	Header nouveauBloc=adresseNouveauBloc;
	nouveauBloc->info.next=theheader->info.next;

	//si l'element etait seul
	if(headerPrecedent==theheader){
		nouveauBloc->info.next=adresseNouveauBloc;
		freelist=nouveauBloc;
	}else{
		headerPrecedent->info.next=adresseNouveauBloc;
	}
	//si l'element etait le premier mais pas le seul
	if(adresseDuBloc==freelist){
		freelist=adresseNouveauBloc;
		nouveauBloc->info.next=theheader->info.next;

	}
}
/**
*searchinfreelist renvoie le pointeur de 
*l'emplacement avec le header modifié ou null
*/
void* searchinfreelist(size_t size){

	void * freeAssezGrand=NULL;
	Header headerPrecedent=chercherBlocByNext(freelist);
	Header headerCourant=freelist;
	void * next= headerPrecedent->info.next;
	//je me positionne : precedent est le dernier, courant est le premier

	do{

		if(headerCourant->info.size>=size){
			printf("taille du bloc : %d\n",headerCourant->info.size );
			printf("taille demandée : %d\n",size );

	// je considere que si la taille n'est pas superieur à la taille demandée
	// + 1 header + 16 je lui donne car sinon le bloc ne sera jamais utilisé et donc inutile
			if(headerCourant->info.size>=(size+HEADER_SIZE+16)){
				printf("je decoupe la case est assez grande\n");
				couper(headerCourant,size);
				return headerCourant;
			}
			else{
				//cas particulier un seul element juste de la taille 
				//la freeliste passe a NULL
				if(headerCourant==freelist && headerCourant->info.next==freelist)
				{
					freelist=NULL;
				}
				//je change freelist
				if(freelist==headerCourant){
					freelist=headerCourant->info.next;
				}
				freeAssezGrand=headerCourant;
				headerPrecedent->info.next=headerCourant->info.next;
				fprintf(stderr, "Block Alloué sans couper @0x%d (Nouvelle Size=%d , next 0x%d)\n", headerCourant, headerCourant->info.size, headerCourant->info.next);

				return freeAssezGrand;
			}
		}
		headerPrecedent=headerCourant;
		headerCourant=headerCourant->info.next;
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
		nb_alloc += 1;
		//blocs potentiels
		if(freelist!=NULL){
			void *adresseRecyclage=searchinfreelist(size);
			//si un emplacement est trouvé
			if(adresseRecyclage){

				return enleverMeta(adresseRecyclage);
			}

		}
		if(adressePlaceRes==NULL){
			adressePlaceRes=sbrk(0);
			if(sbrk(SIZE_FOR_SBRK) ==((void*)-1)){
				printf("premier sbrk mort");
				return NULL;
			}
			printf("sbrk(800)\n");
			nb_sbrk++;
			placeReserve=SIZE_FOR_SBRK;
		}

		// pas assez de place un sbrk a faire
		if(placeReserve<(HEADER_SIZE+size)){
		do{

			if(sbrk(SIZE_FOR_SBRK) ==((void*)-1)){
				printf("second sbrk mort");
				return NULL;
			}
			printf("sbrk(800)\n");
			nb_sbrk++;
			placeReserve+=SIZE_FOR_SBRK;
		}while(placeReserve<HEADER_SIZE+size);
		}

		placeReserve-=(HEADER_SIZE+size);
		nouvelleAdresse=adressePlaceRes;
		creationHeader(nouvelleAdresse,size);
		adressePlaceRes+=(HEADER_SIZE+size);

		return enleverMeta(nouvelleAdresse);


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
		bool add=false;
		nb_dealloc += 1;
		Header aLiberer = recupererMeta(ptr);
		aLiberer->info.next=NULL;
		fprintf(stderr, "Block Liberé @0x%d (size=%d, next 0x%d)\n", aLiberer, aLiberer->info.size, aLiberer->info.next);
		Header headerCourant;
		Header headerSuivant;
		//premier free
		if(!freelist){
			printf("freelist null");
			freelist=aLiberer;
			aLiberer->info.next=freelist;
			add=true;
		}else{
			headerCourant=freelist;
			//aLiberer prend la place de la freelist
			if(aLiberer<headerCourant){
				freelist=aLiberer;
				aLiberer->info.next=headerCourant;
				//un seul element dans la freelist
				if(headerCourant->info.next==headerCourant){
				headerCourant->info.next=aLiberer;
				}else{
					// changement du block next du precedent
					Header precedent=chercherBlocByNext(headerCourant);
					precedent->info.next=aLiberer;
				}
				//on essaye de fusionner les blocks
				fusionBlocs(aLiberer,headerCourant);
				add=true;
			}else{
			headerSuivant=headerCourant->info.next;
			do{
				//on trouve l'emplacement entre deux blocks
				if(headerCourant<aLiberer && aLiberer<headerSuivant){

					if(fusionBlocs(headerCourant,aLiberer)){
						fusionBlocs(headerCourant,headerSuivant);

					}else{
						headerCourant->info.next=aLiberer;
						if(!fusionBlocs(aLiberer,headerSuivant)){
							aLiberer->info.next=headerSuivant;
						}
					}
					add=true;
					break;
				}else{
					headerCourant=headerCourant->info.next;
					headerSuivant=headerSuivant->info.next;
				}
			}while(headerSuivant!=freelist);
			}
		}
		//si add false rajouter à la fin
		if(!add){
			if(!fusionBlocs(headerCourant,aLiberer)){
				headerCourant->info.next=aLiberer;
				aLiberer->info.next=freelist;
			}
		}

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

		Header newBlock = pointeurZone;
    	printf("mycalloc %d, size=%d, next=%d, data=%d\n", newBlock, newBlock->info.size, newBlock->info.next, *(newBlock+HEADER_SIZE+2));
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
				return NULL;
			}else{
        Header Block = ptr - HEADER_SIZE;
        if (Block->info.size<size)// on se deplace
        {                                   
            myfree(ptr);
            void *newData = mymalloc(size);        
            memcpy(newData, ptr, size);
            return newData;
        }
        else // Taille voulu <= à la précédente 
        	//on casse le block si on peut creer deux headers en plus
        {
        	if(Block->info.size<(size+2*HEADER_SIZE)){
            return ptr;
        	}
        	else{
        		couper(Block,size);
        		return Block;
        	}
        }
    }
	}
		
		return NULL;
	}

	#ifdef MALLOC_DBG
	void mymalloc_infos(char *str){

		if(str) printf("\n**************** %s ****************\n\n",str);

		fprintf(stderr, "# allocs = %3d - # deallocs = %3d - # sbrk = %3d\n",
			nb_alloc, nb_dealloc, nb_sbrk);
		/* Ca pourrait être pas mal d'afficher ici les blocs dans la liste libre */
		if (freelist){
			Header iBlock;
			//iBlock = freelist;
			for (iBlock = freelist;iBlock->info.next!= freelist; iBlock=iBlock->info.next)
			{
				printf("Freelist %d",freelist);
				fprintf(stderr, "Block Restant @0x%d (size=%d, next 0x%d)\n", iBlock, iBlock->info.size, iBlock->info.next);
			}
				//iBlock=iBlock->info.next;
				fprintf(stderr, "Block Restant @0x%d (size=%d, next 0x%d)\n", iBlock, iBlock->info.size, iBlock->info.next);
			}
			printf("\n*************************************\n\n");
			//getchar();
		
		//}
	}
	#endif
