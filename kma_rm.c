/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the resource map algorithm
 *    Author: Stefan Birrer
 *    Copyright: 2004 Northwestern University
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    Revision 1.2  2009/10/31 21:28:52  jot836
 *    This is the current version of KMA project 3.
 *    It includes:
 *    - the most up-to-date handout (F'09)
 *    - updated skeleton including
 *        file-driven test harness,
 *        trace generator script,
 *        support for evaluating efficiency of algorithm (wasted memory),
 *        gnuplot support for plotting allocation and waste,
 *        set of traces for all students to use (including a makefile and README of the settings),
 *    - different version of the testsuite for use on the submission site, including:
 *        scoreboard Python scripts, which posts the top 5 scores on the course webpage
 *
 *    Revision 1.1  2005/10/24 16:07:09  sbirrer
 *    - skeleton
 *
 *    Revision 1.2  2004/11/05 15:45:56  sbirrer
 *    - added size as a parameter to kma_free
 *
 *    Revision 1.1  2004/11/03 23:04:03  sbirrer
 *    - initial version for the kernel memory allocator project
 *
 ***************************************************************************/
#ifdef KMA_RM
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>

/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */
struct {
	void* next;
	int size;
	void* pre;
	void * my_page;
} freeblock_t;

struct{
	// void* self;
	int num_used;
	freeblock_t * listhead;
} pageheader_t;
/************Global Variables*********************************************/
kma_page_t* pagehead = NULL;
int pages_num = 0;
// page* pagelist;
/************Function Prototypes******************************************/
void* initialize_head(kma_page_t* page);
void add_to_list(void * ptr, int size);
void* fff(int size);

/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{

	if (size +	sizeof(void*)> PAGESIZE)
	{
		return NULL;
	}
	if(!pagehead){
		initialize_head(get_page());
	}
  
  // add a pointer to the page structure at the beginning of the page
  *((kma_page_t**)page->ptr) = page;

  void* where = fff(size);
  header* 

  return NULL;
}

void
kma_free(void* ptr, kma_size_t size)
{
  kma_page_t* page;
  
  page = *((kma_page_t**)(ptr - sizeof(kma_page_t*)));
  
  free_page(page);
}

#endif // KMA_RM


/*******************************/
void initialize_head(kma_page_t* page){
	/*
	|freeblock_t* __
	|num 				  |
	pageheader_t    v________________
	|id        	  |              	 | 
	|ptr________  |                |
	|size       | |________________|
	kma_page_t<--
	*/
	*((kma_page_t**) page->ptr) = page;
	pagehead = page;
	pageheader_t* pghead; // this page's header
	

	pghead = (pageheader_t*) (page->ptr);

	pghead -> listhead = (freeblock_t*) ((long)pghead + sizeof(pageheader_t));

	add_to_list((void*)pghead->listhead, page->size - sizeof(pageheader_t));
	pghead -> num_used =0;
	pages_num++;
}


void add_to_list(void * ptr, int size){
/*
__ _______________ ______________ ________
  |               |              |
__|firstchunk_____|______________|________              
pagehead->ptr

*/
	pageheader_t* firstpage = (pageheader_t*)(pagehead -> ptr);
	//first chunk of the page
	void *first_chunk = (void*)(firstpage -> listhead);
	
	//#OF PAGES BEFORE
	long position = ((long)ptr - (long)firstpage)/PAGEZISE;

	pageheader_t*  to_add_page  = (pageheader_t*)((long) firstpage + position * PAGESIZE) ;
	freeblock_t* to_add = (freeblock_t*) ptr;
	//change it to !voic
	to_add-> my_page = to_add_page; 

	to_add->size = size;
	to_add-> pre= NULL;
	to_add-> next= NULL;

//if it's the first the 
	if(ptr == first_chunk){
		//do nothing
		return;
	}
/*
			|_________|_________|
	^_new block
*/
	else if(ptr < first_chunk){
		//tail of the block
		void* tail = (void*)((long)ptr + size);

		/* |-------|________|_______
			if the two blocks are adjacent and in same page, merge them
		*/
				//>=
		if((tail == first_chunk)
			&& 
			(((*freeblock_t) tail)->mypage == ((freeblock_t*) first_chunk)->mypage)){

			/*merge*/
			
			/*|---|___|->NULL*/
			//DIFFERENT
			if(!((freeblock_t*)(firstpage->listhead)->next)){
				/*?merge the size*/
				int firstsize = ((freeblock_t*)(firstpage->listhead))->size;
				((freeblock_t*) ptr)->size +=  firstsize;
				/*set the next pointer and the listhead */
				((freeblock_t*) ptr)->next = NULL;
				firstpage->listhead = (freeblock_t*) ptr;
			}
			/*|---|___|->|___|...
			   to_  first  next
			*/
			else{
				freeblock_t * the_next = firstpage->header->next;
				the_next->pre = to_add;
				//switch the pointers
				to_add->next = the_next;
				to_add->size = ((freeblock_t*)ptr) ->size + firstpage->listhead->size;  
				firstpage -> listhead = to_add;
			}
		}

		/*
		|-----|->|_____|...
		*/
		else{
			((freeblock_t*)firstpage->heaer)->pre = to_add;
			to_add->next = ((freeblock_t*)firstpage->heaer);
			firstpage->heaer = to_add;
		}
			
	}
	/*
	|____|_____|
	        |
	      to add
	go through the list ot insert
	*/
	else {

		//??
		while ((freeblock_t*)first_chunk) -> next && first_chunk> ptr){
			first_chunk = (void*)(((freeblock_t*)first_chunk)->next);
		}

		/*
		...->|_first____|->...
			^ to_add			
		*/

		// first_chunk = first_chunk->pre;
		int this_size = ((freeblock_t*)first_chunk)->size;
		void *tail = (void*) ((long)first_chunk+this_size);

		//use add
		if(( tail == ptr) && (((freeblock_t*)nextb)->mypage == ((freeblock_t*)ptr)->mypage)){
			((freeblock_t*)first_chunk)->size = this_size+ to_add->size; 
		}

		/*
		|______|->|----|->|____|...
		*/
		else{
			freeblock_t* temp;
			temp = ((freeblock_t*)first_chunk)->next;

			((freeblock_t*)first_chunk)->next = to_add;
			to_add->next = temp; 
			to_add->pre = first_chunk;
		}
	}


}

void* fff(int size){
	if(size < sizeof(freeblock_t))
		size = sizeof(freeblock_t);

	header * mainpage;
	mainpage = (header*) (mainpage->first);

}