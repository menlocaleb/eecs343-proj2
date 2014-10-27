#ifndef __KMABUD_H__
#define __KMABUD_H__

/************System include***********************************************/

/************Private include**********************************************/

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

#define MIN_POWER 4
#define MAX_POWER 13 // assumes pagesize is 8192
#define MIN_BUFFER_SIZE 16
#define NUM_BUFFER_SIZES 4 * (MAX_POWER - MIN_POWER + 1)
#define BITMAP_SIZE PAGESIZE/(MIN_BUFFER_SIZE * 8)
#define MAX_BUDDY_SYSTEM_PAGES 20 // arbitrary now

typedef unsigned char BYTE;

typedef struct bitmap_and_page_data
{
  kma_page_t* pageData;
  BYTE bitmap[BITMAP_SIZE];
} pageControlStruct_t;

typedef struct free_list
{ 
  void* freeBufferList[NUM_BUFFER_SIZES];
} freeList_t;

typedef struct buddy_system_struct
{
  freeList_t freeList;
  pageControlStruct_t pages[MAX_BUDDY_SYSTEM_PAGES];
} buddySystemStruct_t;

typedef struct buffer_data
{
  void* nextFreeBuffer;
  size_t bufferSize;
} bufferData_t;

#endif /* __KMABUD_H__ */
