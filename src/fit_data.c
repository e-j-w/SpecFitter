/* J. Williams, 2020 */

//forward declarations
float getDispSpBinVal(int dispSpNum, int bin);


double getFWHM(double chan, double widthF, double widthG, double widthH){
  return sqrt(widthF*widthF + widthG*widthG*(chan/1000.) + widthH*widthH*(chan/1000.)*(chan/1000.));
}

//get the value of the fitted gaussian term for a given x value
double evalGaussTerm(int peakNum, double xval){
  double evalG = exp(-0.5* pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/(pow(fitpar.fitParVal[8+(3*peakNum)],2.0)));
  //printf("peakNum: %i, xval: %f, pos: %f, width: %f, eval: %f\n",peakNum,xval,fitpar.fitParVal[7+(3*peakNum)],fitpar.fitParVal[8+(3*peakNum)],evalG);
  return evalG;
}

//evaluate the derivative of a gaussian peak term, needed for non-linear fits 
//derPar: 0=amplitude, 1=centroid, 2=width
double evalGaussTermDerivative(int peakNum, double xval, int derPar){
  double evalGDer = evalGaussTerm(peakNum,xval);
  switch (derPar){
  case 2:
    evalGDer *= fitpar.fitParVal[6+(3*peakNum)];
    evalGDer *= pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/pow(fitpar.fitParVal[8+(3*peakNum)],3.0);
    break;
  case 1:
    evalGDer *= fitpar.fitParVal[6+(3*peakNum)];
    evalGDer *= (xval-fitpar.fitParVal[7+(3*peakNum)])/pow(fitpar.fitParVal[8+(3*peakNum)],2.0);
    break;
  case 0:
    break;
  default:
    printf("WARNING: invalid derivative parameter (%i).\n",derPar);
    break;
  }
  return evalGDer;
}

double evalFitBG(double xval){
  return fitpar.fitParVal[0] + xval*fitpar.fitParVal[1] + xval*xval*fitpar.fitParVal[2];
}

double evalFit(double xval){
  int i;
  double val = evalFitBG(xval);
  for(i=0;i<fitpar.numFitPeaks;i++)
    val += fitpar.fitParVal[6+(3*i)]*evalGaussTerm(i,xval);
  return val;
}

double evalFitOnePeak(double xval, int peak){
  if(peak>=fitpar.numFitPeaks)
    return 0.0;
  double val = evalFitBG(xval);
  val += fitpar.fitParVal[6+(3*peak)]*evalGaussTerm(peak,xval);
  return val;
}

double evalPeakArea(int peakNum){
  return fitpar.fitParVal[6+(3*peakNum)]*fitpar.fitParVal[8+(3*peakNum)]*sqrt(2.0*G_PI)/(1.0*drawing.contractFactor);
}

double evalPeakAreaErr(int peakNum){
  double err = (fitpar.fitParErr[6+(3*peakNum)]/fitpar.fitParVal[6+(3*peakNum)])*(fitpar.fitParErr[6+(3*peakNum)]/fitpar.fitParVal[6+(3*peakNum)]);
  err += (fitpar.fitParErr[8+(3*peakNum)]/fitpar.fitParVal[8+(3*peakNum)])*(fitpar.fitParErr[8+(3*peakNum)]/fitpar.fitParVal[8+(3*peakNum)]);
  err = sqrt(err);
  err = err*evalPeakArea(peakNum);
  return err;
}

//function returns chisq evaluated for the current fit
double getFitChisq(){
  int i,j;
  double chisq = 0.;
  double f;
  double yval,xval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i;
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);
    //background term
    f = fitpar.fitParVal[0] + fitpar.fitParVal[1]*xval + fitpar.fitParVal[2]*xval*xval;
    //gaussian(s)
    for(j=0;j<fitpar.numFitPeaks;j++){
      f += fitpar.fitParVal[6+(3*j)]*evalGaussTerm(j,xval);
    }

    //model cannot give less than 0 counts
    /*if(f<0.)
      return BIG_NUMBER;

    //likelihood ratio chisq
    if((f>0.)&&(yval > 0.)){
      chisq += (f - yval + yval*log(yval/f));
    }else{
      chisq += f;
    }*/
    //pearson chisq
    if(f!=0.)
      chisq += (f-yval)*(f-yval)/fabs(f);
    //printf("yval = %f, f = %f, chisq = %f\n",yval,f,chisq);
    //getc(stdin);
  }
  return chisq;
}

