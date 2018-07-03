#define _GNU_SOURCE
#include "SortedList.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>

int temp_errno;

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	if (list == NULL || element == NULL)
		return;
	SortedListElement_t *prev = list;
	SortedListElement_t *cur = list->next;
	while (cur!=list){
		if (strcmp(cur->key,element->key)>0)
			break;
		else{
			prev = cur;
			cur = cur->next;
		}
	}
	if (opt_yield & INSERT_YIELD){
		pthread_yield();
	}
	prev->next = element;
	cur->prev = element;
	element->prev = prev;
	element->next = cur;
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element){
	if (element == NULL || element->key == NULL)
		return 1;
	if (element->prev->next != element || element->next->prev != element)
		return 1;
	if (opt_yield & INSERT_YIELD){
		pthread_yield();
	}
	element->prev->next = element->next;
	element->next->prev = element->prev;
	return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	if(list == NULL)
		return NULL;
	SortedListElement_t *cur = list->next;
	while(cur!=list){
		if (opt_yield & LOOKUP_YIELD){
			pthread_yield();
		}
		if(strcmp(cur->key,key) == 0)
			return cur;
		cur = cur->next;
	}
	return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list){
	if (list == NULL){
		//delete this
		return -1;
	}
	SortedListElement_t *cur = list->next;
	int length = 0;
	while(cur!=list){
		if (opt_yield & LOOKUP_YIELD){
			pthread_yield();
		}
		if (cur->prev->next != cur || cur->next->prev != cur)
			return -1;
		length++;
		cur = cur->next;
	}
	return length;
}

