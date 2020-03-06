//get the value of the fitted gaussian term for a given x value
double evalGaussTerm(int peakNum, double xval){
  return exp(-0.5* pow((xval-fitpar.fitParVal[6+(3*peakNum)+1]),2.0)/(2.0*fitpar.fitParVal[6+(3*peakNum)+2]*fitpar.fitParVal[6+(3*peakNum)+2]));
}

//function returns chisq evaluated for the current fit
double getFitChisq(){
  int i,j;
  double chisq = 0.;
  double f;
  double yval,xval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i + (drawing.contractFactor/2.); //central value of bin
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);
    //background term
    f = fitpar.fitParVal[0] + fitpar.fitParVal[1]*xval + fitpar.fitParVal[2]*xval*xval;
    //gaussian(s)
    for(j=0;j<fitpar.numFitPeaks;j++){
      f += fitpar.fitParVal[6+(3*j)]*evalGaussTerm(j,xval);
    }
    //pearson chisq
    if(f!=0.){
      chisq += pow(yval-f,2.0)/f;
    }
  }
  return chisq;
}

//Linear fitting stage
//Used to fit linear terms ie. background parameters and peak amplitudes
//Chisq minimized by finding set of parameters for which d chisq/d parameter = 0 for all parameters  
int fitLinearStage(lin_eq_type *linEq){
  
  //solve system of equations and assign values
	if(!(solve_lin_eq(linEq)==1))
		{
      printf("Failed fit - linear stage.\n");
			return 0;
		}

  return 1;
}

//Non-linear fitting stage
//Used to fit non-linear terms ie. peak positions and widths
//chisq minimized wrt these parameters using an iterative approach
int fitNonLinearStage(){
  return 1;
}

int setupFit(){

  int i,j,k;

  lin_eq_type linEq;
  linEq.dim = 3 + fitpar.numFitPeaks;

  //setup sums for linearized part of fit
  double xval,yval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i + (drawing.contractFactor/2.); //central value of bin
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);
    linEq.matrix[0][0] += 1.;
    linEq.matrix[0][1] += xval;
    linEq.matrix[0][2] += xval*xval;
    linEq.matrix[1][2] += xval*xval*xval;
    linEq.matrix[2][2] += xval*xval*xval*xval;
    linEq.vector[0] += yval;
    linEq.vector[1] += yval*xval;
    linEq.vector[2] += yval*xval*xval;
    for(j=0;j<fitpar.numFitPeaks;j++){
      linEq.matrix[0][j+3] += evalGaussTerm(j,xval);
      linEq.matrix[1][j+3] += xval*evalGaussTerm(j,xval);
      linEq.matrix[2][j+3] += xval*xval*evalGaussTerm(j,xval);
      for(k=0;k<fitpar.numFitPeaks;k++){
        linEq.matrix[k+3][j+3] = evalGaussTerm(j,xval)*evalGaussTerm(k,xval);
      }
      linEq.vector[j+3] += yval*evalGaussTerm(j,xval);
    }
  }
  linEq.matrix[1][0] = linEq.matrix[0][1];
  linEq.matrix[2][0] = linEq.matrix[0][2];
  linEq.matrix[1][1] = linEq.matrix[0][2];
  linEq.matrix[2][1] = linEq.matrix[1][2];
  for(i=0;i<fitpar.numFitPeaks;i++){
    linEq.matrix[j+3][0] = linEq.matrix[0][j+3];
    linEq.matrix[j+3][1] = linEq.matrix[1][j+3];
    linEq.matrix[j+3][2] = linEq.matrix[2][j+3];
  }
  
  return 1;
}