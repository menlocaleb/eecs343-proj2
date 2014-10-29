/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the power-of-two free list
 *             algorithm
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
#ifdef KMA_P2FL
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

//basic buffer struct
typedef struct
{
  void* head;
} buffer_t;

//struct for keeping track of requested pages
typedef struct kpage_l_struct
{
    kma_page_t* page; //points to kma_page_t that was returned by getPage();
    struct kpage_l_struct* next;
} kpage_l;    // linked list of pages

//struct for maintaining a list of free buffers of a certain size
typedef struct
{
    int size;   // buffer size
    kpage_l *pagelist;  // points to first page that was used to hold buffers of this size
    int used;   // number of buffers used
    buffer_t * start; // list of free buffers of this.size size
} freelist;

//administrative struct for keeping track of everything
typedef struct
{
  freelist buf16;
  freelist buf32;
  freelist buf64;
  freelist buf128;
  freelist buf256;
  freelist buf512;
  freelist buf1024;
  freelist buf2048;
  freelist buf4096;
  freelist buf8192;
  freelist kpages;   // stores a list of free kpage_l buffers
  int used;
} main_list;

/*

main_list:
    64buf:
        size
        used
        start -> null
        pagelist -> 1
        
    kpages:
        size: sizeof(kpage_l);
        used: 0
        start -> 2 -> 3 -> 4 ->5...
        pagelist: not used

each buffer has a header that points to either next free, or to struct freelist

start page = |0main_list|1 kpage_l|2 kpage_l|3 kpage_l|4 kpage_l|5 kpage_l|6 kpage_l|7 kpage_l|
kpage_page = |8 kpage_l|9 kpage_l|10 kpage_l|11 kpage_l|
64 page = |HDD HDD      HDD | 
when malloced, H points to corresponding freelist and we return pointer to DD
when freed, we get freelist that was pointed to in H
if no more space, special fucntion to getPage and use it ONLY for kpage_l

*/

/************Global Variables*********************************************/
// starting page
kma_page_t* start = NULL;
// vars for tracking performance
//uncomment the next line to record and print out stats
// #endif
/************Function Prototypes******************************************/
//set up admin data
void
setupMainPage(kma_page_t*);
//adds a kpage to its associated freelist
void
addKpageToFreelist(kma_page_t*, freelist*);
//returns a buffer from the freelist
void*
getBufferFromFreelist(freelist*);
//add more buffers to a freelist
void
addBuffersTo(freelist* list);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  /*
    search from lowest to highest size freelist to find one that can
    satisfy the request, then gets a buffer from it
  */
  if(start == NULL){
    start = get_page();
    setupMainPage(start);   
  }

  void* result = NULL;
  main_list* mainlist = (main_list*)start->ptr;
  freelist* request = NULL;

  //consider the header size
  int adjusted = size+sizeof(buffer_t);
  if (adjusted <= 16) {
    request = &mainlist->buf16;
  } else if (adjusted <= 32) {
    request = &mainlist->buf32;
  } else if (adjusted <= 64) {
    request = &mainlist->buf64;
  } else if (adjusted <= 128) {
    request = &mainlist->buf128;
  } else if (adjusted <= 256) {
    request = &mainlist->buf256;
  } else if (adjusted <= 512) {
    request = &mainlist->buf512;
  } else if (adjusted <= 1024) {
    request = &mainlist->buf1024;
  } else if (adjusted <= 2048) {
    request = &mainlist->buf2048;
  } else if (adjusted <= 4096) {
    request = &mainlist->buf4096;
  } else if (adjusted <= 8192) {
    request = &mainlist->buf8192;
  }
  if (request != NULL)
    result = getBufferFromFreelist(request);
  
  return result;
}


