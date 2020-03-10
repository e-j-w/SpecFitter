//forward declarations
float getDispSpBinVal(int dispSpNum, int bin);


double getFWHM(double chan, double widthF, double widthG, double widthH){
  return sqrt(widthF*widthF + widthG*widthG*(chan/1000.) + widthH*widthH*(chan/1000.)*(chan/1000.));
}

//get the value of the fitted gaussian term for a given x value
double evalGaussTerm(int peakNum, double xval){
  double evalG = exp(-0.5* pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/(2.0*fitpar.fitParVal[8+(3*peakNum)]*fitpar.fitParVal[8+(3*peakNum)]));
  //printf("peakNum: %i, xval: %f, pos: %f, width: %f, eval: %f\n",peakNum,xval,fitpar.fitParVal[7+(3*peakNum)],fitpar.fitParVal[8+(3*peakNum)],evalG);
  return evalG;
}

double evalFit(double xval){
  int i;
  double val = fitpar.fitParVal[0] + xval*fitpar.fitParVal[1] + xval*xval*fitpar.fitParVal[2];
  for(i=0;i<fitpar.numFitPeaks;i++)
    val += fitpar.fitParVal[6+(3*i)]*evalGaussTerm(i,xval);
  return val;
}

double evalFitBG(double xval){
  return fitpar.fitParVal[0] + xval*fitpar.fitParVal[1] + xval*xval*fitpar.fitParVal[2];
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
    if(f<0.)
      return BIG_NUMBER;

    //likelihood ratio chisq
    if((f>0.)&&(yval > 0.)){
      chisq += (f - yval + yval*log(yval/f));
    }else{
      chisq += f;
    }
    //pearson chisq
    //chisq += (f-yval)*(f-yval)/f;
    //printf("yval = %f, f = %f, chisq = %f\n",yval,f,chisq);
    //getc(stdin);
  }
  return chisq;
}


