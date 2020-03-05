//function returns chisq evaluated for the current fit
double getFitChisq(int numFitPk){
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
    for(j=0;j<numFitPk;j++){
      f += fitpar.fitParVal[6+(3*j)]*exp(-0.5* pow((xval-fitpar.fitParVal[6+(3*j)+1]),2.0)/(2.0*fitpar.fitParVal[6+(3*j)+2]*fitpar.fitParVal[6+(3*j)+2]));
    }
    //pearson chisq
    if(f!=0.){
      chisq += pow(yval-f,2.0)/f;
    }
  }
  return chisq;
}