int getParameterErrors(lin_eq_type *linEq){

  if(linEq->dim != 3 + (3*fitpar.numFitPeaks)){
    printf("WARNING: trying to get parameter errors without all parameters in use!\n");
  }

  int i;

  //Calculate covariances and uncertainties, see J. Wolberg 
  //'Data Analysis Using the Method of Least Squares' sec 2.5
  int ndf = (int)((fitpar.fitEndCh - fitpar.fitStartCh)/(1.0*drawing.contractFactor)) - (3+(3*fitpar.numFitPeaks));
  for(i=0;i<3;i++)
    fitpar.fitParErr[i]=sqrt((double)(linEq->inv_matrix[i][i]*(getFitChisq()/ndf)));
  for(i=0;i<(3*fitpar.numFitPeaks);i++)
    fitpar.fitParErr[6+i]=sqrt((double)(linEq->inv_matrix[3+i][3+i]*(getFitChisq()/ndf)));

  //add Guassian parameter errors in quadrature against Cramer–Rao lower bounds
  //ie. I'm assuming the errors on the fit parameters and the errors from
  //Poisson statistics are independent
  for(i=0;i<fitpar.numFitPeaks;i++){
    
    //Cramer–Rao lower bound variances
    //(see https://en.wikipedia.org/wiki/Gaussian_function#Gaussian_profile_estimation for an explanation)
    double aCRLB = fabs(3.0*fitpar.fitParVal[6+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[8+(3*i)]));
    double pCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));
    double wCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));

    fitpar.fitParErr[6+(3*i)] = sqrt(fitpar.fitParErr[6+(3*i)]*fitpar.fitParErr[6+(3*i)] + aCRLB);
    fitpar.fitParErr[7+(3*i)] = sqrt(fitpar.fitParErr[7+(3*i)]*fitpar.fitParErr[7+(3*i)] + pCRLB);
    fitpar.fitParErr[8+(3*i)] = sqrt(fitpar.fitParErr[8+(3*i)]*fitpar.fitParErr[8+(3*i)] + wCRLB);

  }

  return 1;
}


