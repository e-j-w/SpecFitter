/* © J. Williams, 2020-2022 */

//File contains functions for reading spectra of various formats.
//.jf3 - compressed multiple spectra, with titles and comments
//.txt - column plaintext data (compatible with spreadsheet software), with titles and comments
//.spe - RadWare, with title
//.mca - integer array
//.fmca - float array
//.C - ROOT macro

//function reads an .jf3 file into a double array and returns the array
int readJF3(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  uint8_t ucharBuf, numSpec;
  uint32_t uintBuf;
  FILE *inp;
  uint8_t startNumViews = rawdata.numViews;

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }

  if(fread(&ucharBuf, sizeof(uint8_t), 1, inp)!=1){fclose(inp); return 0;}
  if(ucharBuf==2){
    //version 2 of file format
    if(fread(&ucharBuf, sizeof(uint8_t), 1, inp)!=1){fclose(inp); return 0;}
    numSpec = ucharBuf;
    if(numSpec > 0){

      if((numSpec + outHistStartSp)>NSPECT){
        printf("Cannot open file %s, number of spectra would exceed maximum (%i, %i spectra already open, %i in file)!\n", filename, NSPECT, outHistStartSp, numSpec);
        fclose(inp);
        return -1; //over-import error
      }

      //read labels
      for(uint32_t i=0;i<numSpec;i++){
        if(i<NSPECT){
          if(fread(rawdata.histComment[i+outHistStartSp],sizeof(rawdata.histComment[i+outHistStartSp]), 1, inp)!=1){fclose(inp); return 0;}
        }
      }

      //read calibration parameters
      if(fread(&calpar, sizeof(calpar), 1, inp)!=1){fclose(inp); return 0;}
      if((calpar.calpar[1]==0.0)&&(calpar.calpar[2]==0.0)){
        //invalid calibration, fix parameters
        calpar.calpar[1]=1.0;
      }

      //read comments
      if(fread(&uintBuf, sizeof(uint32_t), 1, inp)!=1){fclose(inp); return 0;}
      if(outHistStartSp == 0){
        //no comments exist already
        rawdata.numChComments = uintBuf;
        for(uint32_t i=0;i<rawdata.numChComments;i++){
          if(i<NCHCOM){
            if(fread(&rawdata.chanCommentView[i],sizeof(rawdata.chanCommentView[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentSp[i],sizeof(rawdata.chanCommentSp[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentCh[i],sizeof(rawdata.chanCommentCh[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentVal[i],sizeof(rawdata.chanCommentVal[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(rawdata.chanComment[i],sizeof(rawdata.chanComment[i]), 1, inp)!=1){fclose(inp); return 0;}
          }
        }
      }else{
        //appending spectra, comments may already exist
        for(uint32_t i=rawdata.numChComments;i<rawdata.numChComments+uintBuf;i++){
          if(i<NCHCOM){
            if(fread(&rawdata.chanCommentView[i],sizeof(rawdata.chanCommentView[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentSp[i],sizeof(rawdata.chanCommentSp[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(rawdata.chanCommentView[i] == 1){
              rawdata.chanCommentSp[i]=(uint8_t)(rawdata.chanCommentSp[i]+startNumViews); //assign to the correct (appended) view
            }else{
              rawdata.chanCommentSp[i]=(uint8_t)(rawdata.chanCommentSp[i]+outHistStartSp); //assign to the correct (appended) spectrum
            }
            //printf("Comment %i went to sp %i\n",i,rawdata.chanCommentSp[i]+1);
            if(fread(&rawdata.chanCommentCh[i],sizeof(rawdata.chanCommentCh[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentVal[i],sizeof(rawdata.chanCommentVal[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(rawdata.chanComment[i],sizeof(rawdata.chanComment[i]), 1, inp)!=1){fclose(inp); return 0;}
          }
        }
        rawdata.numChComments += uintBuf;
        if(rawdata.numChComments > NCHCOM){
          printf("WARNING: over-imported comments.  Truncating.\n");
          rawdata.numChComments = NCHCOM;
        }
      }

      //read views
      if(fread(&uintBuf, sizeof(uint32_t), 1, inp)!=1){fclose(inp); return 0;}
      if(outHistStartSp == 0){
        //no comments exist already
        rawdata.numViews = (uint8_t)uintBuf;
        for(uint32_t i=0;i<rawdata.numViews;i++){
          if(i<MAXNVIEWS){
            memset(rawdata.viewScaleFactor[i],0,sizeof(rawdata.viewScaleFactor[i]));
            if(fread(&rawdata.viewComment[i],sizeof(rawdata.viewComment[i]),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewMultiplotMode[i],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewNumMultiplotSp[i],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            for(uint32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(fread(&rawdata.viewMultiPlots[i][j],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            }
            for(uint32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(rawdata.viewMultiPlots[i][j]<NSPECT){
                if(fread(&rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]],sizeof(double),1,inp)!=1){fclose(inp); return 0;}
              }
            }
          }
        }
      }else{
        //appending spectra, comments may already exist
        for(uint32_t i=rawdata.numViews;i<rawdata.numViews+uintBuf;i++){
          if(i<MAXNVIEWS){
            memset(rawdata.viewScaleFactor[i],0,sizeof(rawdata.viewScaleFactor[i]));
            if(fread(&rawdata.viewComment[i],sizeof(rawdata.viewComment[i]),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewMultiplotMode[i],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewNumMultiplotSp[i],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            for(uint32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(fread(&rawdata.viewMultiPlots[i][j],sizeof(uint8_t),1,inp)!=1){fclose(inp); return 0;}
            }
            for(uint32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(rawdata.viewMultiPlots[i][j]<NSPECT){
                if(fread(&rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]],sizeof(double),1,inp)!=1){fclose(inp); return 0;}
              }
            }
            //realign view data
            for(uint32_t j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if((j+outHistStartSp)<NSPECT){
                rawdata.viewScaleFactor[i][j+outHistStartSp]=rawdata.viewScaleFactor[i][j];
                rawdata.viewMultiPlots[i][j+outHistStartSp]=rawdata.viewMultiPlots[i][j];
              }else{
                printf("WARINING: cannot re-align appended view.\n");
              }
            }
          }
        }
        rawdata.numViews = (uint8_t)(rawdata.numViews+uintBuf);
        if(rawdata.numViews > MAXNVIEWS){
          printf("WARNING: over-imported views.  Truncating.\n");
          rawdata.numViews = MAXNVIEWS;
        }
      }

      //printf("num comments: %i\n",rawdata.numChComments);
      
      //read spectra
      int8_t scharBuf;
      char doneSp;
      uint32_t spInd;
      float val;
      for(uint32_t i=0;i<numSpec;i++){
        
        doneSp = 0;
        spInd = 0;
        while(doneSp==0){
          //read packet header
          if(fread(&scharBuf,sizeof(int8_t), 1, inp)!=1){fclose(inp); return 0;}
          //printf("read packet counter: %i\n",scharBuf);
          if(scharBuf == 0){
            if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in final value
            outHist[i+outHistStartSp][spInd] = (double)val;
            spInd++;
            doneSp = 1; //move on to the next spectrum
          }else if(scharBuf > 0){
            //duplicated entries
            if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in value
            for(uint32_t j=0;j<(uint32_t)scharBuf;j++){
              outHist[i+outHistStartSp][spInd+j] = (double)val;
            }
            spInd += (uint32_t)abs(scharBuf);
          }else{
            //non-duplicated entries
            uint32_t numEntr = (uint32_t)abs(scharBuf);
            for(uint32_t j=0;j<numEntr;j++){
              if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in value
              outHist[i+outHistStartSp][spInd+j] = (double)val;
              //printf("val %f\n",val);
            }
            spInd += numEntr;
          }

        }

        //fill the rest of the histogram
        for(uint32_t j=spInd;j<S32K;j++)
          outHist[i+outHistStartSp][j] = 0.;
      }

    }else{
      printf("ERROR: file %s contains no spectra.\n",filename);
      fclose(inp);
      return 0;
    }
  }else{
    printf("ERROR: file %s has unknown .jf3 file format version (%i).\n",filename,ucharBuf);
    fclose(inp);
    return 0;
  }

  fclose(inp);
  return numSpec;
}

//function reads an .mca file into a double array and returns the number of spectra read in
int readMCA(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  int32_t tmpHist[S32K];
  FILE *inp;

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }

  //get the number of spectra in the .mca file
  uint32_t numSpec = S32K;
  for(uint32_t i = 0; i < numSpec; i++)
    if(fread(tmpHist, S32K * sizeof(int), 1, inp) != 1){
      numSpec = i;
      break;
    }
  fclose(inp);
  //printf("number of spectra in file '%s': %i\n",filename,numSpec);	
  if((outHistStartSp+numSpec)>NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum (%i, %i spectra already open, %i in file)!\n", filename, NSPECT, outHistStartSp, numSpec);
    return -1; //over-import error
  }

  if((inp = fopen(filename, "r")) != NULL){ //reopen the file
    for(uint32_t i = outHistStartSp; i < (outHistStartSp+numSpec); i++){
      if(fread(tmpHist, S32K * sizeof(int), 1, inp) != 1){
        printf("ERROR: Cannot read spectrum %i from the .mca file: %s\n", i, filename);
        printf("Verify that the format and number of spectra in the file are correct.\n");
        exit(-1);
      }
      else{
        for(uint32_t j = 0; j < S32K; j++){
          outHist[i][j] = (double)tmpHist[j];
        }
      }
      snprintf(rawdata.histComment[i],256,"Spectrum %i of %s",i-outHistStartSp+1,basename((char*)filename));
    }
  }

  fclose(inp);
  return (int)numSpec;
}

//function reads an .fmca file into a double array and returns the number of spectra read in
int readFMCA(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  float tmpHist[S32K];
  FILE *inp;

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1); //over-import error
  }

  //get the number of spectra in the .fmca file
  uint32_t numSpec = S32K;
  for(uint32_t i = 0; i < numSpec; i++){
    if(fread(tmpHist, S32K * sizeof(float), 1, inp) != 1){
      numSpec = i;
      break;
    }
  }
  fclose(inp);
  //printf("number of spectra in file '%s': %i\n",filename,numSpec);
  if((outHistStartSp+numSpec)>NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum (%i, %i spectra already open, %i in file)!\n", filename, NSPECT, outHistStartSp, numSpec);
    return -1;
  }

  if((inp = fopen(filename, "r")) != NULL){ //reopen the file
    for(uint32_t i = outHistStartSp; i < (outHistStartSp+numSpec); i++){
      if(fread(tmpHist, S32K * sizeof(float), 1, inp) != 1){
        printf("ERROR: Cannot read spectrum %i from the .fmca file: %s\n", i, filename);
        printf("Verify that the format and number of spectra in the file are correct.\n");
        exit(-1);
      }else{
        for(uint32_t j = 0; j < S32K; j++)
          outHist[i][j] = (double)tmpHist[j];
      }
      snprintf(rawdata.histComment[i],256,"Spectrum %i of %s",i-outHistStartSp+1,basename((char*)filename));
    }
  }

  fclose(inp);
  return (int)numSpec;
}

//function reads an .chn (Maestro) file into the output hist and returns the number of spectra read in
int readCHN(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  char acqTime[8], month[4];
  short fType;
  unsigned short chanOffset, mcaNum, numCh, segment;
  int32_t livetime, realtime, chData;
  FILE *inp;
  int32_t hist[S32K];
  memset(hist,0,sizeof(hist));

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }else{
    if(fread(&fType,sizeof(fType),1,inp) != 1) return 0;
    if(fType!=-1){
      printf("ERROR: Input file %s is not a valid .chn file.\n",filename);
      exit(-1);
    }
      
    //read in the .chn file header and do absolutely nothing with it
    if(fread(&mcaNum, sizeof(mcaNum), 1, inp) != 1) return 0;         // MCA #
    if(fread(&segment, sizeof(segment), 1, inp) != 1) return 0;         // seg #
    if(fread(acqTime, sizeof(char), 2, inp) != 2) return 0;            // start time
    if(fread(&realtime, sizeof(realtime), 1, inp) != 1) return 0;       // real time, 20 ms ticks
    if(fread(&livetime, sizeof(livetime), 1, inp) != 1) return 0;       // live time, 20 ms ticks
    if(fread(acqTime, sizeof(char), 2, inp) != 2) return 0;            // start day
    if(fread(month, sizeof(char), 3, inp) != 3) return 0;               // start month
    if(fread(acqTime, sizeof(char), 2, inp) != 2) return 0;            // start year
    if(fread(acqTime, sizeof(char), 1, inp) != 1) return 0;            // century
    if(fread(acqTime, sizeof(char), 2, inp) != 2) return 0;            // hour
    if(fread(acqTime, sizeof(char), 2, inp) != 2) return 0;            // minute
    if(fread(&chanOffset, sizeof(chanOffset), 1, inp) != 1) return 0; // offset
    if(fread(&numCh, sizeof(numCh), 1, inp) != 1) return 0;     // # channels

    //read in histogram data from the .chn file
    for(int32_t i=0;i<numCh;i++){
      if(i<S32K){
        if(fread(&chData, sizeof(chData), 1, inp) != 1) return 0;
        hist[i]=chData;
      }
    }
  }

  for(int32_t i=0;i<S32K;i++){
    outHist[outHistStartSp][i] = (double)hist[i];
  }
  snprintf(rawdata.histComment[outHistStartSp],256,"%s",basename((char*)filename));
  
  fclose(inp);
  return 1;

}

//function reads an .spe (Maestro) file into the output hist and returns the number of spectra read in
int readSPEM(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  uint32_t numChsRead = 0;
  char str[1024];
  char *tok;
  FILE *inp;
  int32_t numCh = -1; // the detected number of channels

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }else{
    while(!(feof(inp))){ //go until the end of file is reached
      if(numChsRead<S32K){
        if(fgets(str,1024,inp)!=NULL){ //get an entire line
          str[strcspn(str, "\r\n")] = 0;//strips newline characters from the string read by fgets
          if(numCh>0){
            if(numChsRead < (uint32_t)numCh){
              outHist[outHistStartSp][numChsRead] = atof(str);
              numChsRead++;
            }
          }else if(strcmp(str,"$DATA:")==0){
            if(fgets(str,1024,inp)!=NULL){ //get the next line
              str[strcspn(str, "\r\n")] = 0;//strips newline characters from the string read by fgets
              tok = strtok(str," ");
              if(tok!=NULL){
                if(strcmp(tok,"0")==0){
                  tok = strtok(NULL," ");
                  if(tok!=NULL){
                    numCh = atoi(tok);
                  }
                }else{
                  printf("ERROR: unexpected header format in Maestro file, number of channels not specified.\n");
                  return 0;
                }
              }
            }
          }
          
        }
      }else{
        break;
      }
      
    }
  }

  //check for import errors
  if(numChsRead == 0){
    printf("ERROR: Empty input file: %s\n", filename);
    fclose(inp);
    return 0;
  }

  for(uint32_t i=numChsRead;i<S32K;i++){
    outHist[outHistStartSp][i]=0.;
  }
  snprintf(rawdata.histComment[outHistStartSp],256,"%s",basename((char*)filename));

  fclose(inp);
  return 1;
}

//function reads an .spe file into a double array and returns the array
int readSPE(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  float inpHist[4096];
  FILE *inp;

  //radware file header info
  char spLabel[8], header[24];
  int32_t intBuf = 1;

  if((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }

  //read .spe header
  if(fread(&intBuf,sizeof(int32_t),1,inp)!=1) return 0;
  if(fread(&spLabel,sizeof(spLabel),1,inp)!=1) return 0;
  if(fread(header, 24, 1, inp) != 1){
    printf("ERROR: Cannot read header from the .spe file: %s\n", filename);
    printf("Verify that the format of the file is correct.\n");
    fclose(inp);
    return 0;
  }
  uint32_t numElementsRead = (uint32_t)fread(inpHist, sizeof(float), 4096, inp);
  if(numElementsRead < 1){
    printf("ERROR: Cannot read spectrum from the .spe file: %s\n", filename);
    printf("fread code: %u\n",numElementsRead);
    printf("Verify that the format of the file is correct.\n");
    fclose(inp);
    return 0;
  }

  if(outHistStartSp>=NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum (%i, %i spectra already open, 1 in file)!\n", filename, NSPECT, outHistStartSp);
    return -1; //over-import error
  }

  //convert input data to double
  for(uint32_t i = 0; i < numElementsRead; i++)
    outHist[outHistStartSp][i] = (double)inpHist[i];
  for(uint32_t i = numElementsRead; i < S32K; i++)
    outHist[outHistStartSp][i] = 0.;

  gchar *label = g_convert(spLabel, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL); //can get weirdly encoded junk, make sure it is properly converted to UTF-8
  snprintf(rawdata.histComment[outHistStartSp],256,"%s %s",label,basename((char*)filename));
  g_free(label);

  fclose(inp);
  return 1;
}

int readTXT(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  uint32_t numElementsRead = 0;
  char str[1024], str2[1024];
  char *tok;
  FILE *inp;
  double num[NSPECT];
  uint8_t numColumns = 0; // the detected number of columns in the first line
  uint8_t startNumViews = rawdata.numViews;

  if ((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }else{
    while(!(feof(inp))){ //go until the end of file is reached
      if(numElementsRead<S32K){
        if(fgets(str,1024,inp)!=NULL){ //get an entire line
          strncpy(str2,str,1024);
          uint8_t numLineEntries = 0;
          tok = strtok(str2," ");
          if((strcmp(tok,"SPECTRUM1")==0)||(strcmp(tok,"TITLE")==0)||(strcmp(tok,"VIEW")==0)||(strcmp(tok,"VIEWPAR")==0)||(strcmp(tok,"VIEWSP")==0)||(strcmp(tok,"VIEWSCALE")==0)||(strcmp(tok,"COMMENT")==0)||(strcmp(tok,"CALPAR")==0)||(strcmp(tok,"CALXUNIT")==0)||(strcmp(tok,"CALYUNIT")==0)){
            numLineEntries = 0;
          }else{
            //line is data
            while((tok!=NULL)&&((strcmp(tok,"")!=0))){
              if(numLineEntries<NSPECT){
                tok[strcspn(tok, "\r\n")] = 0;//strips newline characters from the string
                num[numLineEntries] = atof(tok);
                //printf("%s, numLineEntries: %i, num: %f\n",tok,numLineEntries, num[numLineEntries]);
                numLineEntries++;
                tok = strtok(NULL," ");
              }else{
                numLineEntries++;
                break;
              }
            }
          }
          if(numLineEntries > 0){
            if(numColumns == 0){
              numColumns = numLineEntries;
              //set default histogram titles
              for(uint32_t i=0;i<numColumns;i++){
                snprintf(rawdata.histComment[outHistStartSp+i],256,"Spectrum %i of %s",i,basename((char*)filename));
              }
            }else if(numLineEntries != numColumns){
              printf("ERROR: inconsistent number of columns (%i) in line %i of file: %s\n",numLineEntries,numElementsRead,filename);
              fclose(inp);
              return 0;
            }
            //printf("numLineEntries: %u, numColumns: %u\n",numLineEntries,numColumns);
            for(uint32_t i=0;i<numLineEntries;i++){
              outHist[outHistStartSp+i][numElementsRead]=num[i];
            }
            numElementsRead++;
          }else{
            tok = strtok(str," ");
            if(tok!=NULL){
              if(strcmp(tok,"COMMENT")==0){
                if(rawdata.numChComments < NCHCOM){
                  tok = strtok(NULL," ");
                  if(tok!=NULL){
                    rawdata.chanCommentView[rawdata.numChComments] = (uint8_t)atoi(tok);
                    tok = strtok(NULL," ");
                    if(tok!=NULL){
                      rawdata.chanCommentSp[rawdata.numChComments] = (uint8_t)atoi(tok);
                      if(outHistStartSp > 0){
                        if(rawdata.chanCommentView[rawdata.numChComments] == 1){
                          rawdata.chanCommentSp[rawdata.numChComments]=(uint8_t)(rawdata.chanCommentSp[rawdata.numChComments]+startNumViews); //assign to the correct (appended) view
                        }else{
                          rawdata.chanCommentSp[rawdata.numChComments]=(uint8_t)(rawdata.chanCommentSp[rawdata.numChComments]+outHistStartSp); //assign to the correct (appended) spectrum
                        }
                      }
                      tok = strtok(NULL," ");
                      if(tok!=NULL){
                        rawdata.chanCommentCh[rawdata.numChComments] = atoi(tok);
                        tok = strtok(NULL," ");
                        if(tok!=NULL){
                          rawdata.chanCommentVal[rawdata.numChComments] = (float)atof(tok);
                          tok = strtok(NULL,""); //get the rest of the string
                          if(tok!=NULL){
                            strncpy(rawdata.chanComment[rawdata.numChComments],tok,sizeof(rawdata.chanComment[rawdata.numChComments])-1);
                            rawdata.chanComment[rawdata.numChComments][strcspn(rawdata.chanComment[rawdata.numChComments], "\r\n")] = 0;//strips newline characters from the string
                            rawdata.numChComments += 1;
                          }
                        }
                      }
                    }
                  }
                }
              }else if(strcmp(tok,"TITLE")==0){
                tok = strtok(NULL," ");
                if(tok!=NULL){
                  int32_t spID = atoi(tok) - 1 + (int)outHistStartSp;
                  if((spID >= 0)&&(spID < NSPECT)){
                    tok = strtok(NULL,""); //get the rest of the string
                    if(tok!=NULL){
                      strncpy(rawdata.histComment[spID],tok,sizeof(rawdata.histComment[spID])-1);
                      rawdata.histComment[spID][strcspn(rawdata.histComment[spID], "\r\n")] = 0;//strips newline characters from the string
                    }
                  }						
                }
              }else if(strcmp(tok,"VIEW")==0){
                if(rawdata.numViews < MAXNVIEWS){
                  tok = strtok(NULL,""); //get the rest of the string
                  if(tok!=NULL){
                    strncpy(rawdata.viewComment[rawdata.numViews],tok,sizeof(rawdata.viewComment[rawdata.numViews])-1);
                    rawdata.viewComment[rawdata.numViews][strcspn(rawdata.viewComment[rawdata.numViews], "\r\n")] = 0;//strips newline characters from the string
                    if(fgets(str,256,inp)!=NULL){ //get an entire line
                      tok = strtok(str," ");
                      if(tok!=NULL){
                        if(strcmp(tok,"VIEWPAR")==0){
                          tok = strtok(NULL," ");
                          if(tok!=NULL){
                            rawdata.viewMultiplotMode[rawdata.numViews] = (uint8_t)atoi(tok);
                            tok = strtok(NULL," ");
                            if(tok!=NULL){
                              rawdata.viewNumMultiplotSp[rawdata.numViews] = (uint8_t)atoi(tok);
                              if(fgets(str,256,inp)!=NULL){ //get an entire line
                                tok = strtok(str," ");
                                if(tok!=NULL){
                                  if(strcmp(tok,"VIEWSP")==0){
                                    for(uint32_t i=0;i<rawdata.viewNumMultiplotSp[rawdata.numViews];i++){
                                      tok = strtok(NULL," ");
                                      if(tok!=NULL){
                                        rawdata.viewMultiPlots[rawdata.numViews][i] = (uint8_t)atoi(tok);
                                      }
                                    }
                                    if(fgets(str,256,inp)!=NULL){ //get an entire line
                                      tok = strtok(str," ");
                                      if(tok!=NULL){
                                        if(strcmp(tok,"VIEWSCALE")==0){
                                          for(uint32_t i=0;i<rawdata.viewNumMultiplotSp[rawdata.numViews];i++){
                                            if(rawdata.viewMultiPlots[rawdata.numViews][i]<NSPECT){
                                              tok = strtok(NULL," ");
                                              if(tok!=NULL){
                                                rawdata.viewScaleFactor[rawdata.numViews][rawdata.viewMultiPlots[rawdata.numViews][i]] = atof(tok);
                                              }
                                            }
                                          }
                                          rawdata.numViews = (uint8_t)(rawdata.numViews+1);
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }else if(strcmp(tok,"CALPAR")==0){
                tok = strtok(NULL," ");
                if(tok!=NULL){
                  calpar.calpar[2] = (float)atof(tok);
                  tok = strtok(NULL," ");
                  if(tok!=NULL){
                    calpar.calpar[1] = (float)atof(tok);
                    tok = strtok(NULL," ");
                    if(tok!=NULL){
                      calpar.calpar[2] = (float)atof(tok);
                      tok = strtok(NULL," ");
                      if(tok!=NULL){
                        calpar.calMode = (uint8_t)atoi(tok);
                      }
                    }
                  }
                }
                if((calpar.calpar[1]==0.0)&&(calpar.calpar[2]==0.0)){
                  //invalid calibration, fix parameters
                  calpar.calpar[1]=1.0;
                }
              }else if(strcmp(tok,"CALXUNIT")==0){
                tok = strtok(NULL,""); //get the rest of the string
                if(tok!=NULL){
                  strncpy(calpar.calUnit,tok,sizeof(calpar.calUnit)-1);
                  calpar.calUnit[strcspn(calpar.calUnit, "\r\n")] = 0;//strips newline characters from the string
                }						
              }else if(strcmp(tok,"CALYUNIT")==0){
                tok = strtok(NULL,""); //get the rest of the string
                if(tok!=NULL){
                  strncpy(calpar.calYUnit,tok,sizeof(calpar.calYUnit)-1);
                  calpar.calYUnit[strcspn(calpar.calYUnit, "\r\n")] = 0;//strips newline characters from the string
                }						
              }
            }
          }
        }
      }else{
        break;
      }
      
    }
  }

  //check for import errors
  if(numElementsRead == 0){
    printf("ERROR: Empty input file: %s\n", filename);
    fclose(inp);
    return 0;
  }

  for(uint32_t i=0;i<numColumns;i++){
    for(uint32_t j=numElementsRead;j<S32K;j++){
      outHist[outHistStartSp+i][j]=0.;
    }
  }

  fclose(inp);
  return (int)numColumns;
}

//function reads an .C ROOT macro file into a double array and returns the number of spectra read in
int readROOT(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){

  FILE *inp;

  if ((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1); //over-import error
  }

  char *tok;
  char histType = 0; //0=no hist, 1=TH1F,TH1D,TH1I
  uint32_t histNum=0;
  int32_t ind;
  double val;
  char *str = malloc(1024);
  char *histName = malloc(256);
  strncpy(histName,"",256);

  //read the input file
  if(fgets(str,1024,inp)!=NULL){
    while((!feof(inp))&&(fgets(str,1024,inp))){ //check for end of file and read next line
      //printf("%s",str); //print the line
      tok = strtok (str," *->(),");
      if(tok!=NULL){
        if(histType>0){
          if(outHistStartSp+histNum<=NSPECT){
            if(strncmp(tok,histName,256)==0){
              if(histType==1){
                //parse TH1F
                tok = strtok (NULL," *->(),");
                if((tok!=NULL)&&(strncmp(tok,"SetBinContent",256)==0)){
                  tok = strtok (NULL," *->(),");
                  if(tok!=NULL){
                    ind=atoi(tok);
                    tok = strtok (NULL," *->(),");
                    if(tok!=NULL){
                      val=atof(tok);
                      //printf("Read bin %i with value %f\n",ind,val);
                      if((ind>=0)&&(ind<S32K)){
                        outHist[outHistStartSp+histNum-1][ind]=val;
                      }
                    }
                  }
                }
              }
            }
          }
        }

        if((strncmp(tok,"TH1F",256)==0)||(strncmp(tok,"TH1D",256)==0)||(strncmp(tok,"TH1I",256)==0)){
          //printf("1-D histogram (floating point) found.\n");
          histType = 1;
          tok = strtok (NULL," *");
          if(tok!=NULL){
            //printf("Histogram name: %s\n",tok);
            strncpy(histName,tok,255);
            histNum++;
            if(outHistStartSp+histNum<=NSPECT){
              //get rid of any previous histogram values (for when overwriting other histos)
              memset(outHist[outHistStartSp+histNum-1],0,sizeof(outHist[outHistStartSp+histNum-1]));
              snprintf(rawdata.histComment[outHistStartSp+histNum-1],256,"Spectrum %i of %s",histNum,basename((char*)filename));
            }
          }
        }
      }
      
    }
  }else{
    printf("Specified file %s has nothing in it.\n",filename);
    fclose(inp);
    free(histName);
    free(str);
    return 0;
  }

  fclose(inp);
  free(histName);
  free(str);
  return (int)histNum;
}

//reads a file containing spectrum data into an array
//returns the number of spectra read (0 if reading fails, -1 if too many files)
int readSpectrumDataFile(const char *filename, double outHist[NSPECT][S32K], const uint32_t outHistStartSp){
  
  int32_t numSpec = 0;

  const char *dot = strrchr(filename, '.'); //get the file extension
  if(dot==NULL){
    return -2; //invalid file type
  }
  if(strcmp(dot + 1, "mca") == 0){
    numSpec = readMCA(filename, outHist, outHistStartSp);
  }else if(strcmp(dot + 1, "fmca") == 0){
    numSpec = readFMCA(filename, outHist, outHistStartSp);
  }else if((strcmp(dot + 1, "spe") == 0)||(strcmp(dot + 1, "Spe") == 0)){
    printf("Trying Maestro SPE format.\n");
    numSpec = readSPEM(filename, outHist, outHistStartSp);
    if(numSpec != 1){
      printf("Trying RadWare SPE format.\n");
      numSpec = readSPE(filename, outHist, outHistStartSp);
    }
  }else if((strcmp(dot + 1, "chn") == 0)||(strcmp(dot + 1, "Chn") == 0)){
    numSpec = readCHN(filename, outHist, outHistStartSp);
  }else if(strcmp(dot + 1, "txt") == 0){
    numSpec = readTXT(filename, outHist, outHistStartSp);
  }else if(strcmp(dot + 1, "C") == 0){
    numSpec = readROOT(filename, outHist, outHistStartSp);
  }else if(strcmp(dot + 1, "jf3") == 0){
    numSpec = readJF3(filename, outHist, outHistStartSp);
  }else{
    //printf("Improper format of input file: %s\n", filename);
    //printf("Supported file formats are: jf3 (.jf3), plaintext (.txt) integer array (.mca), float array (.fmca), radware (.spe), or ROOT macro (.C) files.\n");
    return -2;
  }

  if((numSpec==-1)||(((int)outHistStartSp+numSpec)>NSPECT)){
    return -1; //too many spectra opened
  }

  if(numSpec >= 0){
    printf("Opened file: %s, number of spectra read in: %u\n", filename, numSpec);

    //discard empty spectra
    if(rawdata.dropEmptySpectra){
      int32_t checkedSpCount = 0;
      uint32_t passedSpCount = 0;
      int32_t passed = 0;
      while(checkedSpCount < numSpec){
        passed = 0;
        for(uint32_t i=0;i<S32K;i++){
          if(outHist[outHistStartSp+passedSpCount][i] != 0.){
            //printf("Passed spectrum %i\n",checkedSpCount);
            passed = 1;
            passedSpCount++;
            break;
          }
        }
        if(passed == 0){
          //spectrum was empty, overwrite it
          for(uint32_t i=passedSpCount;i<(uint32_t)(numSpec-1);i++){
            memcpy(outHist[outHistStartSp+i],outHist[outHistStartSp+i+1],sizeof(outHist[i+1]));
          }
        }
        checkedSpCount++;
      }
      int32_t dropCount = numSpec-(int)passedSpCount;
      if(dropCount>0)
        printf("Dropped %i empty spectra.\n",dropCount);
      return (int)passedSpCount;
    }
  }

  return numSpec;
}
