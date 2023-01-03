/* © J. Williams, 2020-2023 */

//routine to write a .jf3 file
//header containing: file format version number (uint8_t), number of spectra (uint8_t), label for each spactrum (each 256 element char array),
//number of comments (uint8_t), individual comments (comment sp (char), ch (int32), y-val (float32), followed by a 256 element char array for the comment itself)
//spectrum data is compressed using a basic RLE method: packet header (signed char) specifying number of elements to repeat, then the element as a 32-bit float
//alternatively, the packet header may be a negative number -n, in which case n non-repeating elements follow as 32-bit floats
//if the packet header is 0, that is the end of the spectrum  
int writeJF3(const char *filename, double inpHist[NSPECT][S32K]){
  
  FILE *out;
  uint8_t ucharBuf;
  uint32_t uintBuf;

  if ((out = fopen(filename, "w")) == NULL) //open the file
  {
    printf("ERROR: Cannot open the output file: %s\n", filename);
    printf("The file may not be accesible.\n");
    return 1;
  }

  //printf("Number of spectra to write: %i\n",rawdata.numSpOpened);

  ucharBuf = 2; //file format version number
  fwrite(&ucharBuf,sizeof(uint8_t),1,out);
  ucharBuf = rawdata.numSpOpened; //number of spectra to write
  fwrite(&ucharBuf,sizeof(uint8_t),1,out);
  for(int32_t i=0;i<rawdata.numSpOpened;i++){
    fwrite(&rawdata.histComment[i],sizeof(rawdata.histComment[i]),1,out);
  }
  //write calibration parameters
  fwrite(&calpar,sizeof(calpar),1,out);
  //write comments
  uintBuf = rawdata.numChComments; //number of comments to write
  fwrite(&uintBuf,sizeof(uint32_t),1,out);
  for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
    fwrite(&rawdata.chanCommentView[i],sizeof(rawdata.chanCommentView[i]),1,out);
    fwrite(&rawdata.chanCommentSp[i],sizeof(rawdata.chanCommentSp[i]),1,out);
    fwrite(&rawdata.chanCommentCh[i],sizeof(rawdata.chanCommentCh[i]),1,out);
    fwrite(&rawdata.chanCommentVal[i],sizeof(rawdata.chanCommentVal[i]),1,out);
    fwrite(&rawdata.chanComment[i],sizeof(rawdata.chanComment[i]),1,out);
  }
  //write views
  uintBuf = rawdata.numViews; //number of views to write
  fwrite(&uintBuf,sizeof(uint32_t),1,out);
  for(int32_t i=0;i<(int32_t)rawdata.numViews;i++){
    fwrite(&rawdata.viewComment[i],sizeof(rawdata.viewComment[i]),1,out);
    fwrite(&rawdata.viewMultiplotMode[i],sizeof(uint8_t),1,out);
    fwrite(&rawdata.viewNumMultiplotSp[i],sizeof(uint8_t),1,out);
    for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
      fwrite(&rawdata.viewMultiPlots[i][j],sizeof(uint8_t),1,out);
    }
    for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
      fwrite(&rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]],sizeof(double),1,out);
    }
  }

  float lastBin = 0.;
  float currentBin;
  float val;
  signed char packetCounter;
  for(int32_t i=0;i<rawdata.numSpOpened;i++){
    if(i<NSPECT){

      //printf("Writing spectrum %i\n",i);

      //scan for end of spectrum
      int lastCh = 0;
      for(int32_t j=S32K-1;j>=0;j--){
        if(inpHist[i][j]!=0){
          lastCh=j;
          break;
        }
      }

      //encode spectrum
      packetCounter = 1;
      for(int32_t j=0;j<S32K;j++){
        
        //get bin values
        currentBin = (float)inpHist[i][j];
        if(j>0)
          lastBin = (float)inpHist[i][j-1];
        else
          lastBin = (float)inpHist[i][j];
        
        //printf("bin: %i cuurent val: %f last val: %f packetCounter: %i\n",j,currentBin,lastBin,packetCounter);

        if(j>=lastCh){
          //end of spectrum reached
          if(packetCounter > 0){
            //write last packet
            fwrite(&packetCounter,sizeof(signed char),1,out);
            //printf("wrote packet counter %i\n",packetCounter);
            val = (float)inpHist[i][j-1];
            fwrite(&val,sizeof(float),1,out);
          }else{
            //write last packet
            fwrite(&packetCounter,sizeof(signed char),1,out);
            //printf("wrote packet counter %i\n",packetCounter);
            for(int32_t k=0;k<(packetCounter*-1);k++){
              val = (float)inpHist[i][j+packetCounter+k];
              fwrite(&val,sizeof(float),1,out);
            }
          }
          //write final packet
          packetCounter = 0;
          fwrite(&packetCounter,sizeof(signed char),1,out);
          //printf("wrote packet counter %i\n",packetCounter);
          val = (float)inpHist[i][j];
          fwrite(&val,sizeof(float),1,out);
          break;
        }else if(packetCounter > 126){
          //write last packet
          fwrite(&packetCounter,sizeof(signed char),1,out);
          val = (float)inpHist[i][j-1];
          fwrite(&val,sizeof(float),1,out);
          //start new packet
          packetCounter = 1;
        }else if (packetCounter < -126){
          //write last packet
          fwrite(&packetCounter,sizeof(signed char),1,out);
          //printf("wrote packet counter %i\n",packetCounter);
          for(int32_t k=0;k<(packetCounter*-1);k++){
            val = (float)inpHist[i][j+packetCounter+k];
            fwrite(&val,sizeof(float),1,out);
          }
          //start new packet
          packetCounter = 1;
        }else if((currentBin == lastBin)&&(packetCounter>0)){
          //continue packet
          if(j>0) //remember we start off with a value of 1
            packetCounter++;
        }else if((currentBin != lastBin)&&(packetCounter>1)){
          //write last packet
          fwrite(&packetCounter,sizeof(signed char),1,out);
          //printf("wrote packet counter %i\n",packetCounter);
          val = (float)inpHist[i][j-1];
          fwrite(&val,sizeof(float),1,out);
          //start new packet
          packetCounter = 1;
        }else if((currentBin != lastBin)&&(packetCounter==1)){
          //change packet type
          packetCounter = -2;
        }else if((currentBin == lastBin)&&(packetCounter<0)){
          //write last packet
          fwrite(&packetCounter,sizeof(signed char),1,out);
          //printf("wrote packet counter %i\n",packetCounter);
          for(int32_t k=0;k<(packetCounter*-1);k++){
            val = (float)inpHist[i][j+packetCounter+k];
            fwrite(&val,sizeof(float),1,out);
          }
          //start new packet
          packetCounter = 1;
        }else if((currentBin != lastBin)&&(packetCounter<0)){
          //continue packet
          packetCounter--;
        }
      }

    }

  }

  fclose(out);
  printf("Wrote data to file: %s\n",filename);
  return 0;
}

