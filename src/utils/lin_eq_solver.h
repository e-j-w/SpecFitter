#ifndef LIN_EQ_SOLVER_H
#define LIN_EQ_SOLVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_DIM 36 //for specfitter, should be 6 + 3*MAX_FIT_PK

typedef struct
{
  //properties set by the user
  long double matrix[MAX_DIM][MAX_DIM];
  long double mat_weights[MAX_DIM][MAX_DIM];
  long double vector[MAX_DIM];
  uint8_t dim;
  //properties determined by the solver
  long double inv_matrix[MAX_DIM][MAX_DIM];//inverse of matrix specified above
  long double solution[MAX_DIM];
}lin_eq_type;

uint8_t solve_lin_eq(lin_eq_type *lin_eq,int);
uint8_t get_inv(lin_eq_type *lin_eq);

#endif
