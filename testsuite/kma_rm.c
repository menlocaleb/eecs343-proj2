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
typedef struct {
	void* next;
	int size;
	void* pre;
	void * my_page;
} freeblock_t;

typedef struct{
	void* me;
	int num_used;
	int page_count;
	freeblock_t * listhead;
} pageheader_t;
/************Global Variables*********************************************/
kma_page_t* pagehead = NULL;
//int pages_num = 0;
// page* pagelist;
/************Function Prototypes******************************************/
void initialize_head(kma_page_t* page);
void add_to_list(void * ptr, int size);
void* fff(int size);
void remove_node(void* ptr);
void initialize_page(kma_page_t* page);
void freethepage();

/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
//ignore overrequested size
	if(size + sizeof(void*)>PAGESIZE) return NULL;

	
	if(!pagehead) initialize_head(get_page());
	void * final = fff(size);
	pageheader_t* thepage = ((freeblock_t*)final)->my_page;
	thepage->num_used++;
	return final;

}

void
kma_free(void* ptr, kma_size_t size)
{

	add_to_list(ptr,size);
	(((pageheader_t*)(((freeblock_t*)ptr)->my_page))->num_used)--;
	freethepage();
}




/*******************************/
void initialize_head(kma_page_t* page){
	/*
	|freeblock_t* __
	|num 			|
	pageheader_t    v________________
	|id        	  	|              	 | 
	|ptr________  	|                |
	|size       | 	|________________|
	kma_page_t<--
	*/
	*((kma_page_t**) page->ptr) = page;
	pagehead = page;
	pageheader_t* newpghead; // this page's header
	

	newpghead = (pageheader_t*) (page->ptr);

	newpghead->listhead = (freeblock_t*) ((long)newpghead + sizeof(pageheader_t));

	add_to_list(((void*)(newpghead->listhead)), page->size - sizeof(pageheader_t));
	newpghead->num_used =0;
	newpghead->page_count = 0;
}

void initialize_page(kma_page_t* page){

	*((kma_page_t**) page->ptr) = page;
	pageheader_t* newpghead; // this page's header
	

	newpghead = (pageheader_t*) (page->ptr);

	newpghead->listhead = (freeblock_t*)((long)newpghead + sizeof(pageheader_t));

	add_to_list((void*)newpghead->listhead, page->size - sizeof(pageheader_t));
	newpghead->num_used =0;
	newpghead->page_count = 0;
}

void add_to_list(void * ptr, int size){
/*
__ _______________ ______________ ________
  |               |              |
__|firstchunk_____|______________|________              
pagehead->ptr

*/
	pageheader_t* firstpage = (pageheader_t*)(pagehead->ptr);
	//first chunk of the page
	void *first_chunk = (void*)(firstpage->listhead);
	
	//#OF PAGES BEFORE
	long position = ((long)ptr - (long)firstpage)/PAGESIZE;

	pageheader_t*  to_add_page  = (pageheader_t*)((long) firstpage + position * PAGESIZE);
	freeblock_t* to_add = (freeblock_t*)ptr;
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
			(((freeblock_t*)tail)->my_page == ((freeblock_t*)first_chunk)->my_page)){

			/*merge*/
			
			/*|---|___|->NULL*/
			//DIFFERENT
			if(!(((freeblock_t*)(firstpage->listhead))->next)){
				/*?merge the size*/
				int firstsize = ((freeblock_t*)(firstpage->listhead))->size;
				((freeblock_t*)ptr)->size +=  firstsize;
				/*set the next pointer and the listhead */
				((freeblock_t*)ptr)->next = NULL;
				firstpage->listhead = (freeblock_t*)ptr;
			}
			/*|---|___|->|___|...
			   to_  first  next
			*/
			else{
				freeblock_t * the_next = ((freeblock_t*)(firstpage->listhead))->next;
				the_next->pre = ptr;
				//switch the pointers
				((freeblock_t*)ptr)->next = the_next;
				((freeblock_t*)ptr)->size += ((freeblock_t*)(firstpage->listhead))->size;  
				firstpage->listhead = (freeblock_t*)ptr;
			}
		}

		/*
		|-----|->|_____|...
		*/
		else{
			((freeblock_t*)(firstpage->listhead))->pre = to_add;
			to_add->next = ((freeblock_t*)firstpage->listhead);
			firstpage->listhead= (freeblock_t*)ptr;
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
		while (((freeblock_t*)first_chunk)->next){
			 if(first_chunk> ptr) break;
			first_chunk = ((void*)(((freeblock_t*)first_chunk)->next));
		}

		/*
		...->|_first____|->...
			^ to_add			
		*/

		// first_chunk = first_chunk->pre;
		int this_size = ((freeblock_t*)first_chunk)->size;
		void *tail = (void*) ((long)first_chunk+this_size);

		//use add
		if(( tail == ptr) && (((freeblock_t*)tail)->my_page == ((freeblock_t*)ptr)->my_page)){
			((freeblock_t*)first_chunk)->size = this_size+ ((freeblock_t*)ptr)->size; 
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
			if(temp) temp->pre = to_add;
		}
	}

}

