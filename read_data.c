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
				for (j = 0; j < S32K; j++)
					outHist[i][j] = (double)tmpHist[j];
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
	int numElementsRead = 0;
	char str[256];
	double inpHist[S32K];
	FILE *inp;

	memset(outHist, 0, sizeof(*outHist));

	if ((inp = fopen(filename, "r")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1);
	}

	numElementsRead=0;
	while(fscanf(inp,"%s\n",str)!=EOF){
		if(numElementsRead<S32K){
			inpHist[numElementsRead] = atof(str);
		}
		numElementsRead++;
	}

	//check for import errors
	if(numElementsRead == 0){
		printf("ERROR: Empty input file: %s\n", filename);
		return 0;
	}

	//copy imported data
	memcpy(outHist[outHistStartSp],inpHist,sizeof(inpHist));

	fclose(inp);
	return 1;
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

	return numSpec;
}
