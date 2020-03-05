//function returns chisq evaluated for the current fit
double getFitChisq(){
  int i,j;
  double chisq = 0.;
  double f;
  float yval,xval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i + (drawing.contractFactor/2.); //central value of bin
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);
    //background term
    f = fitpar.fitParVal[0] + fitpar.fitParVal[1]*xval + fitpar.fitParVal[2]*xval*xval;
    //gaussian(s)
    for(j=0;j<fitpar.numFitPeaks;j++){
      f += fitpar.fitParVal[6+(3*j)]*exp(-0.5* pow((xval-fitpar.fitParVal[6+(3*j)+1]),2.0)/(2.0*fitpar.fitParVal[6+(3*j)+2]*fitpar.fitParVal[6+(3*j)+2]));
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
int fitLinearStage(){
  int i,j;
  lin_eq_type linEq;
  linEq.dim=3 + fitpar.numFitPeaks;

  //setup linear equations
  for(i=0;i<linEq.dim;i++){
    for(j=0;j<linEq.dim;j++){
      linEq.matrix[i][j] = fitpar.sums[i][j];
    }
    linEq.vector[i] = fitpar.ysums[i];
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

  int i;

  //setup sums for linearized part of fit
  float xval,yval;
  memset(fitpar.sums,0,sizeof(fitpar.sums));
  memset(fitpar.ysums,0,sizeof(fitpar.ysums));
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i + (drawing.contractFactor/2.); //central value of bin
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);
    fitpar.sums[0][0] += 1.;
    fitpar.sums[0][1] += xval;
    fitpar.sums[0][2] += xval*xval;
    fitpar.sums[1][2] += xval*xval*xval;
    fitpar.sums[2][2] += xval*xval*xval*xval;
    fitpar.ysums[0] += yval;
    fitpar.ysums[1] += yval*xval;
    fitpar.ysums[2] += yval*xval*xval;
  }
  fitpar.sums[1][0] = fitpar.sums[0][1];
  fitpar.sums[2][0] = fitpar.sums[0][2];
  fitpar.sums[1][1] = fitpar.sums[0][2];
  fitpar.sums[2][1] = fitpar.sums[1][2];
  
  return 1;
}