void* fff(int size){
	if(size < sizeof(freeblock_t))
		size = sizeof(freeblock_t);

	pageheader_t* firstpage = (pageheader_t*)(pagehead->ptr);
	freeblock_t* interation_block = ((freeblock_t *)(firstpage->listhead));

	while(interation_block){
		if(interation_block ->size < size){
			interation_block = interation_block->next;
			continue;
		}
		if(interation_block->size == size||(interation_block->size-size)< sizeof(freeblock_t)){
			remove_node(interation_block);
			return (void*)interation_block;
		}
		else {
			add_to_list((void*)(long)interation_block+size,interation_block->size-size);
			remove_node(interation_block);
			return (void*)interation_block;		
		}

	}
	
	//didn't find one
	initialize_page(get_page());
	firstpage->page_count++;
	return fff(size);
}

void remove_node(void* ptr){
	freeblock_t* current_block = (freeblock_t*)ptr;
	freeblock_t* next = current_block->next;
	freeblock_t* pre = current_block->pre;

//only header
	if(!next && !pre){
		pageheader_t* firstpage = (pageheader_t*) pagehead->ptr;

		firstpage->listhead = NULL;
		pagehead = NULL;
		return;
	}
//header
	if(!pre){
		pageheader_t* firstpage = (pageheader_t*) pagehead->ptr;
		next->pre = NULL;
		firstpage->listhead = next;
		return;
	}
//tail
	if(!next){
		pre->next = NULL;
		return;
	}
//regular
	next ->pre = pre;
	pre->next = next;
	return;  

}

//free what's not used
void freethepage(){
	pageheader_t* firstpage = (pageheader_t*) (pagehead->ptr);
	pageheader_t* iteration_page;

//start from the last page
	int i = firstpage->page_count;

	bool flag = FALSE;
	do{
			flag = FALSE;
//locate the ith page
		iteration_page = ((pageheader_t*)((long)firstpage + i* PAGESIZE));
		freeblock_t* interation_block = firstpage->listhead;
		
		//if this page is not used
		if(!(pageheader_t*)iteration_page->num_used){
			//delete all the nodes in the list
			while(interation_block){
				freeblock_t* next_block = interation_block->next;
				if(interation_block->my_page == iteration_page) 
					remove_node(interation_block);
				interation_block = next_block;		
			}
			flag = TRUE;//continue
			//till the first page
			if(iteration_page == firstpage){
				pagehead = NULL;
				flag = TRUE;
			}
			free_page(iteration_page->me);
			i--;
			if(pagehead) firstpage->page_count--;
		}
		if(iteration_page == firstpage){
				break;
		}

	}while(flag);

}

#endif // KMA_RM