//routine to export a RadWare compatible file
//exportMode: 0=write displayed spectrum, 1=write all imported spectra
int exportSPE(const char *filePrefix, const int exportMode, const int rebin){
  
  int spID;
  float val;
  FILE *out;
  char outFileName[256];

  //radware file header info
  int32_t buffSize = 24;
  char spLabel[8];
  int32_t arraySize = 4096;
  int32_t junk = 1;
  int32_t byteSize = arraySize*4;

  switch(exportMode + (drawing.multiplotMode != MULTIPLOT_SUMMED)){
    case 1:
      //export all
      for(int32_t i=0;i<rawdata.numSpOpened;i++){

        snprintf(outFileName,256,"%s_hist%i.spe",filePrefix,i+1);
        if((out = fopen(outFileName, "w")) == NULL){ //open the file
          printf("ERROR: Cannot open the output file: %s\n", outFileName);
          printf("The file may not be accesible.\n");
          return 1;
        }

        //write .spe header
        strncpy(spLabel,rawdata.histComment[i],7);
        fwrite(&buffSize,sizeof(int32_t),1,out);
        fwrite(&spLabel,sizeof(spLabel),1,out);
        fwrite(&arraySize,sizeof(int32_t),1,out);
        fwrite(&junk,sizeof(int32_t),1,out);
        fwrite(&junk,sizeof(int32_t),1,out);
        fwrite(&junk,sizeof(int32_t),1,out);
        fwrite(&buffSize,sizeof(int32_t),1,out);
        fwrite(&byteSize,sizeof(int32_t),1,out);
        
        //write histogram
        if(rebin){
          for(int32_t j=0;j<(arraySize*drawing.contractFactor);j+=drawing.contractFactor){
            val = getSpBinValRaw(i,j,drawing.scaleFactor[i],drawing.contractFactor);
            fwrite(&val,sizeof(float),1,out);
          }
        }else{
          for(int32_t j=0;j<arraySize;j++){
            val = getSpBinValRaw(i,j,1,1);
            fwrite(&val,sizeof(float),1,out);
          }
        }
        fwrite(&byteSize,sizeof(int32_t),1,out);
        fclose(out);
        printf("Wrote data to file: %s\n",outFileName);
      }
      break;
    case 0:
      //export current (summed) spectrum view
      snprintf(outFileName,256,"%s.spe",filePrefix);
      if((out = fopen(outFileName, "w")) == NULL){ //open the file
        printf("ERROR: Cannot open the output file: %s\n", outFileName);
        printf("The file may not be accesible.\n");
        return 1;
      }

      //write .spe header
      strncpy(spLabel,"Summed",7);
      fwrite(&buffSize,sizeof(int32_t),1,out);
      fwrite(&spLabel,sizeof(spLabel),1,out);
      fwrite(&arraySize,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&buffSize,sizeof(int32_t),1,out);
      fwrite(&byteSize,sizeof(int32_t),1,out);

      //write histogram
      int32_t numBinsWritten = 0;
      for(int32_t j=0;j<S32K;j+=drawing.contractFactor){
        if(numBinsWritten < arraySize){
          val = getSpBinValOrWeight(0,j,0);
          fwrite(&val,sizeof(float),1,out);
          numBinsWritten++;
        }else{
          break;
        }
      }
      while(numBinsWritten < arraySize){
        val = 0.0f;
        fwrite(&val,sizeof(float),1,out);
        numBinsWritten++;
      }
      fwrite(&byteSize,sizeof(int32_t),1,out);
      fclose(out);
      printf("Wrote data to file: %s\n",outFileName);

      break;
    default:
      //export one histogram
      
      spID = exportMode-2;
      if((spID < 0)||(spID >= rawdata.numSpOpened)){
        printf("ERROR: invalid spectrum index for export.\n");
        return 2;
      }

      snprintf(outFileName,256,"%s.spe",filePrefix);
      if((out = fopen(outFileName, "w")) == NULL){ //open the file
        printf("ERROR: Cannot open the output file: %s\n", outFileName);
        printf("The file may not be accesible.\n");
        return 1;
      }

      //write .spe header
      strncpy(spLabel,rawdata.histComment[spID],7);
      fwrite(&buffSize,sizeof(int32_t),1,out);
      fwrite(&spLabel,sizeof(spLabel),1,out);
      fwrite(&arraySize,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&junk,sizeof(int32_t),1,out);
      fwrite(&buffSize,sizeof(int32_t),1,out);
      fwrite(&byteSize,sizeof(int32_t),1,out);

      //write histogram
      if(rebin){
        for(int32_t i=0;i<(arraySize*drawing.contractFactor);i+=drawing.contractFactor){
          val = getSpBinValRaw(spID,i,drawing.scaleFactor[spID],drawing.contractFactor);
          fwrite(&val,sizeof(float),1,out);
        }
      }else{
        for(int32_t i=0;i<arraySize;i++){
          val = getSpBinValRaw(spID,i,1,1);
          fwrite(&val,sizeof(float),1,out);
        }
      }
      fwrite(&byteSize,sizeof(int32_t),1,out);
      fclose(out);
      printf("Wrote data to file: %s\n",outFileName);
      break;
  }

  return 0;
}

