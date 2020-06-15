/* J. Williams, 2020 */

//function writes a .jf3 file
//header containing: file format version number (unsigned char), number of spectra (unsigned char), label for each spactrum (each 256 element char array),
//number of comments (unsigned char), individual comments (comment sp (char), ch (int32), y-val (float32), followed by a 256 element char array for the comment itself)
//spectrum data is compressed using a basic RLE method: packet header (signed char) specifying number of elements to repeat, then the element as a 32-bit float
//alternatively, the packet header may be a negative number -n, in which case n non-repeating elements follow as 32-bit floats
//if the packet header is 0, that is the end of the spectrum  
int writeJF3(const char *filename, double inpHist[NSPECT][S32K])
{
	int i, j, k;
	FILE *out;
	unsigned char ucharBuf;

	if ((out = fopen(filename, "w")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the output file: %s\n", filename);
		printf("The file may not be accesible.\n");
		return 1;
	}

	//printf("Number of spectra to write: %i\n",rawdata.numSpOpened);

	ucharBuf = 0; //file version number
	fwrite(&ucharBuf,sizeof(unsigned char),1,out);
	ucharBuf = (unsigned char)rawdata.numSpOpened; //number of spectra to write
	fwrite(&ucharBuf,sizeof(unsigned char),1,out);
	for(i=0;i<rawdata.numSpOpened;i++){
		fwrite(&rawdata.histComment[i],sizeof(rawdata.histComment[i]),1,out);
	}
	ucharBuf = rawdata.numChComments; //number of comments to write
	fwrite(&ucharBuf,sizeof(unsigned char),1,out);
	for(i=0;i<rawdata.numChComments;i++){
		fwrite(&rawdata.chanCommentSp[i],sizeof(rawdata.chanCommentSp[i]),1,out);
		fwrite(&rawdata.chanCommentCh[i],sizeof(rawdata.chanCommentCh[i]),1,out);
		fwrite(&rawdata.chanCommentVal[i],sizeof(rawdata.chanCommentVal[i]),1,out);
		fwrite(&rawdata.chanComment[i],sizeof(rawdata.chanComment[i]),1,out);
	}

	float lastBin = 0.;
	float currentBin;
	float val;
	signed char packetCounter;
	for(i=0;i<rawdata.numSpOpened;i++){
		if(i<NSPECT){

			//printf("Writing spectrum %i\n",i);

			//scan for end of spectrum
			int lastCh = 0;
			for(j=S32K-1;j>=0;j--){
				if(inpHist[i][j]!=0){
					lastCh=j;
					break;
				}
			}

			//encode spectrum
			packetCounter = 1;
			for(j=0;j<S32K;j++){
				
				//get bin values
				currentBin = inpHist[i][j];
				if(j>0)
					lastBin = inpHist[i][j-1];
				else
					lastBin = inpHist[i][j];
				
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
						for(k=0;k<(packetCounter*-1);k++){
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
					for(k=0;k<(packetCounter*-1);k++){
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
					for(k=0;k<(packetCounter*-1);k++){
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
