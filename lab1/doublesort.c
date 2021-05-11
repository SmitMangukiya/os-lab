// doublesort.c

#include "doublesort.h"

static int maxIndex (double *A, int N);
static int compare (double a, double b);
static void swap (double *a, double *b);

void doublesort (double *A, int N)
{
  int i;

  for (i = N; i > 1; --i)
  {
    int iMax = maxIndex (A, i);

    swap (&A[i - 1], &A[iMax]);
  }
}

int maxIndex (double *A, int N)
{
  int iMax = 0;
  int i;

  for (i = 0; i < N; ++i)
  {
    if (compare (A[iMax], A[i]))
      iMax = i;
  }
  return iMax;
}

int compare (double a, double b)
{
  return a < b;
}

void swap (double *a, double *b)
{
  double t = *a;

  *a = *b;
  *b = t;
}
