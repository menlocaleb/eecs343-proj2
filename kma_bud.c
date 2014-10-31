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
void** getFreeBufferPointer(int index);
size_t getPageNumber(void* addressOfStartOfPage); // linear search through page array
size_t getByteIndex(void* addressOfStartOfPage, void* ptr); // index into bitmap array of bytes
size_t getByteOffset(void* addressOfStartOfPage, void* ptr); // number of left shifts for setting/unsetting bit
void setBitmask(void* ptr, int sizeInBytes);
void unsetBitmask(void* ptr, int sizeInBytes);
void alterBitMask(void* ptr, int sizeInBytes, bool setBits);
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

    // clear or 0 all memory
    int i,j;
    for (i=0; i < NUM_BUFFER_SIZES; i++)
    {
       startOfManagedMemory->freeList.freeBufferList[i] = NULL;
    }
    for (i=0; i < MAX_BUDDY_SYSTEM_PAGES; i++)
    {
      startOfManagedMemory->pages[i].pageData = NULL;
      for (j=0; j < BITMAP_SIZE; j++)
      {
        startOfManagedMemory->pages[i].bitmap[j] = 0;
      }
    } 

    // store pointer to page data in pages array
    startOfManagedMemory->pages[0].pageData = startOfNewPage;

    // need to update bitmap
    setBitmask((void*) startOfManagedMemory, PAGESIZE/2);

    // now need to split block into buffers because used half
    int bufferListIndex = getFreeBufferIndex(PAGESIZE/2);

    void** bufferPtr = getFreeBufferPointer(bufferListIndex);
    // make free list point to first free block
    *bufferPtr = (void*)((char*)startOfManagedMemory + PAGESIZE/2); 
    bufferData_t* bufferData = (bufferData_t*)*bufferPtr;
    bufferData->nextFreeBuffer = NULL;
    bufferData->bufferSize = PAGESIZE/2;
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

// returns pointer to start of free buffer list, not pointer stored at start of free buffer list
void** getFreeBufferPointer(int index)
{
  assert(index >= 0);
  assert(index < NUM_BUFFER_SIZES);

  if (startOfManagedMemory == NULL)
  {
    return NULL;
  }

  return &startOfManagedMemory->freeList.freeBufferList[index];
}

// 0 indexed
size_t getPageNumber(void* addressOfStartOfPage)
{
  size_t i = 0;
  kma_page_t* pageDataPtr = startOfManagedMemory->pages[i].pageData;
  while (pageDataPtr != NULL && pageDataPtr->ptr != addressOfStartOfPage)
  {
    i = i + 1;
    pageDataPtr = startOfManagedMemory->pages[i].pageData;
  }
  if (pageDataPtr == NULL)
  {
    return -1;
  }

  return i;
}

size_t getByteIndex(void* addressOfStartOfPage, void* ptr)
{
  return 0;
}

size_t getByteOffset(void* addressOfStartOfPage, void* ptr)
{
  return 0;
}

void setBitmask(void* ptr, int sizeInBytes)
{
  // calculate proportion of memory to overwrite and offset

  // given number of bits and offset overwrite bits
  // figure out how many fully bytes can overwrite, then move into individual bits
}

void unsetBitmask(void* ptr, int sizeInBytes)
{
}

void alterBitMask(void* ptr, int sizeInBytes, bool setBits)
{
  BYTE byteValue;
  if (setBits)
  {
    byteValue = 255;
  }
  else
  {
    byteValue = 0;
  }

  // get base addr of page
  void* startOfPage = BASEADDR(ptr);
  size_t pageNumber = getPageNumber(startOfPage);
  assert(pageNumber > 0);  

  int numBitsInBitmap = sizeInBytes/MIN_BUFFER_SIZE * 8; 
  size_t bitmapIndex = getByteIndex(startOfPage, ptr);
  size_t bitmapOffset = getByteOffset(startOfPage, ptr);

  BYTE* bitmapLoc = &startOfManagedMemory->pages[pageNumber].bitmap[bitmapIndex];

  int i;
  if (bitmapOffset == 0)
  {
    // if offset is 0 then possible that we can set bytes at a time
    for (i=0; i < numBitsInBitmap/8; i++)
    {
      *bitmapLoc = byteValue;
      bitmapLoc = bitmapLoc + 1;
    }
  }
  // if there is a offset within a byte, then we know numBitsInBitmap < 8 because buffer has offset multiples of size 
  assert(bitmapOffset < 8);

  for (i=bitmapOffset; i < numBitsInBitmap % 8; i++)
  {
    if (setBits)
    {
      *bitmapLoc |= 1 << (7-i);
    }
    else
    {
      *bitmapLoc &= ~(1 << (7-i));
    }
  }
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
