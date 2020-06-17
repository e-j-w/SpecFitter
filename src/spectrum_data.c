/* J. Williams, 2020 */

//This file contains routines for accessing spectrum data, 
//mainly to help other parts of the program with drawing, 
//fitting, and saving displayed data to disk

int getFirstNonemptySpectrum(int numSpOpened){
  if(numSpOpened>=NSPECT){
    return -1;
  }
  int i,j;
  for(i=0;i<numSpOpened;i++){
    for(j=0;j<S32K;j++){
      if(rawdata.hist[i][j]!=0.0){
        return i;
      }
    }
  }
  return -1;
}

//used to check whether a spectrum has been selected in multiplot mode
int isSpSelected(int spNum){
  int i;
  for(i=0;i<drawing.numMultiplotSp;i++){
    if(drawing.multiPlots[i] == spNum){
      return 1;
    }
  }
  return 0;
}

//get a calibrated value from an uncalibrated one
double getCalVal(double val){
  return calpar.calpar0 + calpar.calpar1*val + calpar.calpar2*val*val;
}

//lower level spectrum data access routine which takes rebinning into account
float getSpBinValRaw(const int spNumRaw, const int bin, const double scaleFactor, const int contractFactor){
  int i;
  float val = 0.;
  for(i=0;i<contractFactor;i++){
    val += scaleFactor*rawdata.hist[spNumRaw][bin+i];
  }
  return val;
}

//if getWeight is set, will return weight values for fitting
float getSpBinValOrWeight(const int dispSpNum, const int bin, const int getWeight){

  if((dispSpNum >= drawing.numMultiplotSp)||(dispSpNum < 0)){
    //invalid displayed spectrum number
    return 0;
  }

  int j,k;
  float val = 0.;

  switch(drawing.multiplotMode){
    case 1:
      //sum spectra
      for(j=0;j<drawing.contractFactor;j++){
        if(getWeight){
          for(k=0;k<drawing.numMultiplotSp;k++){
            val += drawing.scaleFactor[drawing.multiPlots[k]]*drawing.scaleFactor[drawing.multiPlots[k]]*fabs(rawdata.hist[drawing.multiPlots[k]][bin+j]);
          }
        }else{
          for(k=0;k<drawing.numMultiplotSp;k++){
            val += drawing.scaleFactor[drawing.multiPlots[k]]*rawdata.hist[drawing.multiPlots[k]][bin+j];
          }
        }
        break;
      }
      break;
    case 4:
      //stacked
    case 3:
      //overlay (independent scaling)
    case 2:
      //overlay (common scaling)
    case 0:
      //no multiplot
      val = getSpBinValRaw(drawing.multiPlots[dispSpNum],bin,drawing.scaleFactor[drawing.multiPlots[dispSpNum]],drawing.contractFactor);
      break;
    default:
      break;
  }
  
  return val;
}
//get the value of the ith bin of the displayed spectrum
//i is in channel units, offset from drawing.lowerLimit
//for contracted spectra, the original channel units are retained, but the sum
//of j bins is returned, where j is the contraction factor
//spNum is the displayed spectrum number (for multiplot), 0 is the first displayed spectrum
float getDispSpBinVal(const int dispSpNum, const int bin){
  return getSpBinValOrWeight(dispSpNum,drawing.lowerLimit+bin,0);
}
float getSpBinVal(const int dispSpNum, const int bin){
  return getSpBinValOrWeight(dispSpNum,bin,0);
}
float getSpBinFitWeight(const int dispSpNum, const int bin){
  return getSpBinValOrWeight(dispSpNum,bin,1);
}