//Non-linearized fitting stages
//Used to fit non-linear terms ie. peak positions and widths
//chisq minimized wrt these parameters using an iterative approach
int fitNonLinearizedWidths(double initVaryAmount){

  int i,j;
  int varying;
  double varyAmount;
  double finChisq;
  int failCount;

  //get initial chisq
  double initChisq = getFitChisq();
  //printf("chisq: %f\n",initChisq);

  //vary width parameters (F,G,H)
  for(i=0;i<3;i++){
    varying = 1;
    failCount = 0;

    //pick a random direction to start
    if((rand() % 2)==0)
      varyAmount = initVaryAmount;
    else
      varyAmount = -1*initVaryAmount;
    
    while(varying){
      fitpar.widthFGH[i] += varyAmount;
      
      //check for invalid width values and reverse course if neccesary
      if((fitpar.widthFGH[i] < 0)||(fitpar.widthFGH[i] > 10)){
        fitpar.widthFGH[i] -= (varyAmount+initVaryAmount);
        varyAmount = -1.*initVaryAmount*varyAmount/fabs(varyAmount);
      }

      //recalculate all peak widths
      for(j=0;j<fitpar.numFitPeaks;j++){
        fitpar.fitParVal[8+(3*j)] = getFWHM(fitpar.fitParVal[7+(3*j)],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
      }
      finChisq = getFitChisq();
      if((finChisq >= initChisq)||(finChisq < 0)){
        fitpar.widthFGH[i] -= varyAmount; //reset the parameter value
        //recalculate all peak widths
        for(j=0;j<fitpar.numFitPeaks;j++){
          fitpar.fitParVal[8+(3*j)] = getFWHM(fitpar.fitParVal[7+(3*j)],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
        }
        varyAmount = -1.*initVaryAmount*varyAmount/fabs(varyAmount);
        failCount++;
      }else{
        varyAmount = -initVaryAmount*(finChisq - initChisq)/varyAmount; //move based on derivative
        initChisq = finChisq; //new best chisq value
        failCount = 0;
      }
      //set a threshold in channels
      if((varyAmount < 0.0001*initVaryAmount)||(failCount >= 20)){
        varying = 0; //stop optimizing this parameter
      }
    }

  }
  return 1;
}
int fitNonLinearizedPositions(double initVaryAmount){

  int i;
  int varying;
  double varyAmount;
  double finChisq;
  int failCount;

  //get initial chisq
  double initChisq = getFitChisq();
  //printf("chisq: %f\n",initChisq);

  //vary position parameters
  for(i=0;i<fitpar.numFitPeaks;i++){
    varying = 1;
    failCount = 0;

    //pick a random direction to start
    if((rand() % 2)==0)
      varyAmount = initVaryAmount;
    else
      varyAmount = -1*initVaryAmount;
    
    while(varying){
      
      fitpar.fitParVal[7+(3*i)] += varyAmount;

      //check for invalid position values and reverse course if neccesary
      if((fitpar.fitParVal[7+(3*i)] < 0)||(fitpar.fitParVal[7+(3*i)] >= S32K)){
        fitpar.fitParVal[7+(3*i)] -= (varyAmount+initVaryAmount);
        varyAmount = -1.*initVaryAmount*varyAmount/fabs(varyAmount);
      }

      finChisq = getFitChisq();
      if((finChisq >= initChisq)||(finChisq < 0)){
        fitpar.fitParVal[7+(3*i)] -= varyAmount; //reset the parameter value
        varyAmount = -1.*initVaryAmount*varyAmount/fabs(varyAmount); 
        failCount++;
      }else{
        varyAmount = -1.*initVaryAmount*(finChisq - initChisq)/varyAmount; //move based on derivative
        initChisq = finChisq; //new best chisq value
        failCount = 0;
      }
      //printf("varyAmount: %f, varyDer: %f, chisq: %f\n",varyAmount,varyDer,initChisq);
      //set a threshold in channels
      if((varyAmount < 0.0001*initVaryAmount)||(failCount >= 20)){
        varying = 0; //stop optimizing this parameter
      }
    }
  }
  return 1;
}

//type=0: background fit only
//type=1: amplitude fit only
//type=2: combined fit
void setupFitSums(lin_eq_type *linEq, int type){

  int i,j,k;
  memset(linEq->matrix,0,sizeof(linEq->matrix));
  memset(linEq->vector,0,sizeof(linEq->vector));
  double xval,yval;

  switch(type){
    case 2:
      linEq->dim = 3 + fitpar.numFitPeaks;
      for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
        xval = i;
        yval = getDispSpBinVal(0,i-drawing.lowerLimit);
        //printf("xval: %f, yval: %f\n",xval,yval);
        linEq->matrix[0][0] += 1./xval;
        linEq->matrix[0][1] += 1.;
        linEq->matrix[0][2] += xval;
        linEq->matrix[1][2] += xval*xval;
        linEq->matrix[2][2] += xval*xval*xval;
        linEq->vector[0] += yval/xval;
        linEq->vector[1] += yval;
        linEq->vector[2] += yval*xval;
        for(j=0;j<fitpar.numFitPeaks;j++){
          linEq->matrix[0][j+3] += evalGaussTerm(j,xval)/xval;
          linEq->matrix[1][j+3] += evalGaussTerm(j,xval);
          linEq->matrix[2][j+3] += xval*evalGaussTerm(j,xval);
          linEq->vector[j+3] += yval*evalGaussTerm(j,xval)/xval;
          for(k=0;k<fitpar.numFitPeaks;k++){
            linEq->matrix[k+3][j+3] += evalGaussTerm(j,xval)*evalGaussTerm(k,xval)/xval;
          }
        }
      }
      linEq->matrix[1][0] = linEq->matrix[0][1];
      linEq->matrix[2][0] = linEq->matrix[0][2];
      linEq->matrix[1][1] = linEq->matrix[0][2];
      linEq->matrix[2][1] = linEq->matrix[1][2];
      for(i=0;i<fitpar.numFitPeaks;i++){
        linEq->matrix[i+3][0] = linEq->matrix[0][i+3];
        linEq->matrix[i+3][1] = linEq->matrix[1][i+3];
        linEq->matrix[i+3][2] = linEq->matrix[2][i+3];
      }
      break;
    case 1:
      linEq->dim = fitpar.numFitPeaks; //peak amplitude terms only
      for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
        xval = i;
        yval = getDispSpBinVal(0,i-drawing.lowerLimit) - fitpar.fitParVal[0] - fitpar.fitParVal[1]*xval - fitpar.fitParVal[2]*xval*xval;
        //printf("xval: %f, yval: %f\n",xval,yval);
        for(j=0;j<fitpar.numFitPeaks;j++){
          linEq->vector[j] += yval*evalGaussTerm(j,xval)/xval;
          for(k=0;k<fitpar.numFitPeaks;k++){
            linEq->matrix[k][j] += evalGaussTerm(j,xval)*evalGaussTerm(k,xval)/xval;
          }
        }
      }
      break;
    case 0:
      linEq->dim = 3; //quadratic background terms only
      for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
        xval = i;
        yval = getDispSpBinVal(0,i-drawing.lowerLimit);
        //printf("xval: %f, yval: %f\n",xval,yval);
        linEq->matrix[0][0] += 1./xval;
        linEq->matrix[0][1] += 1.;
        linEq->matrix[0][2] += xval;
        linEq->matrix[1][2] += xval*xval;
        linEq->matrix[2][2] += xval*xval*xval;
        linEq->vector[0] += yval/xval;
        linEq->vector[1] += yval;
        linEq->vector[2] += yval*xval;
      }
      linEq->matrix[1][0] = linEq->matrix[0][1];
      linEq->matrix[2][0] = linEq->matrix[0][2];
      linEq->matrix[1][1] = linEq->matrix[0][2];
      linEq->matrix[2][1] = linEq->matrix[1][2];
      break;
  }

  
  
}