//setup sums for the non-linearized fit
//using a CURFIT-like method
//see eq. 2.4.14, 2.4.15, pg. 47 J. Wolberg 
//'Data Analysis Using the Method of Least Squares'
//returns 1 if successful
int setupFitSums(lin_eq_type *linEq, double flambda){

  int i,j,k;
  long double cmatrix[MAX_DIM][MAX_DIM];
  memset(linEq->matrix,0,sizeof(linEq->matrix));
  memset(linEq->vector,0,sizeof(linEq->vector));
  memset(linEq->solution,0,sizeof(linEq->solution));
  memset(linEq->inv_matrix,0,sizeof(linEq->inv_matrix));
  memset(cmatrix,0,sizeof(cmatrix));
  double xval,yval,ydiff;

  linEq->dim = 3 + (3*fitpar.numFitPeaks);

  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = (double)(i);
    yval = getDispSpBinVal(0,i-drawing.lowerLimit);

    if(yval != 0){
      ydiff = yval - evalFit(xval);
      if(yval < 1.)
        yval = 1.;
    
      linEq->matrix[0][0] += 1./yval;
      linEq->matrix[0][1] += xval/yval;
      linEq->matrix[0][2] += xval*xval/yval;
      linEq->matrix[1][2] += xval*xval*xval/yval;
      linEq->matrix[2][2] += xval*xval*xval*xval/yval;
      linEq->vector[0] += ydiff/yval;
      linEq->vector[1] += ydiff*xval/yval;
      linEq->vector[2] += ydiff*xval*xval/yval;
      for(j=3;j<linEq->dim;j++){
        linEq->matrix[0][j] += evalGaussTermDerivative((int)((j-3)/3),xval,j % 3)/yval;
        linEq->matrix[1][j] += xval*evalGaussTermDerivative((int)((j-3)/3),xval,j % 3)/yval;
        linEq->matrix[2][j] += xval*xval*evalGaussTermDerivative((int)((j-3)/3),xval,j % 3)/yval;
        linEq->vector[j] += ydiff*evalGaussTermDerivative((int)((j-3)/3),xval,j % 3)/yval;
        //printf("j: %i, j-3/3: %i, jmod3: %i\n",j,(int)(j-3)/3,j % 3);
        for(k=3;k<linEq->dim;k++){
          linEq->matrix[j][k] += evalGaussTermDerivative((int)((j-3)/3),xval,j % 3)*evalGaussTermDerivative((int)((k-3)/3),xval,k % 3)/yval;
          //printf("yval: %f, j: %i, k: %i, evalgaussder j: %f, evalgaussder k: %f\n",yval,j,k,evalGaussTermDerivative((int)(j-3)/3,xval,j % 3),evalGaussTermDerivative((int)(k-3)/3,xval,k % 3));
        }
      }
    }
    
  }
  linEq->matrix[1][0] = linEq->matrix[0][1];
  linEq->matrix[2][0] = linEq->matrix[0][2];
  linEq->matrix[1][1] = linEq->matrix[0][2];
  linEq->matrix[2][1] = linEq->matrix[1][2];
  for(i=3;i<linEq->dim;i++){
    linEq->matrix[i][0] = linEq->matrix[0][i];
    linEq->matrix[i][1] = linEq->matrix[1][i];
    linEq->matrix[i][2] = linEq->matrix[2][i];
  }

  /*printf("Orig Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }*/

  //check if matrix has zeroes
  for(i=0;i<linEq->dim;i++){
    if(linEq->matrix[i][i] == 0.){
      printf("WARNING: matrix element %i is zero, cannot solve.\n",i);
      return 0;
    }
  } 

  //modify the curvature matrix
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      if(i!=j){
        linEq->mat_weights[i][j] = 1.0/sqrt(linEq->matrix[i][i]*linEq->matrix[j][j]);
        cmatrix[i][j] = linEq->matrix[i][j]*linEq->mat_weights[i][j];
      }  
    }
    linEq->mat_weights[i][i] = 1.0/sqrt(linEq->matrix[i][i]*linEq->matrix[i][i]);
    cmatrix[i][i] = flambda + 1.0;
  }

  memcpy(linEq->matrix,cmatrix,sizeof(linEq->matrix));

  /*printf("Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }
  printf("Weight Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->mat_weights[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<linEq->dim;i++){
    printf("%10.4Lf ",linEq->vector[i]);
  }
  printf("\n");*/

  return 1;

}

//function which specifies constraining conditions for peak fit parameters
int areParsValid(){
  int i;
  int fitRange = fitpar.fitEndCh - fitpar.fitStartCh;
  for(i=0;i<fitpar.numFitPeaks;i+=3){
    
    if(fitpar.fitParVal[7+(3*i)] < fitpar.fitStartCh){
      return 0;
    }
    if(fitpar.fitParVal[7+(3*i)] > fitpar.fitEndCh){
      return 0;
    }
    if(fabs(fitpar.fitParVal[7+(3*i)] - fitpar.fitPeakInitGuess[i]) > (fitRange)/4.){
      return 0;
    }
    if(fabs(fitpar.fitParVal[8+(3*i)]) > (fitRange)/2.){
      return 0;
    }
    if(getDispSpBinVal(0,fitpar.fitPeakInitGuess[i]-drawing.lowerLimit) > 0){
      if(fitpar.fitParVal[6+(3*i)] < 0.){
        return 0;
      }
    }else{
      if(fitpar.fitParVal[6+(3*i)] > 0.){
        return 0;
      }
    }
  }
  return 1;
}


