/* J. Williams, 2020 */

//File contains functions for reading spectra of various formats.
//.jf3 - compressed multiple spectra, with titles and comments
//.txt - column plaintext data (compatible with spreadsheet software), with titles and comments
//.spe - RadWare, with title
//.mca - integer array
//.fmca - float array
//.C - ROOT macro

//function reads an .jf3 file into a double array and returns the array
int readJF3(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  unsigned int i,j;
  unsigned char ucharBuf, numSpec;
  unsigned int uintBuf;
  FILE *inp;
  unsigned char startNumViews = rawdata.numViews;

  if ((inp = fopen(filename, "r")) == NULL) //open the file
  {
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }

  if(fread(&ucharBuf, sizeof(unsigned char), 1, inp)!=1){fclose(inp); return 0;}
  if(ucharBuf==1){
    //version 0 of file format
    if(fread(&ucharBuf, sizeof(unsigned char), 1, inp)!=1){fclose(inp); return 0;}
    numSpec = ucharBuf;
    if(numSpec > 0){

      if((numSpec + outHistStartSp)>NSPECT){
        printf("Cannot open file %s, number of spectra would exceed maximum!\n", filename);
        fclose(inp);
        return -1; //over-import error
      }

      //read labels
      for(i=0;i<numSpec;i++){
        if(i<NSPECT){
          if(fread(rawdata.histComment[i+outHistStartSp],sizeof(rawdata.histComment[i+outHistStartSp]), 1, inp)!=1){fclose(inp); return 0;}
        }
      }

      //read calibration parameters
      if(fread(&calpar, sizeof(calpar), 1, inp)!=1){fclose(inp); return 0;}

      //read comments
      if(fread(&uintBuf, sizeof(unsigned int), 1, inp)!=1){fclose(inp); return 0;}
      if(outHistStartSp == 0){
        //no comments exist already
        rawdata.numChComments = uintBuf;
        for(i=0;i<rawdata.numChComments;i++){
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
        for(i=rawdata.numChComments;i<rawdata.numChComments+uintBuf;i++){
          if(i<NCHCOM){
            if(fread(&rawdata.chanCommentView[i],sizeof(rawdata.chanCommentView[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.chanCommentSp[i],sizeof(rawdata.chanCommentSp[i]), 1, inp)!=1){fclose(inp); return 0;}
            if(rawdata.chanCommentView[i] == 1){
              rawdata.chanCommentSp[i]+=startNumViews; //assign to the correct (appended) view
            }else{
              rawdata.chanCommentSp[i]+=(unsigned char)outHistStartSp; //assign to the correct (appended) spectrum
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
      if(fread(&uintBuf, sizeof(unsigned int), 1, inp)!=1){fclose(inp); return 0;}
      if(outHistStartSp == 0){
        //no comments exist already
        rawdata.numViews = (unsigned char)uintBuf;
        for(i=0;i<rawdata.numViews;i++){
          if(i<MAXNVIEWS){
            memset(rawdata.viewScaleFactor[i],0,sizeof(rawdata.viewScaleFactor[i]));
            if(fread(&rawdata.viewComment[i],sizeof(rawdata.viewComment[i]),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewMultiplotMode[i],sizeof(unsigned char),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewNumMultiplotSp[i],sizeof(int),1,inp)!=1){fclose(inp); return 0;}
            for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(fread(&rawdata.viewMultiPlots[i][j],sizeof(unsigned char),1,inp)!=1){fclose(inp); return 0;}
            }
            for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if((rawdata.viewMultiPlots[i][j]>=0)&&(rawdata.viewMultiPlots[i][j]<NSPECT)){
                if(fread(&rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]],sizeof(double),1,inp)!=1){fclose(inp); return 0;}
              }
            }
          }
        }
      }else{
        //appending spectra, comments may already exist
        for(i=rawdata.numViews;i<rawdata.numViews+uintBuf;i++){
          if(i<MAXNVIEWS){
            memset(rawdata.viewScaleFactor[i],0,sizeof(rawdata.viewScaleFactor[i]));
            if(fread(&rawdata.viewComment[i],sizeof(rawdata.viewComment[i]),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewMultiplotMode[i],sizeof(unsigned char),1,inp)!=1){fclose(inp); return 0;}
            if(fread(&rawdata.viewNumMultiplotSp[i],sizeof(int),1,inp)!=1){fclose(inp); return 0;}
            for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if(fread(&rawdata.viewMultiPlots[i][j],sizeof(unsigned char),1,inp)!=1){fclose(inp); return 0;}
            }
            for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if((rawdata.viewMultiPlots[i][j]>=0)&&(rawdata.viewMultiPlots[i][j]<NSPECT)){
                if(fread(&rawdata.viewScaleFactor[i][rawdata.viewMultiPlots[i][j]],sizeof(double),1,inp)!=1){fclose(inp); return 0;}
              }
            }
            //realign view data
            for(j=0;j<rawdata.viewNumMultiplotSp[i];j++){
              if((j+outHistStartSp)<NSPECT){
                rawdata.viewScaleFactor[i][j+outHistStartSp]=rawdata.viewScaleFactor[i][j];
                rawdata.viewMultiPlots[i][j+outHistStartSp]=rawdata.viewMultiPlots[i][j];
              }else{
                printf("WARINING: cannot re-align appended view.\n");
              }
            }
          }
        }
        rawdata.numViews += (unsigned char)uintBuf;
        if(rawdata.numViews > MAXNVIEWS){
          printf("WARNING: over-imported views.  Truncating.\n");
          rawdata.numViews = MAXNVIEWS;
        }
      }

      //printf("num comments: %i\n",rawdata.numChComments);
      
      //read spectra
      signed char scharBuf;
      char doneSp;
      unsigned int spInd;
      float val;
      for(i=0;i<numSpec;i++){
        
        doneSp = 0;
        spInd = 0;
        while(doneSp==0){
          //read packet header
          if(fread(&scharBuf,sizeof(signed char), 1, inp)!=1){fclose(inp); return 0;}
          //printf("read packet counter: %i\n",scharBuf);
          if(scharBuf == 0){
            if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in final value
            outHist[i+outHistStartSp][spInd] = (double)val;
            spInd++;
            doneSp = 1; //move on to the next spectrum
          }else if(scharBuf > 0){
            //duplicated entries
            if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in value
            for(j=0;j<scharBuf;j++){
              outHist[i+outHistStartSp][spInd+j] = (double)val;
            }
            spInd += (unsigned int)scharBuf;
          }else{
            //non-duplicated entries
            unsigned int numEntr = abs(scharBuf);
            for(j=0;j<numEntr;j++){
              if(fread(&val,sizeof(float), 1, inp)!=1){fclose(inp); return 0;} //read in value
              outHist[i+outHistStartSp][spInd+j] = (double)val;
              //printf("val %f\n",val);
            }
            spInd += numEntr;
          }

        }

        //fill the rest of the histogram
        for(j=spInd;j<S32K;j++)
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
int readMCA(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  unsigned int i, j;
  int tmpHist[S32K];
  FILE *inp;

  if ((inp = fopen(filename, "r")) == NULL) //open the file
  {
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }

  //get the number of spectra in the .mca file
  unsigned int numSpec = S32K;
  for (i = 0; i < numSpec; i++)
    if (fread(tmpHist, S32K * sizeof(int), 1, inp) != 1)
    {
      numSpec = i;
      break;
    }
  fclose(inp);
  //printf("number of spectra in file '%s': %i\n",filename,numSpec);	
  if((outHistStartSp+numSpec)>=NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum!\n", filename);
    fclose(inp);
    return -1; //over-import error
  }

  if ((inp = fopen(filename, "r")) != NULL){ //reopen the file
    for (i = outHistStartSp; i < (outHistStartSp+numSpec); i++)
    {
      if (fread(tmpHist, S32K * sizeof(int), 1, inp) != 1)
      {
        printf("ERROR: Cannot read spectrum %i from the .mca file: %s\n", i, filename);
        printf("Verify that the format and number of spectra in the file are correct.\n");
        exit(-1);
      }
      else{
        for (j = 0; j < S32K; j++){
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
int readFMCA(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  unsigned int i, j;
  float tmpHist[S32K];
  FILE *inp;

  if ((inp = fopen(filename, "r")) == NULL) //open the file
  {
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1); //over-import error
  }

  //get the number of spectra in the .fmca file
  unsigned int numSpec = S32K;
  for (i = 0; i < numSpec; i++)
    if (fread(tmpHist, S32K * sizeof(float), 1, inp) != 1)
    {
      numSpec = i;
      break;
    }
  fclose(inp);
  //printf("number of spectra in file '%s': %i\n",filename,numSpec);
  if((outHistStartSp+numSpec)>=NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum!\n", filename);
    fclose(inp);
    return -1;
  }

  if ((inp = fopen(filename, "r")) != NULL){ //reopen the file
    for (i = outHistStartSp; i < (outHistStartSp+numSpec); i++)
    {
      if (fread(tmpHist, S32K * sizeof(float), 1, inp) != 1)
      {
        printf("ERROR: Cannot read spectrum %i from the .fmca file: %s\n", i, filename);
        printf("Verify that the format and number of spectra in the file are correct.\n");
        exit(-1);
      }
      else
      {
        for (j = 0; j < S32K; j++)
          outHist[i][j] = (double)tmpHist[j];
      }
      snprintf(rawdata.histComment[i],256,"Spectrum %i of %s",i-outHistStartSp+1,basename((char*)filename));
    }
  }

  fclose(inp);
  return (int)numSpec;
}

//function reads an .spe file into a double array and returns the array
int readSPE(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  unsigned int i;
  float inpHist[4096];
  FILE *inp;

  //radware file header info
  char spLabel[8], header[24];
  int32_t intBuf = 1;

  if ((inp = fopen(filename, "r")) == NULL) //open the file
  {
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
  unsigned int numElementsRead = (unsigned int)fread(inpHist, sizeof(float), 4096, inp);
  if(numElementsRead < 1){
    printf("ERROR: Cannot read spectrum from the .spe file: %s\n", filename);
    printf("fread code: %u\n",numElementsRead);
    printf("Verify that the format of the file is correct.\n");
    fclose(inp);
    return 0;
  }

  if(outHistStartSp>=NSPECT){
    printf("Cannot open file %s, number of spectra would exceed maximum!\n", filename);
    fclose(inp);
    return -1; //over-import error
  }

  //convert input data to double
  for (i = 0; i < numElementsRead; i++)
    outHist[outHistStartSp][i] = (double)inpHist[i];
  for (i = numElementsRead; i < S32K; i++)
    outHist[outHistStartSp][i] = 0.;

  gchar *label = g_convert(spLabel, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL); //can get weirdly encoded junk, make sure it is properly converted to UTF-8
  snprintf(rawdata.histComment[outHistStartSp],256,"%s %s",label,basename((char*)filename));
  g_free(label);

  fclose(inp);
  return 1;
}

int readTXT(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  unsigned int i,j;
  unsigned int numElementsRead = 0;
  char str[1024], str2[1024];
  char *tok;
  FILE *inp;
  double num[NSPECT];
  int numColumns = 0; // the detected number of columns in the first line
  unsigned char startNumViews = rawdata.numViews;

  if ((inp = fopen(filename, "r")) == NULL){ //open the file
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1);
  }else{
    while(!(feof(inp)))//go until the end of file is reached
      {
        if(numElementsRead<S32K){
          if(fgets(str,1024,inp)!=NULL){ //get an entire line
            strncpy(str2,str,1024);
            int numLineEntries = 0;
            tok = strtok(str2," ");
            if((strcmp(tok,"SPECTRUM1")==0)||(strcmp(tok,"TITLE")==0)||(strcmp(tok,"VIEW")==0)||(strcmp(tok,"VIEWPAR")==0)||(strcmp(tok,"VIEWSP")==0)||(strcmp(tok,"VIEWSCALE")==0)||(strcmp(tok,"COMMENT")==0)||(strcmp(tok,"CALPAR")==0)||(strcmp(tok,"CALXUNIT")==0)||(strcmp(tok,"CALYUNIT")==0)){
              numLineEntries = 0;
            }else{
              //line is data
              while(tok!=NULL){
                if(numLineEntries<NSPECT){
                  num[numLineEntries] = atof(tok);
                  //printf("numLineEntries: %i, num: %f\n",numLineEntries, num[numLineEntries]);
                  numLineEntries++;
                  tok = strtok(NULL," ");
                }else{
                  numLineEntries++;
                  break;
                }
              }
              numLineEntries--;
            }
            //getc(stdin);
            if(numLineEntries > 0){
              if(numColumns == 0){
                numColumns = numLineEntries;
                //set default histogram titles
                for(i=0;i<numColumns;i++){
                  snprintf(rawdata.histComment[outHistStartSp+i],256,"Spectrum %i of %s",i,basename((char*)filename));
                }
              }else if(numLineEntries != numColumns){
                printf("ERROR: inconsistent number of columns (%i) in line %i of file: %s\n",numLineEntries,numElementsRead,filename);
                fclose(inp);
                return 0;
              }
              for(i=0;i<numLineEntries;i++){
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
                      rawdata.chanCommentView[rawdata.numChComments] = (unsigned char)atoi(tok);
                      tok = strtok(NULL," ");
                      if(tok!=NULL){
                        rawdata.chanCommentSp[rawdata.numChComments] = (unsigned char)atoi(tok);
                        if(outHistStartSp > 0){
                          if(rawdata.chanCommentView[rawdata.numChComments] == 1){
                            rawdata.chanCommentSp[rawdata.numChComments]+=startNumViews; //assign to the correct (appended) view
                          }else{
                            rawdata.chanCommentSp[rawdata.numChComments]+=(unsigned char)outHistStartSp; //assign to the correct (appended) spectrum
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
                    int spID = atoi(tok) - 1 + (int)outHistStartSp;
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
                              rawdata.viewMultiplotMode[rawdata.numViews] = (unsigned char)atoi(tok);
                              tok = strtok(NULL," ");
                              if(tok!=NULL){
                                rawdata.viewNumMultiplotSp[rawdata.numViews] = atoi(tok);
                                if(fgets(str,256,inp)!=NULL){ //get an entire line
                                  tok = strtok(str," ");
                                  if(tok!=NULL){
                                    if(strcmp(tok,"VIEWSP")==0){
                                      for(i=0;i<rawdata.viewNumMultiplotSp[rawdata.numViews];i++){
                                        tok = strtok(NULL," ");
                                        if(tok!=NULL){
                                          rawdata.viewMultiPlots[rawdata.numViews][i] = (unsigned char)atoi(tok);
                                        }
                                      }
                                      if(fgets(str,256,inp)!=NULL){ //get an entire line
                                        tok = strtok(str," ");
                                        if(tok!=NULL){
                                          if(strcmp(tok,"VIEWSCALE")==0){
                                            for(i=0;i<rawdata.viewNumMultiplotSp[rawdata.numViews];i++){
                                              if((rawdata.viewMultiPlots[rawdata.numViews][i]>=0)&&(rawdata.viewMultiPlots[rawdata.numViews][i]<NSPECT)){
                                                tok = strtok(NULL," ");
                                                if(tok!=NULL){
                                                  rawdata.viewScaleFactor[rawdata.numViews][rawdata.viewMultiPlots[rawdata.numViews][i]] = atof(tok);
                                                }
                                              }
                                            }
                                            rawdata.numViews += 1;
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
                    calpar.calpar0 = (float)atof(tok);
                    tok = strtok(NULL," ");
                    if(tok!=NULL){
                      calpar.calpar1 = (float)atof(tok);
                      tok = strtok(NULL," ");
                      if(tok!=NULL){
                        calpar.calpar2 = (float)atof(tok);
                        tok = strtok(NULL," ");
                        if(tok!=NULL){
                          calpar.calMode = (unsigned char)atoi(tok);
                        }
                      }
                    }
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

  for(i=0;i<numColumns;i++){
    for(j=numElementsRead;j<S32K;j++){
      outHist[outHistStartSp+i][j]=0.;
    }
  }

  fclose(inp);
  return numColumns;
}

//function reads an .C ROOT macro file into a double array and returns the number of spectra read in
int readROOT(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{

  FILE *inp;

  if ((inp = fopen(filename, "r")) == NULL) //open the file
  {
    printf("ERROR: Cannot open the input file: %s\n", filename);
    printf("Check that the file exists.\n");
    exit(-1); //over-import error
  }

  char *tok;
  char histType = 0; //0=no hist, 1=TH1F,TH1D,TH1I
  unsigned int histNum=0;
  int ind;
  double val;
  char *str = malloc(1024);
  char *histName = malloc(256);
  strncpy(histName,"",256);

  //read the input file
  if(fgets(str,1024,inp)!=NULL){
    while((!feof(inp))&&(fgets(str,1024,inp))) //check for end of file and read next line
      {
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
int readSpectrumDataFile(const char *filename, double outHist[NSPECT][S32K], const unsigned int outHistStartSp)
{
  int numSpec = 0;

  const char *dot = strrchr(filename, '.'); //get the file extension
  if (strcmp(dot + 1, "mca") == 0)
    numSpec = readMCA(filename, outHist, outHistStartSp);
  else if (strcmp(dot + 1, "fmca") == 0)
    numSpec = readFMCA(filename, outHist, outHistStartSp);
  else if (strcmp(dot + 1, "spe") == 0)
    numSpec = readSPE(filename, outHist, outHistStartSp);
  else if (strcmp(dot + 1, "txt") == 0)
    numSpec = readTXT(filename, outHist, outHistStartSp);
  else if (strcmp(dot + 1, "C") == 0)
    numSpec = readROOT(filename, outHist, outHistStartSp);
  else if (strcmp(dot + 1, "jf3") == 0)
    numSpec = readJF3(filename, outHist, outHistStartSp);
  else{
    //printf("Improper format of input file: %s\n", filename);
    //printf("Supported file formats are: integer array (.mca), float array (.fmca), or radware (.spe) files.\n");
    return 0;
  }

  if((numSpec==-1)||(((int)outHistStartSp+numSpec)>=NSPECT)){
    return -1; //too many spectra opened
  }

  if(numSpec >= 0){
    printf("Opened file: %s, number of spectra read in: %u\n", filename, numSpec);

    //discard empty spectra
    if(rawdata.dropEmptySpectra){
      unsigned int i;
      int checkedSpCount = 0;
      unsigned int passedSpCount = 0;
      int passed = 0;
      while(checkedSpCount < numSpec){
        passed = 0;
        for(i=0;i<S32K;i++){
          if(outHist[outHistStartSp+passedSpCount][i] != 0.){
            //printf("Passed spectrum %i\n",checkedSpCount);
            passed = 1;
            passedSpCount++;
            break;
          }
        }
        if(passed == 0){
          //spectrum was empty, overwrite it
          for(i=passedSpCount;i<(numSpec-1);i++){
            memcpy(outHist[outHistStartSp+i],outHist[outHistStartSp+i+1],sizeof(outHist[i+1]));
          }
        }
        checkedSpCount++;
      }
      int dropCount = numSpec-(int)passedSpCount;
      if(dropCount>0)
        printf("Dropped %i empty spectra.\n",dropCount);
      return (int)passedSpCount;
    }
  }

  return numSpec;
}
