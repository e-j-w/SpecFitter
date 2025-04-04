/* © J. Williams, 2020-2025 */

//This file contains routines for accessing spectrum data, 
//mainly to help other parts of the program with drawing, 
//fitting, and saving displayed data to disk

int getFirstViewDependingOnSp(const int32_t spInd){
  if((spInd<0)||(spInd>=rawdata.numSpOpened)){
    return -1;
  }
  for(int32_t i=0;i<rawdata.numViews;i++){
    if(rawdata.viewMode[i] == VIEWTYPE_SAVEDFIT_OFSP){
      if(rawdata.viewMultiPlots[i][0] == spInd){
        return i;
      }
      //printf("%u %i\n",rawdata.viewMultiPlots[i][0],spInd);
    }else{
      for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
        if(rawdata.viewMultiPlots[i][j] == spInd){
          return i;
        }
      }
    }
  }
  return -1;
}

int getFirstViewDependingOnView(const int32_t viewInd){
  if((viewInd<0)||(viewInd>=rawdata.numViews)){
    return -1;
  }
  for(int32_t i=0;i<rawdata.numViews;i++){
    if(rawdata.viewMode[i] == VIEWTYPE_SAVEDFIT_OFVIEW){
      if(rawdata.viewMultiPlots[i][0] == viewInd){
        return i;
      }
    }
  }
  return -1;
}

