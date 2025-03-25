#include <stdio.h>
#include <nvToolsExt.h>
#include "jacobi.h"

void jacobistep_acc(double **psinew, double **psi, int m, int n)
{
  int i, j;
  //nvtxRangeId_t jacc_Id = nvtxRangeStartA("jacobi_acc");
#pragma acc parallel loop collapse(2)
  for(i=1;i<=m;i++)
    {
      for(j=1;j<=n;j++)
	{
	  psinew[i][j]=0.25*(psi[i-1][j]+psi[i+1][j]+psi[i][j-1]+psi[i][j+1]);
        }
    }
  //nvtxRangeEnd(jacc_Id);
}


double deltasq(double **newarr, double **oldarr, int m, int n)
{
  int i, j;

  double dsq=0.0;
  double tmp;

  for(i=1;i<=m;i++)
    {
      for(j=1;j<=n;j++)
	{
	  tmp = newarr[i][j]-oldarr[i][j];
	  dsq += tmp*tmp;
        }
    }

  return dsq;
}
