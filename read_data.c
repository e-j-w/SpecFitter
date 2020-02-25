//function reads an .mca file into a double array and returns the number of spectra read in
int readMCA(const char *filename, double outHist[NSPECT][S32K])
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

	if ((inp = fopen(filename, "r")) != NULL){ //reopen the file
		for (i = 0; i < numSpec; i++)
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
				sprintf(histComment[i],"%s, spectrum %i",basename((char*)filename),i);
			}	
		}
	}

	fclose(inp);
	return numSpec;
}

//function reads an .fmca file into a double array and returns the number of spectra read in
int readFMCA(const char *filename, double outHist[NSPECT][S32K])
{
	int i, j;
	float tmpHist[S32K];
	FILE *inp;

	if ((inp = fopen(filename, "r")) == NULL) //open the file
	{
		printf("ERROR: Cannot open the input file: %s\n", filename);
		printf("Check that the file exists.\n");
		exit(-1);
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

	if ((inp = fopen(filename, "r")) != NULL){ //reopen the file
		for (i = 0; i < numSpec; i++)
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
				sprintf(histComment[i],"%s, spectrum %i",basename((char*)filename),i);
				printf("Comment %i: %s\n",i,histComment[i]);
			}	
		}
	}

	fclose(inp);
	return numSpec;
}

//function reads an .spe file into a double array and returns the array
int readSPE(const char *filename, double outHist[NSPECT][S32K])
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

	//convert input data to double
	for (i = 0; i < numElementsRead; i++)
		outHist[0][i] = (double)inpHist[i];
	for (i = numElementsRead; i < S32K; i++)
		outHist[0][i] = 0.;
	
	sprintf(histComment[0],"%s",basename((char*)filename));

	fclose(inp);
	return 1;
}

//reads a file containing spectrum data into an array
//returns the number of spectra read (0 if reading fails)
int readSpectrumDataFile(const char *filename, double outHist[NSPECT][S32K])
{
	int numSpec = 0;

	const char *dot = strrchr(filename, '.'); //get the file extension
	if (strcmp(dot + 1, "mca") == 0)
		numSpec = readMCA(filename, outHist);
	else if (strcmp(dot + 1, "fmca") == 0)
		numSpec = readFMCA(filename, outHist);
	else if (strcmp(dot + 1, "spe") == 0)
		numSpec = readSPE(filename, outHist);
	else
	{
		printf("Improper type of input file: %s\n", filename);
		printf("Integer array (.mca), float array (.fmca), or radware (.spe) files are supported.\n");
		//exit(-1);
	}

	printf("Opened file: %s, number of spectra read in: %i\n", filename, numSpec);

	return numSpec;
}
