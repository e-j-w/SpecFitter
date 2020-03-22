#include "lin_eq_solver.h"

int solve_lin_eq(lin_eq_type * lin_eq)
{

  int i,j;//iterators
  int n=lin_eq->dim;//dimension of the matrix (assume square)
  
  if(get_inv(lin_eq)==0) //compute the inverse matrix
    {
      //printf("Linear equation set has no solutions\n");
      return 0;
    }
  
  //use inverse matrix to find solutions
  memset(lin_eq->solution,0,sizeof(lin_eq->solution));
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
      lin_eq->solution[i]+=lin_eq->inv_matrix[i][j]*lin_eq->vector[j];
  
  return 1;
}

//get the inverse matrix using Gauss-Jordan elimination
int get_inv(lin_eq_type * lin_eq)
{

  int i,j,k,l;//iterators
  int n=lin_eq->dim;//dimension of the matrix (assume square) 
  long double s;//storage variable

  //allocate the identity matrix to be transformed to the inverse
  long double id[MAX_DIM][MAX_DIM];//stack overflow risk?
  memset(id,0,sizeof(id));
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
      if(i==j)
        id[i][j]=1.0L;
        
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
                  
                  s=id[i][k];
                  id[i][k]=id[j][k];
                  id[j][k]=s;
                }
              s=1.0L/lin_eq->inv_matrix[i][i];
              for(k=0;k<n;k++)
                {
                  lin_eq->inv_matrix[i][k]=s*lin_eq->inv_matrix[i][k];
                  id[i][k]=s*id[i][k];
                }
              for(k=0;k<n;k++)
                if(k!=i)
                  {
                    s=-1.0L*lin_eq->inv_matrix[k][i];
                    for(l=0;l<n;l++)
                      {
                        lin_eq->inv_matrix[k][l]=lin_eq->inv_matrix[k][l] + s*lin_eq->inv_matrix[i][l];
                        id[k][l]=id[k][l] + s*id[i][l];
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
  
  memcpy(lin_eq->inv_matrix,id,sizeof(id));
  
  //print the inverse matrix
  /*printf("\n");
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
     printf("inverse[%i][%i] = %0.6LE\n",i,j,lin_eq->inv_matrix[i][j]);*/

  return 1;

}
