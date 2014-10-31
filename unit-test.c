#define __KMA_TEST_IMPL__

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

/************Function Prototypes******************************************/
void testInitPage();
void testGetAmountOfMemoryToRequest();
void testGetFreeBufferIndex();
void testGetFreeBufferPointer();
void testGetPageNumber();
void testGetByteIndex();
void testGetByteOffset();
void testSetBitmap();
void testUnsetBitmap();
void testGetMemoryPointer();
void testCoalesceFreeMemory();
void testSplitUntil();
void expectEqual(int expected, int actual, int testNum);
void expectNotEqual(int expected, int actual, int testNum);
void expectEqualPtr(void* expected, void* actual, int testNum);
void expectNotEqualPtr(void* expected, void* actual, int testNum);
void expectEqualByte(BYTE expected, BYTE actual, int testNum);
void expectNotEqualByte(BYTE expected, BYTE actual, int testNum);

/************External Declaration*****************************************/

extern buddySystemStruct_t* startOfManagedMemory;

void initPage(kma_page_t* startOfNewPage);
int getAmountOfMemoryToRequest(int numOfBytesRequested);
int getFreeBufferIndex(int bufferSize);
void** getFreeBufferPointer(int index);
size_t getPageNumber(void* addressOfStartOfPage); // linear search through page array
size_t getByteIndex(void* addressOfStartOfPage, void* ptr); // index into bitmap array of bytes
size_t getByteOffset(void* addressOfStartOfPage, void* ptr); // number of left shifts for setting/unsetting bit
void setBitmap(void* ptr, int sizeInBytes);
void unsetBitmap(void* ptr, int sizeInBytes);
void alterBitMap(void* ptr, int sizeInBytes, bool setBits);
void* getMemoryPointer(int bufferSize);
void coalesceFreeMemory(void* pointer, int bufferSize);
void* splitUntil(void* freeBuffer, int bufferSize, int desiredBufferSize);
void removeFromFreeList(void* buffer, int bufferSize);
void insertIntoFreeList(void* buffer, int bufferSize);

/**************Implementation***********************************************/
int
main(int argc, char* argv[])
{
  printf("Unit testing kma_bud\n");  
  printf("%d\n", sizeof(buddySystemStruct_t));  

  testGetFreeBufferIndex();
  testGetAmountOfMemoryToRequest();
  testGetPageNumber();
  testGetByteIndex();
  testGetByteOffset();
  testInitPage();
  

  return 0;
}

void testInitPage()
{
  printf("initPage Test\n");
  kma_page_t* page;
  
  // get one page
  page = get_page();
  
  initPage(page); 

  // now we can test values in startOfManagedMemory
  // free list should have all NULL pointers except 4096 which should have a pointer
  int i;
  for (i=0; i < NUM_BUFFER_SIZES; i++)
  {
    if (i != 8)
    {
      expectEqualPtr(NULL, *getFreeBufferPointer(i), i);
    }
    else
    {
      expectNotEqualPtr(NULL, *getFreeBufferPointer(i), i);
    }
  }

  // first page should have pointer to page data, and should have first half of bits in bitmap set
  expectNotEqualPtr(NULL, startOfManagedMemory->pages[0].pageData, NUM_BUFFER_SIZES);
  int j;
  for (j=0; j< BITMAP_SIZE; j++) {
    if (j < BITMAP_SIZE/2)
    {
      expectEqualByte(255 ,startOfManagedMemory->pages[0].bitmap[j], i + j); 
    }  
    else
    {
      expectEqualByte(0, startOfManagedMemory->pages[0].bitmap[j], i+j);
    }
  }

  free_page(page);   
  startOfManagedMemory = NULL; // reset global variable for independent tests
}

void testGetAmountOfMemoryToRequest()
{
  printf("GetAmountOfMemoryToRequest\n");
  
  expectEqual(16, getAmountOfMemoryToRequest(4), 1);
  expectEqual(16, getAmountOfMemoryToRequest(12), 2);

  expectEqual(32, getAmountOfMemoryToRequest(16), 3);
  expectEqual(32, getAmountOfMemoryToRequest(20), 4);
  expectEqual(32, getAmountOfMemoryToRequest(28), 5);

  expectEqual(64, getAmountOfMemoryToRequest(32), 6);
  expectEqual(64, getAmountOfMemoryToRequest(40), 7);

  expectEqual(128, getAmountOfMemoryToRequest(64), 8);
  expectEqual(128, getAmountOfMemoryToRequest(80), 9);

  expectEqual(256, getAmountOfMemoryToRequest(128), 10);
  expectEqual(256, getAmountOfMemoryToRequest(150), 11);

  expectEqual(512, getAmountOfMemoryToRequest(256), 12);
  expectEqual(512, getAmountOfMemoryToRequest(300), 13);
}

