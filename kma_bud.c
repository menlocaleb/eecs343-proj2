/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the buddy algorithm
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
#ifdef KMA_BUD
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>

/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"
#include "kma_bud.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

	

/************Global Variables*********************************************/
buddySystemStruct_t* startOfManagedMemory = NULL;

/************Function Prototypes******************************************/
void initPage(kma_page_t* startOfNewPage);
int getAmountOfMemoryToRequest(int numOfBytesRequested);
int getFreeBufferIndex(int bufferSize);
void* getFreeBufferPointer(int index);
size_t getPageNumber(void* addressOfStartOfPage); // linear search through page array
size_t getByteIndex(void* addressOfStartOfPage, void* ptr); // index into bitmap array of bytes
size_t getByteOffset(void* addressOfStartOfPage, void* ptr); // number of left shifts for setting/unsetting bit
void setBitmask(void* ptr, int sizeInBytes);
void unsetBitmask(void* ptr, int sizeInBytes);
void* getMemoryPointer(int bufferSize);
void coalesceFreeMemory(void* pointer, int bufferSize);
void* splitUntil(int bufferSize);
	
/************External Declaration*****************************************/

/**************Implementation***********************************************/

void*
kma_malloc(kma_size_t size)
{
  if (startOfManagedMemory == NULL)
  {
    // get new page and initialize control struct
    kma_page_t* page;
  
    // get one page
    page = get_page();
    initPage(page);

  }
  
  // calculate size, plus size of void ptr then rounded up

  // return null if bigger than a page

  // request memory of rounded up size and return
  return NULL;
}

void 
kma_free(void* ptr, kma_size_t size)
{
  // unset bitmap
  // coalesce free buddies
  // insert into free list
  // if only control struct left, release page (if not first page?)
  ;
}


void initPage(kma_page_t* startOfNewPage)
{
    // cast pointer to pointer for buddySystemStruct_t
    startOfManagedMemory = (buddySystemStruct_t*)startOfNewPage->ptr;

    // store pointer to page data in pages array
    startOfManagedMemory->pages[0].pageData = startOfNewPage;
}

int getAmountOfMemoryToRequest(int numOfBytesRequested)
{
  size_t sizeWithLinkedListPtr = numOfBytesRequested + sizeof(void*);
  if (sizeWithLinkedListPtr > PAGESIZE)
  {
    return -1; // request is too large
  }
  int bufferSize = 16; // smallest buffer size
  while (bufferSize < sizeWithLinkedListPtr)
  {
    bufferSize <<= 1;
  } 
  return bufferSize;
}

int getFreeBufferIndex(int bufferSize)
{
  if (bufferSize % 2 != 0 || bufferSize > PAGESIZE)
  {
    return -1;
  }
  size_t referenceValue = 16;
  int index = 0;
  while (bufferSize != referenceValue)
  {
    referenceValue = referenceValue * 2;
    index = index + 1;
  }
  return index;
}

void* getFreeBufferPointer(int index)
{
  assert(index >= 0);
  assert(index < NUM_BUFFER_SIZES);

  if (startOfManagedMemory == NULL)
  {
    return NULL;
  }

  return startOfManagedMemory->freeList.freeBufferList[index];
}

void setBitmask(void* ptr, int sizeInBytes)
{
}

void unsetBitmask(void* ptr, int sizeInBytes)
{
}

void* getMemoryPointer(int bufferSize)
{
  return NULL;
}

void coalesceFreeMemory(void* pointer, int bufferSize)
{
}

void* splitUntil(int bufferSize)
{
  return NULL;
}

#endif // KMA_BUD