int performGausFit(){

  int i;

  double prevFitParVal[6+(3*MAX_FIT_PK)]; //storage for previous iteration fit parameters

  //width parameters
  fitpar.widthFGH[0] = 3.;
  fitpar.widthFGH[1] = 2.;
  fitpar.widthFGH[2] = 0.;

  //assign initial guesses for non-linear params
  for(i=0;i<fitpar.numFitPeaks;i++){
    fitpar.fitParVal[7+(3*i)] = fitpar.fitPeakInitGuess[i];
    fitpar.fitParVal[8+(3*i)] = getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
  }

  lin_eq_type linEq;

  //iterations
  int iterNum = 0;
  int maxIterNum = 50;
  int maxFirstIterTries = 10;
  int firstIterCounter = 0;
  double firstIterChisq, iterChisq, lastIterChisq;
  double varyFactor = 1.0;
  while (iterNum < maxIterNum){

    if(iterNum > 0)
      lastIterChisq = iterChisq;

    memcpy(prevFitParVal,fitpar.fitParVal,sizeof(fitpar.fitParVal));
    
    //setup and perform linearized quadratic background fit
    setupFitSums(&linEq,0);
    if(!(solve_lin_eq(&linEq))){
      printf("WARNING: failed fit - linearized stage (background only), iteration %i.  Reverting.\n",iterNum);
      memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
    }else{
      //assign background parameters
      for(i=0;i<3;i++){
        fitpar.fitParVal[i] = linEq.solution[i];
        //printf("par %i: %f\n",i,fitpar.fitParVal[i]);
      }
    }
    
    //setup and perform linearized peak amplitude fit
    setupFitSums(&linEq,1);
    if(!(solve_lin_eq(&linEq))){
      printf("WARNING: failed fit - linearized stage (amplitudes only), iteration %i.  Reverting.\n",iterNum);
      memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
    }else{
      //assign amplitude parameters
      for(i=0;i<fitpar.numFitPeaks;i++){
        fitpar.fitParVal[6+(3*i)] = linEq.solution[i];
        //printf("amplitude %i: %f\n",i,fitpar.fitParVal[6+(3*i)]);
      }
    }
    
    if(!(fitNonLinearizedWidths(varyFactor))){
      printf("WARNING: failed fit - nonlinearized stage (widths), iteration %i.\n",iterNum);
      break;
    }

    if(!(fitNonLinearizedPositions(varyFactor))){
      printf("WARNING: failed fit - nonlinearized stage (positions), iteration %i.\n",iterNum);
      break;
    }

    //setup and perform linearized fit of all linear parameters
    setupFitSums(&linEq,2);
    if(!(solve_lin_eq(&linEq))){
      printf("WARNING: failed fit - linearized stage (all parameters), iteration %i.  Reverting.\n",iterNum);
      memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
    }else{
      //assign all linear parameters
      for(i=0;i<3;i++){
        fitpar.fitParVal[i] = linEq.solution[i];
        //printf("par %i: %f\n",i,fitpar.fitParVal[i]);
      }
      for(i=0;i<fitpar.numFitPeaks;i++){
        fitpar.fitParVal[6+(3*i)] = linEq.solution[3+i];
        //printf("amplitude %i: %f\n",i,fitpar.fitParVal[6+(3*i)]);
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

    iterChisq = getFitChisq();
    if(iterNum == 0){
      if(iterChisq != BIG_NUMBER){
        firstIterChisq = iterChisq;
      }else{
        iterNum--; //redo first iteration
        firstIterCounter++;
        if(firstIterCounter > maxFirstIterTries){
          printf("WARNING: failed fit - first iteration\n");
          break;
        }
      }
    }else if((iterChisq >= lastIterChisq)||(iterChisq < 0)){
      //reset the fit parameters, enforce that chisq must decrease
      memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
      iterChisq = lastIterChisq;
      //increase the range over which non-linear parameters are varied
      if(iterNum > 0)
        varyFactor *= 2.;
    }else{
      varyFactor = 1.0;
    }

    printf("Fit iteration %i - chisq: %f, A: %f, B: %f, C: %f",iterNum+1,iterChisq,fitpar.fitParVal[0],fitpar.fitParVal[1],fitpar.fitParVal[2]);
    for(i=0;i<fitpar.numFitPeaks;i++){
      printf(", A%i: %f, P%i: %f, W%i: %f",i+1,fitpar.fitParVal[6+(3*i)],i+1,fitpar.fitParVal[7+(3*i)],i+1,fitpar.fitParVal[8+(3*i)]);
    }
    printf("\n");

    if(iterChisq != BIG_NUMBER){
      if(varyFactor >= 128.){
        if(fabs(lastIterChisq - iterChisq) < fabs(0.0001*firstIterChisq)){
          //end the fit
          printf("Fit converged at iteration %i.\n",iterNum+1);
          break;
        }
      }
      
    }
    

    iterNum++;
  }
  
  return 1;
}