//routine to export an .fmca file (S32K float values per spectrum)
//exportMode: 0=write displayed spectrum, 1=write all imported spectra
int exportFMCA(const char *filePrefix, const int exportMode, const int rebin){
  
  int spID;
  float val;
  FILE *out;
  char outFileName[256];

  snprintf(outFileName,256,"%s.fmca",filePrefix);
  if((out = fopen(outFileName, "w")) == NULL){ //open the file
    printf("ERROR: Cannot open the output file: %s\n", outFileName);
    printf("The file may not be accesible.\n");
    return 1;
  }

  switch(exportMode + (drawing.multiplotMode != MULTIPLOT_SUMMED)){
    case 1:
      //export all
      for(int32_t i=0;i<rawdata.numSpOpened;i++){
        //write histogram
        if(rebin){
          for(int32_t j=0;j<(S32K*drawing.contractFactor);j+=drawing.contractFactor){
            val = getSpBinValRaw(i,j,drawing.scaleFactor[i],drawing.contractFactor);
            fwrite(&val,sizeof(float),1,out);
          }
        }else{
          for(int32_t j=0;j<S32K;j++){
            val = getSpBinValRaw(i,j,1,1);
            fwrite(&val,sizeof(float),1,out);
          }
        }
      }
      fclose(out);
       printf("Wrote data to file: %s\n",outFileName);
      break;
    case 0:
      //export current (summed) spectrum view

      //write histogram
      for(int32_t j=0;j<S32K;j+=drawing.contractFactor){
        val = getSpBinValOrWeight(0,j,0);
        fwrite(&val,sizeof(float),1,out);
      }
      fclose(out);
      printf("Wrote data to file: %s\n",outFileName);

      break;
    default:
      //export one histogram
      
      spID = exportMode-2;
      if((spID < 0)||(spID >= rawdata.numSpOpened)){
        printf("ERROR: invalid spectrum index for export.\n");
        fclose(out);
        return 2;
      }

      //write histogram
      if(rebin){
        for(int32_t i=0;i<(S32K*drawing.contractFactor);i+=drawing.contractFactor){
          val = getSpBinValRaw(spID,i,drawing.scaleFactor[spID],drawing.contractFactor);
          fwrite(&val,sizeof(float),1,out);
        }
      }else{
        for(int32_t i=0;i<S32K;i++){
          val = getSpBinValRaw(spID,i,1,1);
          fwrite(&val,sizeof(float),1,out);
        }
      }
      fclose(out);
      printf("Wrote data to file: %s\n",outFileName);
      break;
  }

  return 0;
}

