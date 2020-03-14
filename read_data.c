//function reads an .mca file into a double array and returns the number of spectra read in
int readMCA(const char *filename, double outHist[NSPECT][S32K], int outHistStartSp)
{
	int i, j;
	int tmpHist[S32K];
	FILE *inp;

	if ((inp = fopen(filename, "r")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1);
	}

	//get the number of spectra in the .mca file
	int numSpec = S32K;
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
		}
	}

	fclose(inp);
	return numSpec;
}

//function reads an .fmca file into a double array and returns the number of spectra read in
int readFMCA(const char *filename, double outHist[NSPECT][S32K], int outHistStartSp)
{
	int i, j;
	float tmpHist[S32K];
	FILE *inp;

	if ((inp = fopen(filename, "r")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1); //over-import error
	}

	//get the number of spectra in the .fmca file
	int numSpec = S32K;
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
		}
	}

	fclose(inp);
	return numSpec;
}

//function reads an .spe file into a double array and returns the array
int readSPE(const char *filename, double outHist[NSPECT][S32K], int outHistStartSp)
{
	int i;
	char header[36];
	float inpHist[4096];
	FILE *inp;

	memset(outHist, 0, sizeof(*outHist));

	if ((inp = fopen(filename, "r")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1);
	}

	if (fread(header, 36, 1, inp) != 1)
	{
		printf("ERROR: Cannot read header from the .spe file: %s\n", filename);
		printf("Verify that the format of the file is correct.\n");
		exit(-1);
	}
	int numElementsRead = fread(inpHist, sizeof(float), 4096, inp);
	if (numElementsRead < 1)
	{
		printf("ERROR: Cannot read spectrum from the .spe file: %s\n", filename);
		printf("fread code: %i\n",numElementsRead);
		printf("Verify that the format of the file is correct.\n");
		exit(-1);
	}

	if(outHistStartSp>=NSPECT){
		printf("Cannot open file %s, number of spectra would exceed maximum!\n", filename);
		return -1; //over-import error
	}

	//convert input data to double
	for (i = 0; i < numElementsRead; i++)
		outHist[outHistStartSp][i] = (double)inpHist[i];
	for (i = numElementsRead; i < S32K; i++)
		outHist[outHistStartSp][i] = 0.;

	fclose(inp);
	return 1;
}

int readTXT(const char *filename, double outHist[NSPECT][S32K], int outHistStartSp)
{
	int i,j;
	int numElementsRead = 0;
	char str[256];
	FILE *inp;
	double num[NSPECT];
	int numColumns; // the detected number of columns in the first line

	memset(outHist, 0, sizeof(*outHist));

	if ((inp = fopen(filename, "r")) == NULL){ //open the file
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1);
	}else{
		while(!(feof(inp)))//go until the end of file is reached
			{
				if(numElementsRead<S32K){
					if(fgets(str,256,inp)!=NULL){ //get an entire line
						int numLineEntries = sscanf(str,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&num[0],&num[1],&num[2],&num[3],&num[4],&num[5],&num[6],&num[7],&num[8],&num[9],&num[10],&num[11]);
						if(numElementsRead==0){
							numColumns = numLineEntries;
						}else{
							if((numLineEntries != numColumns)&&(numLineEntries!=0)){
								printf("ERROR: inconsistent number of columns (%i) in line %i of file: %s\n",numLineEntries,numElementsRead,filename);
								return 0;
							}
						}
						if(numLineEntries > 0){
							for(i=0;i<numLineEntries;i++){
								outHist[outHistStartSp+i][numElementsRead]=num[i];
							}
							numElementsRead++;
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

//reads a file containing spectrum data into an array
//returns the number of spectra read (0 if reading fails)
int readSpectrumDataFile(const char *filename, double outHist[NSPECT][S32K], int outHistStartSp)
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
	else
	{
		//printf("Improper format of input file: %s\n", filename);
		//printf("Supported file formats are: integer array (.mca), float array (.fmca), or radware (.spe) files.\n");
		return 0;
	}

	printf("Opened file: %s, number of spectra read in: %i\n", filename, numSpec);

	//discard empty spectra
	if(rawdata.dropEmptySpectra){
		int i;
		int checkedSpCount = 0;
		int passedSpCount = 0;
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
		if((numSpec-passedSpCount)>0)
			printf("Dropped %i empty spectra.\n",numSpec-passedSpCount);
		return passedSpCount;
	}

	return numSpec;
}
