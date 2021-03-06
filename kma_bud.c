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
#include <stdio.h>
#include <string.h>

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
size_t getNextPageNumber(); // linear search for first one with null pageData
size_t getByteIndex(void* addressOfStartOfPage, void* ptr); // index into bitmap array of bytes
size_t getByteOffset(void* addressOfStartOfPage, void* ptr); // number of left shifts for setting/unsetting bit
void* getBuddyPointer(void* startOfPage, void* ptr, int bufferSize);
bool checkIfBitmapSet(void* ptr, int sizeInBytes);
void setBitmap(void* ptr, int sizeInBytes);
void unsetBitmap(void* ptr, int sizeInBytes);
void alterBitMap(void* ptr, int sizeInBytes, bool setBits);
void* getMemoryPointer(int bufferSize);
void coalesceFreeMemory(void** pointer, int* bufferSize);
void* splitUntil(void* freeBuffer, int bufferSize, int desiredBufferSize);
void removeFromFreeList(void* buffer, int bufferSize);
void insertIntoFreeList(void* buffer, int bufferSize);
bool onlyControlStructureLeft();
	
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
    printf("start of page in memory %p\n", page->ptr);
    initPage(page);

  }
  
  // calculate size, plus size of void ptr then rounded up
  int bufferSize = getAmountOfMemoryToRequest(size);
  printf("Size:%d Buffer:%d\n", size, bufferSize);

  // return null if bigger than a page
  if (bufferSize == -1)
  {
    return NULL;
  }

  // request memory of rounded up size and return
  void* buffer = getMemoryPointer(bufferSize);
  printf("Distance from start of page %d\n", buffer - (void*)startOfManagedMemory);
  printf("buffer, returned pointer %p, %p\n", buffer, (void*)(((BYTE*) buffer) + sizeof(bufferData_t)));
  return (void*)(((BYTE*) buffer) + sizeof(bufferData_t));
}

void 
kma_free(void* ptr, kma_size_t size)
{
  void* internalPtr = (void*)(((BYTE*) ptr) - sizeof(bufferData_t));
  int bufferSize = getAmountOfMemoryToRequest(size);
  size_t pageNum = getPageNumber(BASEADDR(internalPtr));
  // unset bitmap
  unsetBitmap(internalPtr, bufferSize);
  // coalesce free buddies
  coalesceFreeMemory(&internalPtr, &bufferSize);
  if (bufferSize == PAGESIZE)
  {
    // can free page
    free_page(startOfManagedMemory->pages[pageNum].pageData);
    // right way would be to shift all pages after this over

    int pageIndex = pageNum + 1;
    while (startOfManagedMemory->pages[pageIndex].pageData != NULL)
    {
      memcpy((void*)&startOfManagedMemory->pages[pageIndex-1], (void*)&startOfManagedMemory->pages[pageIndex], sizeof(pageControlStruct_t*)); 
      pageIndex = pageIndex + 1;
    }

    // so that the last page in list isn't duplicated
    startOfManagedMemory->pages[pageIndex-1].pageData = NULL;

    int j;
    for (j=0; j < BITMAP_SIZE; j++)
    {
      startOfManagedMemory->pages[pageIndex-1].bitmap[j] = 0;
    }
  }

  if (onlyControlStructureLeft())
  {
    // everything gone except control structure
    // free page
    free_page(startOfManagedMemory->pages[0].pageData);
    startOfManagedMemory = NULL;
    return;
  }
  
  if (bufferSize != PAGESIZE)
  {
    // insert into free list
    insertIntoFreeList(internalPtr, bufferSize);
  }
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
    setBitmap((void*) startOfManagedMemory, PAGESIZE/2);

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
  size_t sizeWithLinkedListPtr = numOfBytesRequested + sizeof(bufferData_t);
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

size_t getNextPageNumber()
{
  int i;
  for (i=0; i < MAX_BUDDY_SYSTEM_PAGES; i++)
  {
    if (startOfManagedMemory->pages[i].pageData == NULL)
    {
      return i;
    }
  }
  return -1;
}

size_t getByteIndex(void* addressOfStartOfPage, void* ptr)
{
  int differenceInBytes = ((BYTE*)ptr) - ((BYTE*)addressOfStartOfPage);
  size_t indexToBit = differenceInBytes/MIN_BUFFER_SIZE;
  return indexToBit/8; // divide by 8 to get bytes
}

size_t getByteOffset(void* addressOfStartOfPage, void* ptr)
{
  int differenceInBytes = ((BYTE*)ptr) - ((BYTE*)addressOfStartOfPage);
  size_t indexToBit = differenceInBytes/MIN_BUFFER_SIZE;
  return indexToBit % 8; // mod by 8 to get bit offset
}

void* getBuddyPointer(void* startOfPage, void* ptr, int bufferSize)
{
  int differenceInBytes = ((BYTE*)ptr) - ((BYTE*)startOfPage);
  assert(differenceInBytes % bufferSize == 0);
  bool isLeftBuddy = (differenceInBytes/bufferSize) % 2 == 0;

  if (isLeftBuddy)
  {
    return (void*)((BYTE*)ptr + bufferSize);
  }
  else
  {
    return (void*)((BYTE*)ptr - bufferSize);
  }
}

bool checkIfBitmapSet(void* ptr, int sizeInBytes)
{
  void* startOfPage = BASEADDR(ptr);
  size_t pageNumber = getPageNumber(startOfPage);
  assert(pageNumber >= 0);  

  int numBitsInBitmap = sizeInBytes/MIN_BUFFER_SIZE; 
  size_t bitmapIndex = getByteIndex(startOfPage, ptr);
  size_t bitmapOffset = getByteOffset(startOfPage, ptr);

  BYTE* bitmapLoc = &startOfManagedMemory->pages[pageNumber].bitmap[bitmapIndex];

  int i;
  if (bitmapOffset == 0)
  {
    // if offset is 0 then possible that we can set bytes at a time
    for (i=0; i < numBitsInBitmap/8; i++)
    {
      if (*bitmapLoc != 0)
      {
        return TRUE;
      }
      bitmapLoc = bitmapLoc + 1;
    }
  }
  // if there is a offset within a byte, then we know numBitsInBitmap < 8 because buffer has offset multiples of size 
  assert(bitmapOffset < 8);

  for (i=bitmapOffset; i < numBitsInBitmap % 8; i++)
  {
      if ((*bitmapLoc & 1 << (7-i)) != 0)
      {
        return TRUE;
      }
  }


  // bitmap all 0s, can coalesce!
  return FALSE;
}

void setBitmap(void* ptr, int sizeInBytes)
{
  alterBitMap(ptr, sizeInBytes, TRUE);
}

void unsetBitmap(void* ptr, int sizeInBytes)
{
  alterBitMap(ptr, sizeInBytes, FALSE);
}

void alterBitMap(void* ptr, int sizeInBytes, bool setBits)
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
  assert(pageNumber >= 0);  

  int numBitsInBitmap = sizeInBytes/MIN_BUFFER_SIZE; 
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
      printf("bitmap altered  %hhu\n", *bitmapLoc);
    }
    else
    {
      *bitmapLoc &= ~(1 << (7-i));
    }
  }
}