void testGetFreeBufferIndex()
{
  printf("GetFreeBufferIndex\n");
  int i = 0;
  expectEqual(0,getFreeBufferIndex(16),i);
  i = i + 1;
  expectEqual(1,getFreeBufferIndex(32),i);
  i = i + 1;
  expectEqual(2,getFreeBufferIndex(64),i);
  i = i + 1;
  expectEqual(3,getFreeBufferIndex(128),i);
  i = i + 1;
  expectEqual(4,getFreeBufferIndex(256),i);
  i = i + 1;
  expectEqual(5,getFreeBufferIndex(512),i);
  i = i + 1;
  expectEqual(6,getFreeBufferIndex(1024),i);
  i = i + 1;
  expectEqual(7,getFreeBufferIndex(2048),i);
  i = i + 1;
  expectEqual(8,getFreeBufferIndex(4096),i);
}

void testGetFreeBufferPointer()
{
}

void testGetPageNumber()
{
  printf("getPageNumber Test\n");
  kma_page_t* page;
  kma_page_t* page2;
  
  // get one page
  page = get_page();
  page2 = get_page();
  
  initPage(page);
  startOfManagedMemory->pages[1].pageData = page2;
   

  size_t pageNum = getPageNumber(page->ptr);
  expectEqual(0, pageNum, 1);

  pageNum = getPageNumber(page2->ptr);
  expectEqual(1, pageNum, 2);

  free_page(page);   
  free_page(page2);
  startOfManagedMemory = NULL; // reset global variable for independent tests
}

void testGetByteIndex()
{
  printf("getByteIndex Test\n");
  kma_page_t* page;
  
  // get one page
  page = get_page();
  
  initPage(page);

  BYTE* startOfPage = (BYTE*) page->ptr;

  void* ptr = (void*) startOfPage;

  size_t byteIndex = getByteIndex((void*) startOfPage, ptr);
  expectEqual(0, byteIndex, 1);

  ptr = (void*) (startOfPage + PAGESIZE/2);

  byteIndex = getByteIndex((void*) startOfPage, ptr);
  expectEqual(BITMAP_SIZE/2, byteIndex, 2);
  
  ptr = (void*) startOfPage + 3*PAGESIZE/4;

  byteIndex = getByteIndex((void*) startOfPage, ptr);
  expectEqual(3*BITMAP_SIZE/4, byteIndex, 3);

  free_page(page);   
  startOfManagedMemory = NULL; // reset global variable for independent tests
}

void testGetByteOffset()
{
  printf("getByteOffset Test\n");
  kma_page_t* page;
  
  // get one page
  page = get_page();
  
  initPage(page);

  BYTE* startOfPage = (BYTE*) page->ptr;

  void* ptr = (void*) startOfPage;

  // offset is number of bits added after byte index (start at left edge of bits in byte)
  size_t byteOffset = getByteOffset((void*) startOfPage, ptr);
  expectEqual(0, byteOffset, 1);

  ptr = (void*) startOfPage + PAGESIZE/2 + MIN_BUFFER_SIZE;

  // offset is number of bits added after byte index (start at left edge of bits in byte)
  byteOffset = getByteOffset((void*) startOfPage, ptr);
  expectEqual(1, byteOffset, 2);
  
  ptr = (void*) startOfPage + 3*PAGESIZE/4 + 2*MIN_BUFFER_SIZE;

  byteOffset = getByteOffset((void*) startOfPage, ptr);
  expectEqual(2, byteOffset, 3);

  free_page(page);   
  startOfManagedMemory = NULL; // reset global variable for independent tests
}

void testSetBitmask()
{
}
void testUnsetBitmask()
{
}

void testGetMemoryPointer()
{
}

void testCoalesceFreeMemory()
{
}

void testSplitUntil()
{
}


void expectEqual(int expected, int actual, int testNum)
{
  if (expected != actual)
  {
    printf("%d != %d : test %d\n", expected, actual, testNum);
  }
}

void expectNotEqual(int expected, int actual, int testNum)
{
  if (expected == actual)
  {
    printf("%d == %d : test %d\n", expected, actual, testNum);
  }
}

void expectEqualPtr(void* expected, void* actual, int testNum)
{
  if (expected != actual)
  {
    printf("%p != %p : test %d\n", expected, actual, testNum);
  }
}

void expectNotEqualPtr(void* expected, void* actual, int testNum)
{
  if (expected == actual)
  {
    printf("%p == %p : test %d\n", expected, actual, testNum);
  }
}

void expectEqualByte(BYTE expected, BYTE actual, int testNum)
{
  if (expected != actual)
  {
    printf("%hhu != %hhu : test %d\n", expected, actual, testNum);
  }
}

void expectNotEqualByte(BYTE expected, BYTE actual, int testNum)
{
  if (expected == actual)
  {
    printf("%hhu == %hhu : test %d\n", expected, actual, testNum);
  }
}