void
setupMainPage(kma_page_t* page)
{
  /*
    set up the admin data and initialize stuff. also create buffers for
    holding the kpage_l struct
    */

  main_list* mainlist = (main_list*)page->ptr;

  // initialize all structs of main
  mainlist->buf16 =(freelist){16,NULL,0,NULL};
  mainlist->buf32 =(freelist){32,NULL,0,NULL};
  mainlist->buf64 = (freelist){64,NULL,0,NULL};
  mainlist->buf128 = (freelist){128,NULL,0,NULL};
  mainlist->buf256 = (freelist){256,NULL,0,NULL};
  mainlist->buf512 = (freelist){512,NULL,0,NULL};
  mainlist->buf1024 = (freelist){1024,NULL,0,NULL};
  mainlist->buf2048 = (freelist){2048,NULL,0,NULL};
  mainlist->buf4096 = (freelist){4096,NULL,0,NULL};
  mainlist->buf8192 = (freelist){8192,NULL,0,NULL};
  mainlist->kpages = (freelist){sizeof(kpage_l)+sizeof(buffer_t), NULL, 0, NULL};

  mainlist->used = 0;
  
  // split remaining page into buffers of sizeof(kpage_l)+sizeof(void*) size

  int remaining_size = PAGESIZE - sizeof(main_list);
  int buffer_count = remaining_size / (sizeof(kpage_l)+sizeof(buffer_t));
  void* begin_point = page->ptr + sizeof(main_list);
  buffer_t* buf_head;
  int i;
  for(i = 0; i < buffer_count; i++){
    buf_head = (buffer_t *)(begin_point + i*(sizeof(kpage_l)+sizeof(buffer_t)));
    buf_head->head = mainlist->kpages.start;
    mainlist->kpages.start = buf_head;
  }

  addKpageToFreelist(page,&mainlist->kpages);
}


void
addKpageToFreelist(kma_page_t* page, freelist* list)
{
  /*
    attaches a kpages to its associated freelist
  */
  main_list* mainl = (main_list*)start->ptr;
  kpage_l* kpl = (kpage_l*)getBufferFromFreelist(&mainl->kpages);
  kpl->page = page;
  
  // add kpl to list->pagelist
  // kpl->next = list->pagelist;
  list->pagelist = kpl;
}

void*
getBufferFromFreelist(freelist* list)
{
  /*
    returns a free buffer from the freelist, creates more if none
  */
  
  if (list->start == NULL)
    {
      kma_page_t* page = get_page();

      // split page into buffers of (list->size) size
      // Again, don't know what i'm doing
      int buffer_count = PAGESIZE / list->size;
      int i;
      void* page_start = page->ptr;
      for(i = 0; i < buffer_count; i++){
        buffer_t* buf = (buffer_t*)(page_start + i*list->size);
        buf->head = list->start;
        list->start = buf;
      }
      addKpageToFreelist(page, list);
    }
  // // get buffer from list->start
  // buffer_t* buf = list->start;
  // list->start = (buffer_t*)buf->head;
  // buf->head = (void*)list;
  // list->used++;
  // main_list*mainl = start->ptr;
  // if(list!=&mainl->kpages)mainl->used++;
  // // return pointer to rest of buffer
  // return (void*)buf+sizeof(buffer_t);
}



/***********************************************************************************
 *
 * Free - related
 *
 * ********************************************************************************/
void
kma_free(void* ptr, kma_size_t size)
{
  // /*
  //   return a buffer to the freelist it belongs to
  //   free all pages of that freelist if none of the buffers for it are used
  // */
  // buffer_t*buf = (buffer_t*)((void*)ptr-sizeof(buffer_t));
  // freelist* list = (freelist*)buf->head;
  // buf->head = list->start;
  // list->start = buf;
  // list->used--;

  // if (list->used==0) {
  //   list->start=NULL;

  //   kpage_l*kpage = list->pagelist;
  //   while (kpage != NULL) {
  //     free_page(kpage->page);
  //     kpage = kpage->next;
  //   }
  //   list->pagelist = NULL;
  // }

  // main_list* mainl = (main_list*)start->ptr;
  // mainl->used--;
  // if (mainl->used == 0) {
  //   kpage_l*kpage = mainl->kpages.pagelist;
  //   while (kpage->next!=NULL) {
  //     free_page(kpage->page);
  //     kpage = kpage->next;
  //   }
  
  //   free_page(kpage->page);
  //   start = NULL;
  // }
}

#endif // KMA_P2FL