void* getMemoryPointer(int bufferSize)
{
  // check that free list, if space, set bits, change freeList, and return pointer
  int freeBufferIndex = getFreeBufferIndex(bufferSize);
  void** freeBufferList = getFreeBufferPointer(freeBufferIndex);
  int currentBufferIndex = freeBufferIndex;
  int currentBufferSize = bufferSize;
  while (*freeBufferList == NULL && currentBufferIndex < NUM_BUFFER_SIZES)
  {
    freeBufferList = freeBufferList + 1;
    currentBufferIndex = currentBufferIndex + 1;
    currentBufferSize = currentBufferSize * 2;
  }

  if (currentBufferIndex == NUM_BUFFER_SIZES)
  {
    // need new page
    // add to 8196 free list
    // set freeListPointer to this list
    kma_page_t* page;
    page = get_page();
    size_t nextPage = getNextPageNumber();
    assert(nextPage > 0);
    startOfManagedMemory->pages[nextPage].pageData = page;
    bufferData_t* buffer = (bufferData_t*)page->ptr;
    buffer->nextFreeBuffer = NULL;
    buffer->bufferSize = PAGESIZE;

    currentBufferIndex = currentBufferIndex - 1;
    freeBufferList = getFreeBufferPointer(currentBufferIndex); // 8192 list
    *freeBufferList = page->ptr;
    currentBufferSize = PAGESIZE;
    // now able to continue with below logic

  }

  // have pointer to free list  
  if (currentBufferIndex > freeBufferIndex)
  {
    // need to split
    return splitUntil(*freeBufferList,currentBufferSize, bufferSize);
  }

  // have perfect sized free list
  void* freeBuffer = *freeBufferList;

  // set bitmap
  setBitmap(freeBuffer, bufferSize);

  // take buffer out of free list
  *freeBufferList = ((bufferData_t*)freeBuffer)->nextFreeBuffer;

  // record buffer size
  ((bufferData_t*)freeBuffer)->bufferSize = bufferSize;
  

  return freeBuffer;
}

// call after unset bitmap for this buffer
void coalesceFreeMemory(void** pointer, int* bufferSize)
{
  // calculate buddy location
  void* startOfPage = BASEADDR(*pointer);
  // check if buddy is free
  void* buddyPtr = getBuddyPointer(startOfPage, *pointer, *bufferSize);
  if (!checkIfBitmapSet(buddyPtr,*bufferSize))
  {
    // can coalesce!
    // need to remove it from freeList
    // handle inserting back in in kma_free
    removeFromFreeList(buddyPtr,*bufferSize);
    *bufferSize = *bufferSize*2;
    if (buddyPtr < *pointer)
    {
      *pointer = buddyPtr;
    }

    if (*bufferSize != PAGESIZE)
    {
      coalesceFreeMemory(pointer, bufferSize);
    }
  }
  // base case is either buddy isn't free of full page free
  
}

