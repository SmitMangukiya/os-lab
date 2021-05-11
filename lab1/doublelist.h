/*************************************************
 * This file provides class definitions and
 * function prototypes for a basic linked list
 * of doubles.
 *
 * You may NOT modify this file
 *************************************************/
#ifndef _DOUBLELIST_H
#define _DOUBLELIST_H

typedef struct node
{
  double data;
  struct node *next;
} ListNode;

/* void addToHead( double v, ListNode **L );
 *
 * Adds value 'v' to the head of the List 'L'
 *
 * INPUT:  double     v: the value to be added to the list
 *         ListNode **L: a pointer to the linked list head pointer
 *         
 * OUTPUT: None
 *
 */
void addToHead (double v, ListNode **L);




/* printList( ListNode *L ) prints the 'data' member of all the nodes in list L 
 * 
 * INPUT:  ListNode *L: the linked list head pointer
 *
 * OUTPUT: None
 */
void printList (ListNode * L);




/* deleteList( ListNode **L ) deletes all the nodes in list L 
 * 
 * INPUT:  ListNode **L: a pointer to the linked list head pointer
 *
 * OUTPUT: None
 */
void deleteList (ListNode **L);

#endif
