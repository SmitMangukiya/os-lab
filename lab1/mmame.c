// mmame.c
// IT215 Systems Software, Winter 2018
// Lab 5

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "doublesort.h"
#include "doublelist.h"

int main( int argc, char *argv[] )
{
  double value;			// a value read from stdin
  int n = 0;			// the number of values read, or counted

  double sum = 0;		// a running sum of the values  
  double min, max;		// the statistical quantities we will be measuring
  double ave, med;

  ListNode *L = NULL;		// List of values, for sorting
  double *A;			// Array for storing the sorted list
  ListNode *pnode;		// pointer for traversing the list

  int fd;			// file descriptor for dup'ing stdin
  int redirected = 0;           // flag indicating if we're reading from redirected stdin
  
  if( argc > 1 )
  {
    fd = dup( STDIN_FILENO );        // save stdin
    freopen( argv[1], "r", stdin );  // redirect stdin to read from the named file
    redirected = 1;
  }
  
  while( 1 )
  {
    // read a value until EOF is reached
    // (actual EOF in the case of reading from a file, or ^D)
    if( EOF == fscanf (stdin, "%lf", &value) ) 
    {
      if( redirected ) // we were reading from a file
      {
        redirected = 0;           // reset our flag
        dup2( fd, STDIN_FILENO ); // restore stdin
        continue;                 // ... read more from stdin
      }
      else break;                 // User entered ^D
    }
    else // read a value
    {
      n += 1;			// new value read: increment our count
      sum = sum + value;	// update the running sum
      if (n == 1)		// first value is a special case that
        min = max = value;	//   initializes min and max,
      else			//   otherwise, update min and max
      {
        if (value < min) min = value;
        else if (value > max) max = value;
      }
      // Add this value to our linked list:
      addToHead (value, &L);
    }
  }
 
  // There must be at least one input value
  if (n > 0)
  {
    // Compute the average:
    ave = sum / n;

    // Allocate an array, for sorting
    A = (double *) calloc (n, sizeof (double));

    // Copy our linked list into our array
    pnode = L;
    n = 0;
    while (pnode != NULL)
    {
      A[n] = pnode->data;
      pnode = pnode->next;
      n += 1;
    }

    // Sort the array
    doublesort (A, n);

    // Find the median
    if (n % 2)
      med = A[n / 2];
    else
      med = 0.5 * (A[n / 2] + A[n / 2 - 1]);
    
    // Print our results:
    printf ("%.3f\t%.3f\t%.3f\t%.3f\n", min, max, ave, med);

    // De-allocate dynamically allocated memory
    free (A);
    deleteList (&L);
  }

  return 0;
}