void* splitUntil(void* freeBuffer, int bufferSize, int desiredBufferSize)
{
  int newBufferSize = bufferSize/2;
  // split and get two pointers
  void* smallerBufferOne = freeBuffer;
  void* smallerBufferTwo = (void*)((BYTE*) freeBuffer + newBufferSize);
  // remove from free list
  removeFromFreeList(freeBuffer, bufferSize);
  // add later pointer to smaller free list
  insertIntoFreeList(smallerBufferTwo, newBufferSize);
  
  // either return or recursively call function
  if (newBufferSize == desiredBufferSize)
  {
    setBitmap(smallerBufferOne, desiredBufferSize);
    bufferData_t* bufferData = (bufferData_t*)smallerBufferOne;
    bufferData->nextFreeBuffer = NULL;
    bufferData->bufferSize = desiredBufferSize;
    
    return smallerBufferOne;
  }
  else
  {
    return splitUntil(smallerBufferOne, newBufferSize, desiredBufferSize);
  }
}

void removeFromFreeList(void* buffer, int bufferSize)
{
  int freeBufferIndex = getFreeBufferIndex(bufferSize);
  void** freeBufferList = getFreeBufferPointer(freeBufferIndex);
  bufferData_t* currentBuffer = (bufferData_t*)*freeBufferList;
  bufferData_t* pastBuffer = NULL;
  while (currentBuffer != NULL && currentBuffer != buffer)
  {
    pastBuffer = currentBuffer;
    currentBuffer = currentBuffer->nextFreeBuffer;
  }

  if (currentBuffer != NULL)
  {
    // found buffer in free list
    if (pastBuffer == NULL)
    {
      // first buffer in list is to be removed
      *freeBufferList = currentBuffer->nextFreeBuffer;
    }
    else
    {
      // pastBuffer now needs to skip input buffer
      pastBuffer->nextFreeBuffer = currentBuffer->nextFreeBuffer;
    }
  }
}

// need to insert into list on same page as buffer
void insertIntoFreeList(void* buffer, int bufferSize)
{
  int freeBufferIndex = getFreeBufferIndex(bufferSize);
  void** freeBufferList = getFreeBufferPointer(freeBufferIndex);
  bufferData_t* currentBuffer = (bufferData_t*)*freeBufferList;
  bufferData_t* pastBuffer = NULL;

  // handle case where list is empty
  if (currentBuffer == NULL)
  {
    *freeBufferList = buffer;
    currentBuffer = (bufferData_t*)*freeBufferList;
    assert(currentBuffer != NULL);
    currentBuffer->nextFreeBuffer = NULL;
    currentBuffer->bufferSize = bufferSize;
    return;
  }

  // now handle case where there are elements in free list
  size_t inputBufferPage = getPageNumber(BASEADDR(buffer));
  size_t currentPageNum = getPageNumber(BASEADDR(currentBuffer));
  assert(currentBuffer != NULL);
  while (currentBuffer->nextFreeBuffer != NULL && currentPageNum < inputBufferPage)
  {
    pastBuffer = currentBuffer;
    currentBuffer = currentBuffer->nextFreeBuffer;
    assert(currentBuffer != NULL);
    currentPageNum = getPageNumber(BASEADDR(currentBuffer));
  }
 
  if (currentBuffer->nextFreeBuffer == NULL)
  {
    // reached end of list, insert at end
    currentBuffer->nextFreeBuffer = buffer;
    currentBuffer = (bufferData_t*)buffer;
    currentBuffer->nextFreeBuffer = NULL;
    currentBuffer->bufferSize = bufferSize;
  }
  else
  {
    // in middle of list but past where we want to input buffer
    if (pastBuffer == NULL)
    {
      // we want to insert buffer at start of free list
      *freeBufferList = buffer;
      ((bufferData_t*)buffer)->nextFreeBuffer = (void*)currentBuffer;
      ((bufferData_t*)buffer)->bufferSize = bufferSize;
    }
    else
    {
      // we want to insert list between pastBuffer and currentBuffer
      assert(pastBuffer != NULL);
      pastBuffer->nextFreeBuffer = buffer;
      ((bufferData_t*)buffer)->nextFreeBuffer = (void*)currentBuffer;
      ((bufferData_t*)buffer)->bufferSize = bufferSize;
    }

  }
}

bool onlyControlStructureLeft()
{
  // only ControlStructure left is all but first half of first page is free
  int i;
  int j;
  // handle first page separately
  for (j=BITMAP_SIZE/2; j < BITMAP_SIZE; j++)
  {
    if (startOfManagedMemory->pages[0].bitmap[j] != 0)
    {
      return FALSE;
    }
  }

  for (i=1; i < MAX_BUDDY_SYSTEM_PAGES; i++)
  {
    for (j=0; j < BITMAP_SIZE; j++)
    {
      if (startOfManagedMemory->pages[i].bitmap[j] != 0)
      {
        return FALSE;
      }
    }
  } 
  return TRUE;
}

#endif // KMA_BUD
