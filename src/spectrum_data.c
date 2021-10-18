/* © J. Williams, 2020-2021 */

//This file contains routines for accessing spectrum data, 
//mainly to help other parts of the program with drawing, 
//fitting, and saving displayed data to disk

int getFirstViewDependingOnSp(const int spInd){
  if((spInd<0)||(spInd>=rawdata.numSpOpened)){
    return -1;
  }
  int i,j;
  for(i=0;i<rawdata.numViews;i++){
    for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
      if(rawdata.viewMultiPlots[i][j] == spInd){
        return i;
      }
    }
  }
  return -1;
}

void deleteSpectrumOrView(const int spInd){
  
  //printf("deleting spectrum %i\n",spInd);
  if((spInd>=(rawdata.numSpOpened+rawdata.numViews))||(spInd<0)){
    printf("WARNING: attempted to delete spectrum at invalid index %i\n",spInd);
    return;
  }

  int i,j;

  if(spInd<rawdata.numSpOpened){
    //deleting spectrum data

    //delete comments
    for(i=0;i<rawdata.numChComments;i++){
      if(rawdata.chanCommentView[i] == 0){
        if(rawdata.chanCommentSp[i] == spInd){
          //delete the comment
          for(j=i;j<(rawdata.numChComments-1);j++){
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
          rawdata.chanCommentSp[i] = (unsigned char)(rawdata.chanCommentSp[i]-1);
        }
      }
    }

    //delete views that depend on the data
    int deletingViews = 1;
    while(deletingViews){
      int viewToDel = getFirstViewDependingOnSp(spInd);
      if(viewToDel >= 0){

        //delete comments associated with the view
        for(i=0;i<rawdata.numChComments;i++){
          if(rawdata.chanCommentView[i] == 1){
            if(rawdata.chanCommentSp[i] == viewToDel){
              //delete the comment
              for(j=i;j<(rawdata.numChComments-1);j++){
                memcpy(&rawdata.chanComment[j],&rawdata.chanComment[j+1],sizeof(rawdata.chanComment[j]));
                memcpy(&rawdata.chanCommentView[j],&rawdata.chanCommentView[j+1],sizeof(rawdata.chanCommentView[j]));
                memcpy(&rawdata.chanCommentSp[j],&rawdata.chanCommentSp[j+1],sizeof(rawdata.chanCommentSp[j]));
                memcpy(&rawdata.chanCommentCh[j],&rawdata.chanCommentCh[j+1],sizeof(rawdata.chanCommentCh[j]));
                memcpy(&rawdata.chanCommentVal[j],&rawdata.chanCommentVal[j+1],sizeof(rawdata.chanCommentVal[j]));
              }
              rawdata.numChComments -= 1;
              i -= 1; //indices have shifted, reheck the current index
            }else if(rawdata.chanCommentSp[i] > viewToDel){
              //change to the new index for the view
              rawdata.chanCommentSp[i] = (unsigned char)(rawdata.chanCommentSp[i]-1);
            }
          }
        }

        //delete view
        for(i=viewToDel;i<(rawdata.numViews-1);i++){
          memcpy(&rawdata.viewComment[i],&rawdata.viewComment[i+1],sizeof(rawdata.viewComment[i]));
          memcpy(&rawdata.viewMultiplotMode[i],&rawdata.viewMultiplotMode[i+1],sizeof(rawdata.viewMultiplotMode[i]));
          memcpy(&rawdata.viewNumMultiplotSp[i],&rawdata.viewNumMultiplotSp[i+1],sizeof(rawdata.viewNumMultiplotSp[i]));
          memcpy(&rawdata.viewScaleFactor[i],&rawdata.viewScaleFactor[i+1],sizeof(rawdata.viewScaleFactor[i]));
          memcpy(&rawdata.viewMultiPlots[i],&rawdata.viewMultiPlots[i+1],sizeof(rawdata.viewMultiPlots[i]));
        }
        if(rawdata.numViews > 0){
          rawdata.numViews = (unsigned char)(rawdata.numViews-1);
        }
      }else{
        deletingViews = 0;
      }
    }

    //realign view data so that it still points to the proper spectra
    for(i=0;i<rawdata.numViews;i++){
      for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
        if(rawdata.viewMultiPlots[i][j] > spInd){
          rawdata.viewMultiPlots[i][j] = (unsigned char)(rawdata.viewMultiPlots[i][j]-1);
        }
      }
      for(j=spInd;j<(rawdata.numSpOpened-1);j++){
        rawdata.viewScaleFactor[i][j]=rawdata.viewScaleFactor[i][j+1];
      }
    }

    //delete spectrum data
    for(i=spInd;i<(rawdata.numSpOpened-1);i++){
      memcpy(&rawdata.hist[i],&rawdata.hist[i+1],sizeof(rawdata.hist[i]));
      memcpy(&rawdata.histComment[i],&rawdata.histComment[i+1],sizeof(rawdata.histComment[i]));
    }
    if(rawdata.numSpOpened > 0){
      rawdata.numSpOpened = (unsigned char)(rawdata.numSpOpened-1);
    }
    if(rawdata.numSpOpened == 0){
      rawdata.openedSp = 0;
    }

  }else{
    //deleting view

    int viewInd = spInd - rawdata.numSpOpened;

    //delete comments associated with the view
    for(i=0;i<rawdata.numChComments;i++){
      if(rawdata.chanCommentView[i] == 1){
        if(rawdata.chanCommentSp[i] == viewInd){
          //delete the comment
          for(j=i;j<(rawdata.numChComments-1);j++){
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
          rawdata.chanCommentSp[i] = (unsigned char)(rawdata.chanCommentSp[i]-1);
        }
      }
    }

    //delete view
    for(i=viewInd;i<(rawdata.numViews-1);i++){
      memcpy(&rawdata.viewComment[i],&rawdata.viewComment[i+1],sizeof(rawdata.viewComment[i]));
      memcpy(&rawdata.viewMultiplotMode[i],&rawdata.viewMultiplotMode[i+1],sizeof(rawdata.viewMultiplotMode[i]));
      memcpy(&rawdata.viewNumMultiplotSp[i],&rawdata.viewNumMultiplotSp[i+1],sizeof(rawdata.viewNumMultiplotSp[i]));
      memcpy(&rawdata.viewScaleFactor[i],&rawdata.viewScaleFactor[i+1],sizeof(rawdata.viewScaleFactor[i]));
      memcpy(&rawdata.viewMultiPlots[i],&rawdata.viewMultiPlots[i+1],sizeof(rawdata.viewMultiPlots[i]));
    }
    if(rawdata.numViews > 0){
      rawdata.numViews = (unsigned char)(rawdata.numViews-1);
    }

  }

  

  return;
}

int getFirstNonemptySpectrum(const int numSpOpened){
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
int isSpSelected(const int spNum){
  int i;
  for(i=0;i<drawing.numMultiplotSp;i++){
    if(drawing.multiPlots[i] == spNum){
      return 1;
    }
  }
  return 0;
}

//get a calibrated value from an uncalibrated one
double getCalVal(const double val){
  return calpar.calpar0 + calpar.calpar1*val + calpar.calpar2*val*val;
}
//get a calibrated width from an uncalibrated one (is this always true?)
//this is used for uncertainties as well (since these are "widths" around a central value)
double getCalWidth(const double val){
  return fabs(calpar.calpar1*val + calpar.calpar2*val*val);
}

//lower level spectrum data access routine which takes rebinning into account
float getSpBinValRaw(const int spNumRaw, const int bin, const double scaleFactor, const int contractFactor){

  if(spNumRaw >= NSPECT){
    return 0;
  }

  int i;
  float val = 0.;
  for(i=0;i<contractFactor;i++){
    if((bin+i) < S32K){
      val += (float)(scaleFactor*rawdata.hist[spNumRaw][bin+i]);
    }
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
    case MULTIPLOT_SUMMED:
      //sum spectra
      for(j=0;j<drawing.contractFactor;j++){
        if(getWeight){
          for(k=0;k<drawing.numMultiplotSp;k++){
            val += (float)(drawing.scaleFactor[drawing.multiPlots[k]]*drawing.scaleFactor[drawing.multiPlots[k]]*fabs(rawdata.hist[drawing.multiPlots[k]][bin+j]));
          }
        }else{
          for(k=0;k<drawing.numMultiplotSp;k++){
            val += (float)(drawing.scaleFactor[drawing.multiPlots[k]]*rawdata.hist[drawing.multiPlots[k]][bin+j]);
          }
        }
      }
      break;
    case MULTIPLOT_STACKED:
      //stacked
    case MULTIPLOT_OVERLAY_INDEPENDENT:
      //overlay (independent scaling)
    case MULTIPLOT_OVERLAY_COMMON:
      //overlay (common scaling)
    case MULTIPLOT_NONE:
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