//non-linearized fitting
//return value: number of iterations performed (if fit not converged), -1 (if fit converged)
int nonLinearizedGausFit(const unsigned int numIter, const double convergenceFrac, lin_eq_type *linEq){

  int i;
  int iterCurrent = 0;
  int conv = 0; //converged?
  int lmCount = 0; //counter if at a chisq local minimum

  double iterStartChisq, iterEndChisq;
  double flamda = .001;

  double prevFitParVal[6+(3*MAX_FIT_PK)]; //storage for previous iteration fit parameters

  while(iterCurrent < numIter){

    iterStartChisq = getFitChisq();
    memcpy(prevFitParVal,fitpar.fitParVal,sizeof(fitpar.fitParVal));

    /*printf("\nFit iteration %i - A: %f, B: %f, C: %f\n",iterCurrent, fitpar.fitParVal[0],fitpar.fitParVal[1],fitpar.fitParVal[2]);
    for(i=0;i<fitpar.numFitPeaks;i++){
      printf("A%i: %f, P%i: %f, W%i: %f\n",i+1,fitpar.fitParVal[6+(3*i)],i+1,fitpar.fitParVal[7+(3*i)],i+1,fitpar.fitParVal[8+(3*i)]);
    }
    printf("chisq: %f\n",iterStartChisq);
    printf("\n");*/

    if(!(setupFitSums(linEq,flamda))){
      //the return value being less than the requested number of iterations indicates a failure
      return iterCurrent; 
    }
    if(!(solve_lin_eq(linEq,1))){
      //the return value being less than the requested number of iterations indicates a failure
      return iterCurrent; 
    }else{
      iterCurrent++;
      conv=1;

      /*printf("Solution\n");
      for(i=0;i<linEq->dim;i++){
        printf("%10.4Lf ",linEq->solution[i]);
      }
      printf("\n");*/

      //assign parameter values
      for(i=0;i<3;i++){
        if((fitpar.fitParVal[i]!=0.)&&(fabs(linEq->solution[i]/fitpar.fitParVal[i]) > convergenceFrac)){
          //printf("frac %i: %f\n",i,fabs(linEq->solution[i]/fitpar.fitParVal[i]));
          conv=0;
        }
        fitpar.fitParVal[i] += linEq->solution[i];
        //printf("par %i: %f\n",i,fitpar.fitParVal[i]);
      }
      for(i=0;i<(3*fitpar.numFitPeaks);i++){
        if((fitpar.fitParVal[6+i]!=0.)&&(fabs(linEq->solution[3+i]/fitpar.fitParVal[6+i]) > convergenceFrac)){
          //printf("frac %i: %f\n",3+i,fabs(linEq->solution[3+i]/fitpar.fitParVal[6+i]));
          conv=0;
        }
        fitpar.fitParVal[6+i] += linEq->solution[3+i];
        //printf("par %i: %f\n",6+i,fitpar.fitParVal[6+i]);
      }
      

      //check chisq, if it increased change value of flambda and try again
      iterEndChisq = getFitChisq();
      //printf("Start chisq: %f, end chisq: %f\n",iterStartChisq,iterEndChisq);

      if((iterEndChisq!=iterEndChisq)||((iterEndChisq > iterStartChisq)&&(iterEndChisq > 0.))){
        if(flamda < 2.0){
          flamda *= 2.0;
          //revert fit parameters
          memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
        }else{
          flamda /= 10.;
        }
      }else if(((iterStartChisq-iterEndChisq)/iterStartChisq) < convergenceFrac) {
        lmCount++;
        flamda /= 10.;
      }else{
        lmCount = 0;
        flamda /= 10.;
      }

      if(areParsValid() == 0){
        //revert fit parameters
        memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
      }

      //check convergence condition
      if(conv == 1){
        return -1;
      }
      //getc(stdin);

    }
  }

  return iterCurrent;
}