void deleteSpectrumOrView(const int32_t spInd){
  
  //printf("deleting spectrum %i\n",spInd);
  if((spInd>=(rawdata.numSpOpened+rawdata.numViews))||(spInd<0)){
    printf("WARNING: attempted to delete spectrum at invalid index %i\n",spInd);
    return;
  }

  if(spInd<rawdata.numSpOpened){
    //deleting spectrum data

    //delete views that depend on the data
    int32_t deletingViews = 1;
    while(deletingViews){
      int32_t viewToDel = getFirstViewDependingOnSp(spInd);
      if(viewToDel >= 0){
        
        //printf("Deleting view: %i\n",viewToDel);
        deleteSpectrumOrView(viewToDel + rawdata.numSpOpened); //recursive function call

      }else{
        deletingViews = 0;
      }
    }

    //delete comments
    for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
      if(rawdata.chanCommentView[i] == 0){
        if(rawdata.chanCommentSp[i] == spInd){
          //delete the comment
          for(int32_t j=i;j<(int32_t)(rawdata.numChComments-1);j++){
            memcpy(&rawdata.chanComment[j],&rawdata.chanComment[j+1],sizeof(rawdata.chanComment[j]));
            memcpy(&rawdata.chanCommentView[j],&rawdata.chanCommentView[j+1],sizeof(rawdata.chanCommentView[j]));
            memcpy(&rawdata.chanCommentSp[j],&rawdata.chanCommentSp[j+1],sizeof(rawdata.chanCommentSp[j]));
            memcpy(&rawdata.chanCommentCh[j],&rawdata.chanCommentCh[j+1],sizeof(rawdata.chanCommentCh[j]));
            memcpy(&rawdata.chanCommentVal[j],&rawdata.chanCommentVal[j+1],sizeof(rawdata.chanCommentVal[j]));
          }
          rawdata.numChComments -= 1;
          i -= 1; //indices have shifted, reheck the current index
        }else if(rawdata.chanCommentSp[i] > spInd){
          //change to the new index for the spectrum
          rawdata.chanCommentSp[i] = (uint8_t)(rawdata.chanCommentSp[i]-1);
        }
      }
    }

    //realign view data so that it still points to the proper spectra
    for(int32_t i=0;i<rawdata.numViews;i++){
      if(rawdata.viewMode[i] == VIEWTYPE_SAVEDFIT_OFSP){
        if(rawdata.viewMultiPlots[i][0] > spInd){
          rawdata.viewMultiPlots[i][0] = (uint8_t)(rawdata.viewMultiPlots[i][0] - 1);
        }
      }else{
        for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
          if(rawdata.viewMultiPlots[i][j] > spInd){
            rawdata.viewMultiPlots[i][j] = (uint8_t)(rawdata.viewMultiPlots[i][j] - 1);
          }
        }
        for(int32_t j=spInd;j<(rawdata.numSpOpened-1);j++){
          rawdata.viewScaleFactor[i][j]=rawdata.viewScaleFactor[i][j+1];
        }
      }
    }

    //delete spectrum data
    for(int32_t i=spInd;i<(rawdata.numSpOpened-1);i++){
      memcpy(&rawdata.hist[i],&rawdata.hist[i+1],sizeof(rawdata.hist[i]));
      memcpy(&rawdata.histComment[i],&rawdata.histComment[i+1],sizeof(rawdata.histComment[i]));
    }
    if(rawdata.numSpOpened > 0){
      rawdata.numSpOpened = (uint8_t)(rawdata.numSpOpened-1);
    }
    if(rawdata.numSpOpened == 0){
      rawdata.openedSp = 0;
    }

  }else{
    //deleting view

    int32_t viewInd = spInd - rawdata.numSpOpened;

    //delete views that depend on the data
    int32_t deletingViews = 1;
    while(deletingViews){
      int32_t viewToDel = getFirstViewDependingOnView(viewInd);
      if(viewToDel >= 0){

        deleteSpectrumOrView(viewToDel + rawdata.numSpOpened); //recursive function call

      }else{
        deletingViews = 0;
      }
    }

    //delete comments associated with the view
    for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
      if(rawdata.chanCommentView[i] == 1){
        if(rawdata.chanCommentSp[i] == viewInd){
          //delete the comment
          for(int32_t j=i;j<(int32_t)(rawdata.numChComments-1);j++){
            memcpy(&rawdata.chanComment[j],&rawdata.chanComment[j+1],sizeof(rawdata.chanComment[j]));
            memcpy(&rawdata.chanCommentView[j],&rawdata.chanCommentView[j+1],sizeof(rawdata.chanCommentView[j]));
            memcpy(&rawdata.chanCommentSp[j],&rawdata.chanCommentSp[j+1],sizeof(rawdata.chanCommentSp[j]));
            memcpy(&rawdata.chanCommentCh[j],&rawdata.chanCommentCh[j+1],sizeof(rawdata.chanCommentCh[j]));
            memcpy(&rawdata.chanCommentVal[j],&rawdata.chanCommentVal[j+1],sizeof(rawdata.chanCommentVal[j]));
          }
          rawdata.numChComments -= 1;
          i -= 1; //indices have shifted, reheck the current index
        }else if(rawdata.chanCommentSp[i] > viewInd){
          //change to the new index for the view
          rawdata.chanCommentSp[i] = (uint8_t)(rawdata.chanCommentSp[i]-1);
        }
      }
    }

    //delete view
    int32_t fitToDel = -1;
    if((rawdata.viewMode[viewInd] == VIEWTYPE_SAVEDFIT_OFVIEW)||(rawdata.viewMode[viewInd] == VIEWTYPE_SAVEDFIT_OFSP)){
      fitToDel = rawdata.viewMultiPlots[viewInd][1];
    }
    for(int32_t i=viewInd;i<(rawdata.numViews-1);i++){
      memcpy(&rawdata.viewComment[i],&rawdata.viewComment[i+1],sizeof(rawdata.viewComment[i]));
      memcpy(&rawdata.viewMode[i],&rawdata.viewMode[i+1],sizeof(rawdata.viewMode[i]));
      memcpy(&rawdata.viewNumMultiplotSp[i],&rawdata.viewNumMultiplotSp[i+1],sizeof(rawdata.viewNumMultiplotSp[i]));
      memcpy(&rawdata.viewScaleFactor[i],&rawdata.viewScaleFactor[i+1],sizeof(rawdata.viewScaleFactor[i]));
      memcpy(&rawdata.viewMultiPlots[i],&rawdata.viewMultiPlots[i+1],sizeof(rawdata.viewMultiPlots[i]));
    }
    if(fitToDel >= 0){
      //delete saved fit data
      //printf("Deleting saved fit %u.\n",fitToDel);
      for(int32_t i=fitToDel; i<(rawdata.numSavedFits-1); i++){
        memcpy(&rawdata.savedFitPar[i],&rawdata.savedFitPar[i+1],sizeof(fitpar));
      }
      rawdata.numSavedFits = (uint8_t)(rawdata.numSavedFits-1);
      //re-map fit data
      for(uint8_t i=0;i<rawdata.numViews;i++){
        if((rawdata.viewMode[i] == VIEWTYPE_SAVEDFIT_OFVIEW)||(rawdata.viewMode[i] == VIEWTYPE_SAVEDFIT_OFSP)){
          if(rawdata.viewMultiPlots[i][1] > fitToDel){
            rawdata.viewMultiPlots[i][1] = (uint8_t)(rawdata.viewMultiPlots[i][1] - 1);
          }
        }
      }
    }

    if(rawdata.numViews > 0){
      rawdata.numViews = (uint8_t)(rawdata.numViews-1);
    }

  }

  

  return;
}

int getFirstNonemptySpectrum(const int32_t numSpOpened){
  if(numSpOpened>=NSPECT){
    return -1;
  }
  for(int32_t i=0;i<numSpOpened;i++){
    for(int32_t j=0;j<S32K;j++){
      if(rawdata.hist[i][j]!=0.0){
        return i;
      }
    }
  }
  return -1;
}

//used to check whether a spectrum has been selected in multiplot mode
int isSpSelected(const int32_t spNum){
  for(int32_t i=0;i<drawing.numMultiplotSp;i++){
    if(drawing.multiPlots[i] == spNum){
      return 1;
    }
  }
  return 0;
}

