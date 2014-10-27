#define __KMA_TEST_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/************Private include**********************************************/
#include "kma_page.h"
#include "kma.h"


/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

enum REQ_STATE
  {
    FREE,
    USED
  };

typedef struct mem
{
  int size;
  void* ptr;
  void* value; // to check correctness
  enum REQ_STATE state;
} mem_t;

/************Global Variables*********************************************/

/************Function Prototypes******************************************/
void testInitPage();
void testGetAmountOfMemoryToRequest();
void testGetFreeBufferIndex();
void testGetFreeBufferPointer();
void testSetBitmask();
void testUnsetBitmask();
void testGetMemoryPointer();
void testCoalesceFreeMemory();
void testSplitUntil();


/************External Declaration*****************************************/

/**************Implementation***********************************************/
int
main(int argc, char* argv[])
{
  printf("Unit testing kma_bud\n");  

  testGetFreeBufferIndex();

  return 0;
}

void testInitPage()
{
}

void testGetAmountOfMemoryToRequest()
{
}

void testGetFreeBufferIndex()
{
  printf("Failed\n");
}

void testGetFreeBufferPointer()
{
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