int performGausFit(){

  int ndf = (int)((fitpar.fitEndCh - fitpar.fitStartCh)/(1.0*drawing.contractFactor)) - (3+(3*fitpar.numFitPeaks));
  if(ndf <= 0){
    printf("Not enough degrees of freedom to fit!\n");
    return 0;
  }

  int i;
  fitpar.errFound = 0;

  //width parameters
  fitpar.widthFGH[0] = 3.;
  fitpar.widthFGH[1] = 2.;
  fitpar.widthFGH[2] = 0.;

  //assign initial guesses for background
  fitpar.fitParVal[0] = (getDispSpBinVal(0,fitpar.fitStartCh-drawing.lowerLimit) + getDispSpBinVal(0,fitpar.fitEndCh-drawing.lowerLimit))/2.0;
  fitpar.fitParVal[1] = (getDispSpBinVal(0,fitpar.fitEndCh-drawing.lowerLimit) - getDispSpBinVal(0,fitpar.fitStartCh-drawing.lowerLimit))/(fitpar.fitEndCh - fitpar.fitStartCh);
  fitpar.fitParVal[2] = 0.0;

  //assign initial guesses for non-linear params
  for(i=0;i<fitpar.numFitPeaks;i++){
    fitpar.fitParVal[6+(3*i)] = getDispSpBinVal(0,fitpar.fitPeakInitGuess[i]-drawing.lowerLimit) - fitpar.fitParVal[0] - fitpar.fitParVal[1]*fitpar.fitPeakInitGuess[i];
    fitpar.fitParVal[7+(3*i)] = fitpar.fitPeakInitGuess[i];
    fitpar.fitParVal[8+(3*i)] = getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
  }

  //printf("Initial guesses: %f %f %f %f %f %f\n",fitpar.fitParVal[0],fitpar.fitParVal[1],fitpar.fitParVal[2],fitpar.fitParVal[6],fitpar.fitParVal[7],fitpar.fitParVal[8]);

  lin_eq_type linEq;


  //do non-linearized fit
  int numNLIterTry = 500;
  int numNLIter = nonLinearizedGausFit(numNLIterTry, 0.0001, &linEq);
  if(numNLIter == -1){
    printf("Non-linear fit converged.\n");
    fitpar.errFound = getParameterErrors(&linEq);
  }else if (numNLIter >= numNLIterTry){
    printf("Fit did not converge after %i iterations.  Giving up...\n",numNLIter);
  }else{
    printf("WARNING: failed fit, iteration %i.\n",numNLIter);
    return 0;
  }

  //get fit parameter uncertainties
  fitpar.errFound = 0;
  if(setupFitSums(&linEq,0.)){
    if(solve_lin_eq(&linEq,1)){
      fitpar.errFound = getParameterErrors(&linEq);
    }
  }
  
  

  /*printf("Matrix\n");
  int j;
  for(i=0;i<linEq.dim;i++){
    for(j=0;j<linEq.dim;j++){
      printf("%Lf ",linEq.matrix[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<linEq.dim;i++){
    printf("%Lf ",linEq.vector[i]);
  }
  printf("\n");*/
  //getc(stdin);

  //make sure widths are positive
  for(i=0;i<fitpar.numFitPeaks;i+=3){
    if(fitpar.fitParVal[8+(3*i)] < 0.){
      fitpar.fitParVal[8+(3*i)] = fabs(fitpar.fitParVal[8+(3*i)]);
    }
  }
    
  printf("\nFit result - chisq/ndf: %f\nA: %f +/- %f, B: %f +/- %f, C: %f +/- %f\n",getFitChisq()/(1.0*ndf),fitpar.fitParVal[0],fitpar.fitParErr[0],fitpar.fitParVal[1],fitpar.fitParErr[1],fitpar.fitParVal[2],fitpar.fitParErr[2]);
  for(i=0;i<fitpar.numFitPeaks;i++){
    printf("A%i: %f +/- %f, P%i: %f +/- %f, W%i: %f +/- %f\n",i+1,fitpar.fitParVal[6+(3*i)],fitpar.fitParErr[6+(3*i)],i+1,fitpar.fitParVal[7+(3*i)],fitpar.fitParErr[7+(3*i)],i+1,fitpar.fitParVal[8+(3*i)],fitpar.fitParErr[8+(3*i)]);
  }
  printf("\n");
  
  return 1;
}
