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

//buffer struct
typedef struct
{
  void* head;
} buffer_t;

//pages list
typedef struct page_t_struct
{
    kma_page_t* self; // self
    struct page_t_struct* next;
} page_t;    // linked list of pages

//struct for maintaining a list of free buffers of a certain size
typedef struct
{
    int size;   // buffer size
    page_t *pagelist;  // points to first page that was used to hold buffers of this size
    int used;   // number of buffers used
    buffer_t * start; // list of free buffers of this.size size
} freelist_t;

//administrative struct for keeping track of everything
typedef struct
{
  freelist_t buf16;
  freelist_t buf32;
  freelist_t buf64;
  freelist_t buf128;
  freelist_t buf256;
  freelist_t buf512;
  freelist_t buf1024;
  freelist_t buf2048;
  freelist_t buf4096;
  freelist_t buf8192;
  freelist_t kpages;   // stores a list of free page_t buffers
  int used;
} ls_col;

/************Global Variables*********************************************/
// page entry
kma_page_t* page_entry = NULL; 
/************Function Prototypes******************************************/
//set first page
void initialize_first_p(kma_page_t*);
//add the new got page to freelist_t
void add_page_to_ls(kma_page_t*, freelist_t*);
//allocate a buffer from the list
void* get_free_buf(freelist_t*);
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void* kma_malloc(kma_size_t size)
{
  // if initial initialize the firs tpage/main page
  if(page_entry == NULL){
    page_entry = get_page();
    initialize_first_p(page_entry);   
  }

  void* free_buf = NULL;
  ls_col* my_free_col = (ls_col*)page_entry->ptr;
  freelist_t* target_ls = NULL;

  //consider the header size
  int round_up = size+sizeof(buffer_t);

  //locate the correct list

  if (round_up <= 16) {
    target_ls = &my_free_col->buf16;
  } else if (round_up <= 32) {
    target_ls = &my_free_col->buf32;
  } else if (round_up <= 64) {
    target_ls = &my_free_col->buf64;
  } else if (round_up <= 128) {
    target_ls = &my_free_col->buf128;
  } else if (round_up <= 256) {
    target_ls = &my_free_col->buf256;
  } else if (round_up <= 512) {
    target_ls = &my_free_col->buf512;
  } else if (round_up <= 1024) {
    target_ls = &my_free_col->buf1024;
  } else if (round_up <= 2048) {
    target_ls = &my_free_col->buf2048;
  } else if (round_up <= 4096) {
    target_ls = &my_free_col->buf4096;
  } else if (round_up <= 8192) {
    target_ls = &my_free_col->buf8192;
  }
  if (target_ls != NULL)
    free_buf = get_free_buf(target_ls);
  
  return free_buf;
}


//initilize everything including the guiding entry
void initialize_first_p(kma_page_t* page)
{
  
  ls_col* my_free_col = (ls_col*)page->ptr;

  // initialize all structs of main
  my_free_col->buf16 =(freelist_t){16,NULL,0,NULL};
  my_free_col->buf32 =(freelist_t){32,NULL,0,NULL};
  my_free_col->buf64 = (freelist_t){64,NULL,0,NULL};
  my_free_col->buf128 = (freelist_t){128,NULL,0,NULL};
  my_free_col->buf256 = (freelist_t){256,NULL,0,NULL};
  my_free_col->buf512 = (freelist_t){512,NULL,0,NULL};
  my_free_col->buf1024 = (freelist_t){1024,NULL,0,NULL};
  my_free_col->buf2048 = (freelist_t){2048,NULL,0,NULL};
  my_free_col->buf4096 = (freelist_t){4096,NULL,0,NULL};
  my_free_col->buf8192 = (freelist_t){8192,NULL,0,NULL};

  my_free_col->kpages = (freelist_t){sizeof(page_t)+sizeof(buffer_t), NULL, 0, NULL};

  my_free_col->used = 0;
  
  // split remaining page into buffers of sizeof(page_t)+sizeof(void*) size

  int remaining_size = PAGESIZE - sizeof(ls_col);
  int buffer_count = remaining_size / (sizeof(page_t)+sizeof(buffer_t));
  void* begin_point = page->ptr + sizeof(ls_col);
  
  int i;
  for(i = 0; i < buffer_count; i++){
    //set and switch the pointers
    buffer_t* buf_head = (buffer_t *)(begin_point + i*(sizeof(page_t)+sizeof(buffer_t)));
    buf_head->head = my_free_col->kpages.start;
    my_free_col->kpages.start = buf_head;
  }

  add_page_to_ls(page,&my_free_col->kpages);
}


void add_page_to_ls(kma_page_t* page, freelist_t* list)
{
  ls_col* mainl = (ls_col*)page_entry->ptr;
  page_t* to_add = (page_t*)get_free_buf(&mainl->kpages);
  to_add->self = page;
  to_add->next = list->pagelist;
  list->pagelist = to_add;
}

void* get_free_buf(freelist_t* list)
{
  //no such size exists
  if (list->start == NULL)
    {
      kma_page_t* page = get_page();

      // divide page into buffers and initilize each buffer
      int buffer_count = PAGESIZE / list->size;
      int i = 0 ;
      void* page_start = page->ptr;
      for(i = 0; i < buffer_count; i++){
        buffer_t* buf = (buffer_t*)(page_start + i*list->size);
        buf->head = list->start;
        list->start = buf;
      }
      add_page_to_ls(page, list);
    }
  // get buffer from list->start

    /*
    list->start -> |*****|____|____|...
    */

    /*do not iterate through the list, get the first one instead*/
  buffer_t* buf = list->start;
  list->start = (buffer_t*)buf->head;
  buf->head = (void*)list;
  list->used++;
  ls_col* mainl = page_entry->ptr;
  if(list!=&mainl->kpages)mainl->used++;



  return (void*)buf+sizeof(buffer_t);
}

void kma_free(void* ptr, kma_size_t size)
{

  buffer_t*buf = (buffer_t*)((void*)ptr-sizeof(buffer_t));

  //translation to coresponding list
  freelist_t* list = (freelist_t*)buf->head;
  buf->head = list->start;
  list->start = buf;
  list->used--;

//if the list is all available
  if (list->used==0) {
    list->start=NULL;

    page_t*kpage = list->pagelist;
    //set them free
    while (kpage != NULL) {
      free_page(kpage->self);
      kpage = kpage->next;
    }
    list->pagelist = NULL;
  }

  ls_col* lcol = (ls_col*)page_entry->ptr;
  lcol->used--;

  //if the whole system use none
  if (lcol->used == 0) {
    page_t*kpage = lcol->kpages.pagelist;

    //set the free!
    while (kpage->next!=NULL) {
      free_page(kpage->self);
      kpage = kpage->next;
    }
  
    free_page(kpage->self);
    page_entry = NULL;
  }
}

#endif // KMA_P2FL