//routine to export a plaintext file
//exportMode: 0=write displayed spectrum, 1=write all imported spectra
int exportTXT(const char *filePrefix, const int exportMode, const int rebin){
  
  int spID;
  int maxArraySize;
  float val;
  FILE *out;
  char outFileName[256];

  snprintf(outFileName,256,"%s.txt",filePrefix);
  if((out = fopen(outFileName, "w")) == NULL){ //open the file
    printf("ERROR: Cannot open the output file: %s\n", outFileName);
    printf("The file may not be accesible.\n");
    return 1;
  }
  
  switch(exportMode + (drawing.multiplotMode != MULTIPLOT_SUMMED)){
    case 1:
      //export all

      //write header
      for(int32_t i=0;i<rawdata.numSpOpened;i++){
        fprintf(out,"SPECTRUM%i ",i+1);
      }
      fprintf(out,"\n");

      //get max array size
      maxArraySize = S32K;
      for(int32_t j=S32K-1;j>=0;j--){
        for(int32_t i=0;i<rawdata.numSpOpened;i++){
          if(rawdata.hist[i][j] != 0.){
            maxArraySize = j+1;
            break;
          }
        }
        if(maxArraySize < S32K){
          break;
        }
      }
      
      //write histogram (not applying rebin or scale factors since the whole session with custom views is saved)
      for(int32_t j=0;j<maxArraySize;j++){
        for(int32_t i=0;i<rawdata.numSpOpened;i++){
          val = getSpBinValRaw(i,j,drawing.scaleFactor[i],1);
          fprintf(out,"%f ",val);
        }
        fprintf(out,"\n");
      }

      //write histogram titles
      for(int32_t i=0;i<rawdata.numSpOpened;i++){
        fprintf(out,"TITLE %i %s\n",i+1,rawdata.histComment[i]);
      }

      //write views
      for(int32_t i=0;i<rawdata.numViews;i++){
        fprintf(out,"VIEW %s\nVIEWPAR %u %i\n",rawdata.viewComment[i],rawdata.viewMultiplotMode[i],rawdata.viewNumMultiplotSp[i]);
        fprintf(out,"VIEWSP ");
        for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
          fprintf(out," %u", rawdata.viewMultiPlots[i][j]);
        }
        fprintf(out,"\nVIEWSCALE ");
        for(int32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
          if(rawdata.viewMultiPlots[i][j]<NSPECT){
            fprintf(out," %0.3f", rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]]);
          }
        }
        fprintf(out,"\n");
      }

      //write comments
      for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
        fprintf(out,"COMMENT %i %i %i %f %s\n", rawdata.chanCommentView[i], rawdata.chanCommentSp[i], rawdata.chanCommentCh[i], rawdata.chanCommentVal[i], rawdata.chanComment[i]);
      }

      break;
    case 0:
      //export current (summed) spectrum view
      //write header
      fprintf(out,"SPECTRUM1\n");

      //get max array size
      maxArraySize = S32K;
      for(int32_t j=S32K-1;j>=0;j--){
        if(getSpBinValOrWeight(0,j,0) != 0.){
          maxArraySize = j+1;
          break;
        }
      }

      for(int32_t j=0;j<maxArraySize;j+=drawing.contractFactor){
        val = getSpBinValOrWeight(0,j,0);
        fprintf(out,"%f\n",val);
      }

      //write histogram title
      fprintf(out,"TITLE 1 Summed View\n");

      break;
    default:
      //export one histogram
      
      spID = exportMode-1;
      if((spID < 0)||(spID >= rawdata.numSpOpened)){
        printf("ERROR: invalid spectrum index for export.\n");
        return 2;
      }

      //write header
      fprintf(out,"SPECTRUM%i\n",exportMode);

      //get array size
      maxArraySize = S32K;
      for(int32_t j=S32K-1;j>=0;j--){
        if(rawdata.hist[spID][j] != 0.){
          maxArraySize = j+1;
          break;
        }
      }

      //write histogram
      if(rebin){
        for(int32_t j=0;j<maxArraySize;j+=drawing.contractFactor){
          val = getSpBinValRaw(spID,j,drawing.scaleFactor[spID],drawing.contractFactor);
          fprintf(out,"%f\n",val);
        }
      }else{
        for(int32_t j=0;j<maxArraySize;j++){
          val = getSpBinValRaw(spID,j,drawing.scaleFactor[spID],1);
          fprintf(out,"%f\n",val);
        }
      }

      //write histogram title
      fprintf(out,"TITLE 1 %s\n",rawdata.histComment[spID]);

      break;
  }

  //write calibration parameters
  fprintf(out,"CALPAR %f %f %f %i\n", calpar.calpar[2], calpar.calpar[1], calpar.calpar[2], calpar.calMode);
  fprintf(out,"CALXUNIT %s\n", calpar.calUnit);
  fprintf(out,"CALYUNIT %s\n", calpar.calYUnit);

  fclose(out);
  printf("Wrote data to file: %s\n",outFileName);

  return 0;
}