//get a calibrated value from an uncalibrated one
double getCalVal(const double val){
  return calpar.calpar[0] + calpar.calpar[1]*val + calpar.calpar[2]*val*val;
}
double getUnCalVal(const double calVal){
  if((calpar.calpar[2] == 0.0)&&(calpar.calpar[1] != 0.0)){
    //linear calibration
    return calVal/calpar.calpar[1];
  }else{
    //use the quadratic equation!
    double x = sqrt(calpar.calpar[1]*calpar.calpar[1] + 4.0*calpar.calpar[2]*(calpar.calpar[0] - calVal))/(2.0*(calpar.calpar[2]));
    if(x != x){
      //NaN
      return 0.0;
    }
    //printf("%f %f %f\n",x,(-1.0*calpar.calpar[1] + x),(-1.0*calpar.calpar[1] - x));
    if((-1.0*calpar.calpar[1] + x) >= 0.0){
      return (-1.0*calpar.calpar[1] + x);
    }else{
      return (-1.0*calpar.calpar[1] - x);
    }
  }
}
//get a calibrated width from an uncalibrated one (is this always true?)
//this is used for uncertainties as well (since these are "widths" around a central value)
//'centroid' specifies the uncalibrated value that the error is taken relative to
//(ie. for peak width errors, this is the peak centroid)
double getCalWidth(const double val, const double centroid){
  return val*getCalVal(centroid)/centroid;
}
double getUnCalWidth(const double calWidth, const double unCalCentroid){
  return calWidth*unCalCentroid/getCalVal(unCalCentroid);
}

//lower level spectrum data access routine which takes rebinning into account
double getSpBinValRaw(const int32_t spNumRaw, const int32_t bin, const double scaleFactor, const int32_t contractFactor){

  if(spNumRaw >= NSPECT){
    return 0;
  }

  double val = 0.;
  for(int32_t i=0;i<contractFactor;i++){
    if((bin+i) < S32K){
      val += (double)(scaleFactor*rawdata.hist[spNumRaw][bin+i]);
    }
  }
  return val;
}

//if getWeight is set, will return weight values for fitting
double getSpBinValOrWeight(const int32_t dispSpNum, const int32_t bin, const int32_t getWeight){

  /*if((dispSpNum >= drawing.numMultiplotSp)||(dispSpNum < 0)){
    //invalid displayed spectrum number
    return 0;
  }*/

  double val = 0.;

  switch(drawing.multiplotMode){
    case VIEWTYPE_SUMMED:
      //sum spectra
      for(int32_t j=0;j<drawing.contractFactor;j++){
        if(getWeight){
          for(int32_t k=0;k<drawing.numMultiplotSp;k++){
            val += (double)(drawing.scaleFactor[drawing.multiPlots[k]]*drawing.scaleFactor[drawing.multiPlots[k]]*fabs(rawdata.hist[drawing.multiPlots[k]][bin+j]));
          }
        }else{
          for(int32_t k=0;k<drawing.numMultiplotSp;k++){
            val += (double)(drawing.scaleFactor[drawing.multiPlots[k]]*rawdata.hist[drawing.multiPlots[k]][bin+j]);
          }
        }
      }
      break;
    case VIEWTYPE_STACKED:
      //stacked
    case VIEWTYPE_OVERLAY_INDEPENDENT:
      //overlay (independent scaling)
    case VIEWTYPE_OVERLAY_COMMON:
      //overlay (common scaling)
    case VIEWTYPE_NONE:
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
double getDispSpBinVal(const int32_t dispSpNum, const int32_t bin){
  return getSpBinValOrWeight(dispSpNum,drawing.lowerLimit+bin,0);
}
double getDispSpBinErr(const int32_t dispSpNum, const int32_t bin){
  return sqrt(fabs(getSpBinValOrWeight(dispSpNum,drawing.lowerLimit+bin,1)));
}
double getDispSpBinValAdj(const int32_t dispSpNum, const int32_t bin){
  switch(drawing.valueDrawMode){
    case VALUE_PLUSERR:
      return getDispSpBinVal(dispSpNum,bin) + getDispSpBinErr(dispSpNum,bin);
      break;
    case VALUE_MINUSERR:
      return getDispSpBinVal(dispSpNum,bin) - getDispSpBinErr(dispSpNum,bin);
      break;
    case VALUE_DATA:
    default:
      return getDispSpBinVal(dispSpNum,bin);
      break;
  }
}
double getSpBinVal(const int32_t dispSpNum, const int32_t bin){
  return getSpBinValOrWeight(dispSpNum,bin,0);
}
double getSpBinFitWeight(const int32_t dispSpNum, const int32_t bin){
  return getSpBinValOrWeight(dispSpNum,bin,1);
}