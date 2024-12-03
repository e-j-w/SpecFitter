#include "lin_eq_solver.h"

uint8_t solve_lin_eq(lin_eq_type *lin_eq, int weighted){

  uint8_t i,j;//iterators
  const uint8_t n=lin_eq->dim;//dimension of the matrix (assume square)

  if(n > MAX_DIM)
    {
      printf("Too many parameters in linear equation.");
      return 0;
    }
  
  if(get_inv(lin_eq)==0) //compute the inverse matrix
    {
      //printf("Linear equation set has no solutions.\n");
      return 0;
    }
  
  //use inverse matrix to find solutions
  memset(lin_eq->solution,0,sizeof(lin_eq->solution));
  if(weighted){
    for(i=0;i<n;i++)
      for(j=0;j<n;j++)
        lin_eq->solution[i]+=lin_eq->inv_matrix[i][j]*lin_eq->vector[j]*lin_eq->mat_weights[i][j];
  }else{
    for(i=0;i<n;i++)
      for(j=0;j<n;j++)
        lin_eq->solution[i]+=lin_eq->inv_matrix[i][j]*lin_eq->vector[j];
  }
  
  return 1;
}

//get the inverse matrix using Gauss-Jordan elimination
uint8_t get_inv(lin_eq_type *lin_eq){

  uint8_t i,j,k,l;//iterators
  const uint8_t n=lin_eq->dim;//dimension of the matrix (assume square) 
  long double s;//storage variable

  //allocate the identity matrix to be transformed to the inverse
  long double *id = calloc(1,MAX_DIM*MAX_DIM*sizeof(long double));
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
      if(i==j)
        id[i*MAX_DIM + j]=1.0L;
        
  memcpy(lin_eq->inv_matrix,lin_eq->matrix,sizeof(lin_eq->matrix));
        
  for(i=0;i<n;i++)
    {
      for(j=i;j<n;j++)
        {
          if(lin_eq->inv_matrix[j][i]!=0.0L)
            {
              for(k=0;k<n;k++)
                {
                  s=lin_eq->inv_matrix[i][k];
                  lin_eq->inv_matrix[i][k]=lin_eq->inv_matrix[j][k];
                  lin_eq->inv_matrix[j][k]=s;
                  
                  s=id[i*MAX_DIM + k];
                  id[i*MAX_DIM + k]=id[j*MAX_DIM + k];
                  id[j*MAX_DIM + k]=s;
                }
              s=1.0L/lin_eq->inv_matrix[i][i];
              for(k=0;k<n;k++)
                {
                  lin_eq->inv_matrix[i][k]=s*lin_eq->inv_matrix[i][k];
                  id[i*MAX_DIM + k]=s*id[i*MAX_DIM + k];
                }
              for(k=0;k<n;k++)
                if(k!=i)
                  {
                    s=-1.0L*lin_eq->inv_matrix[k][i];
                    for(l=0;l<n;l++)
                      {
                        lin_eq->inv_matrix[k][l]=lin_eq->inv_matrix[k][l] + s*lin_eq->inv_matrix[i][l];
                        id[k*MAX_DIM + l]=id[k*MAX_DIM + l] + s*id[i*MAX_DIM + l];
                      }
                  }
            }
          break;
        }
      if(lin_eq->inv_matrix[j][i]==0.0L)
        return 0;//matrix is singular
    }
        
  //print the new identity matrix      
  /*for(i=0;i<n;i++)
    for(j=0;j<n;j++)
     printf("id[%i][%i] = %0.6LE\n",i,j,lin_eq->inv_matrix[i][j]);*/
  
  memcpy(lin_eq->inv_matrix,id,MAX_DIM*MAX_DIM*sizeof(long double));
  
  //print the inverse matrix
  /*printf("\n");
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
     printf("inverse[%i][%i] = %0.6LE\n",i,j,lin_eq->inv_matrix[i][j]);*/
  
  free(id);

  return 1;

}
