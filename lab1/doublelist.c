/* Provides an implementation for doublelist.h
 *
 * You may NOT modify this code!
 */
#include <stdio.h>
#include <stdlib.h>
#include "doublelist.h"

void addToHead (double v, ListNode ** L)
{
  ListNode *N = (ListNode *) calloc (1, sizeof (ListNode));

  N->data = v;
  N->next = *L;
  *L = N;
}

void deleteList (ListNode ** L)
{
  while (*L != 0)
  {
    ListNode *N = *L;

    *L = (*L)->next;
    free (N);
  }
}

void printList (ListNode * L)
{
  while (L != 0)
  {
    printf ("%.3f\n", L->data);
    L = L->next;
  }
}
