#ifndef LIN_EQ_SOLVER_H
#define LIN_EQ_SOLVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_DIM 20

typedef struct
{
  //properties set by the user
  long double matrix[MAX_DIM][MAX_DIM];
  long double vector[MAX_DIM];
  int dim;
  //properties determined by the solver
  long double inv_matrix[MAX_DIM][MAX_DIM];//inverse of matrix specified above
  long double solution[MAX_DIM];
}lin_eq_type;

int solve_lin_eq(lin_eq_type *lin_eq);
long double det(int m, lin_eq_type *lin_eq);
int get_inv(lin_eq_type *lin_eq);

#endif
