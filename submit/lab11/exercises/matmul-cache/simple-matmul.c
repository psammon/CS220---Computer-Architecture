#include "matmul.h"

void
matrix_multiply(int n, double a[][n], double b[][n], double c[][n])
{
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      c[i][j] = 0;
      for (int k = 0; k < n; k++) {
	c[i][j] += a[i][k]*b[k][j];
      }
    }
  }
}

