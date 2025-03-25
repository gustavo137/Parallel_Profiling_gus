#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "arraymalloc.h"
#include "boundary.h"
#include "jacobi.h"
#include "cfdio.h"

int main(int argc, char **argv)
{
  int printfreq=1000; //output frequency
  double error, bnorm;
  double tolerance=0.0; //tolerance for convergence. <=0 means do not check

  //main arrays
  double **psi;
  //temporary versions of main arrays
  double **psitmp;

  //command line arguments
  int scalefactor, numiter;

  double re; // Reynold's number - must be less than 3.7

  //simulation sizes
  int bbase=10;
  int hbase=15;
  int wbase=5;
  int mbase=32;
  int nbase=32;

  int checkerr = 0;

  int m,n,b,h,w;
  int iter;
  int i,j;

  double tstart, tstop, ttot, titer;

  //do we stop because of tolerance?
  if (tolerance > 0)
    {
      checkerr = 1;
    }

  //check command line parameters and parse them

  if (argc <3|| argc >4)
    {
      printf("Usage: cfd <scale> <numiter> [reynolds]\n");
      return 0;
    }

  scalefactor=atoi(argv[1]);
  numiter=atoi(argv[2]);
  re=-1.0;

  if(!checkerr)
    {
      printf("Scale Factor = %i, iterations = %i\n",scalefactor, numiter);
    }
  else
    {
      printf("Scale Factor = %i, iterations = %i, tolerance= %g\n",scalefactor,numiter,tolerance);
    }


  //Calculate b, h & w and m & n
  b = bbase*scalefactor;
  h = hbase*scalefactor;
  w = wbase*scalefactor;
  m = mbase*scalefactor;
  n = nbase*scalefactor;

  re = re / (double)scalefactor;

  printf("Running CFD on %d x %d grid in serial\n",m,n);

  //allocate arrays

  psi    = (double **) arraymalloc2d(m+2,n+2,sizeof(double));
  psitmp = (double **) arraymalloc2d(m+2,n+2,sizeof(double));

  //zero the psi array
  for (i=0;i<m+2;i++)
    {
      for(j=0;j<n+2;j++)
	{
	  psi[i][j]=0.0;
	}
    }
 
  //set the psi boundary conditions

  boundarypsi(psi,m,n,b,h,w);

  //compute normalisation factor for error

  bnorm=0.0;

  for (i=0;i<m+2;i++)
    {
      for (j=0;j<n+2;j++)
	{
	  bnorm += psi[i][j]*psi[i][j];
	}
    }

  bnorm=sqrt(bnorm);

  //begin iterative Jacobi loop

  printf("\nStarting main loop...\n\n");
  
  tstart=gettime();

  for(iter=1;iter<=numiter;iter++)
    {
      //calculate psi for next iteration

	jacobistep(psitmp,psi,m,n);

      //calculate current error if required

      if (checkerr || iter == numiter)
	{
	  error = deltasq(psitmp,psi,m,n);
	  error=sqrt(error);
	  error=error/bnorm;
	}

      //copy back

      for(i=1;i<=m;i++)
	{
	  for(j=1;j<=n;j++)
	    {
	      psi[i][j]=psitmp[i][j];
	    }
	}

      //quit early if we have reached required tolerance

      if (checkerr)
	{
	  if (error < tolerance)
	    {
	      printf("Converged on iteration %d\n",iter);
	      break;
	    }
	}

      //print loop information

      if(iter%printfreq == 0)
	{
	  if (!checkerr)
	    {
	      printf("Completed iteration %d\n",iter);
	    }
	  else
	    {
	      printf("Completed iteration %d, error = %g\n",iter,error);
	    }
	}
    }

  if (iter > numiter) iter=numiter;

  tstop=gettime();

  ttot=tstop-tstart;
  titer=ttot/(double)iter;


  //print out some stats

  printf("\n... finished\n");
  printf("After %d iterations, the error is %g\n",iter,error);
  printf("Time for %d iterations was %g seconds\n",iter,ttot);
  printf("Each iteration took %g seconds\n",titer);
  double sum = 0;
  for(i=1;i<=m;i++)
        {
          for(j=1;j<=n;j++)
            {
              sum=sum+psi[i][j];
            }
        }
  printf("sum : %f",sum);



  //output results

  writedatafiles(psi,m,n, scalefactor);

  writeplotfile(m,n,scalefactor);

  //free un-needed arrays
  free(psi);
  free(psitmp);

  printf("... finished\n");

  return 0;
}
