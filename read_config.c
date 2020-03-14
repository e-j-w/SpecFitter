//read data from a config file
int readConfigFile(FILE *file)
{
	char fullLine[256],par[256],val[256];
	char *tok;

	while(!(feof(file))){ //go until the end of file is reached
    if(fgets(fullLine,256,file)!=NULL){

			fullLine[strcspn(fullLine, "\r\n")] = 0; //strips newline characters from the string read by fgets

			//parse the line
			tok = strtok(fullLine,"=");
			if(tok != NULL){
				strncpy(par,tok,sizeof(par));

				//handle values which might include the '=' sign
				if(strcmp(par,"cal_unit") == 0){
					tok = strtok(NULL,"");
					if(tok != NULL){
						strncpy(val,tok,sizeof(val));
					}
				}else{
					tok = strtok(NULL,"=");
					if(tok != NULL){
						strncpy(val,tok,sizeof(val));
					}
				}
				
			}

			//printf("par: %s, val: %s\n",par,val);

			//read in parameter values
			if(strcmp(par,"discard_empty_spectra") == 0){
				if(strcmp(val,"yes") == 0){
					rawdata.dropEmptySpectra = 1;
				}else{
					rawdata.dropEmptySpectra = 0;
				}
			}

			if(strcmp(par,"show_bin_errors") == 0){
				if(strcmp(val,"yes") == 0){
					gui.showBinErrors = 1;
				}else{
					gui.showBinErrors = 0;
				}
			}

			if(strcmp(par,"calibrate") == 0){
				if(strcmp(val,"yes") == 0){
					calpar.calMode = 1;
				}else{
					calpar.calMode = 0;
				}
			}

			if(strcmp(par,"cal_parameter0") == 0){
				calpar.calpar0 = atof(val);
			}
			if(strcmp(par,"cal_parameter1") == 0){
				calpar.calpar1 = atof(val);
			}
			if(strcmp(par,"cal_parameter2") == 0){
				calpar.calpar2 = atof(val);
			}
			if(strcmp(par,"cal_unit") == 0){
				strncpy(calpar.calUnit,val,16);
			}
			

		}
	}
	return 1;
}

//write current settings to a config file
int writeConfigFile(FILE *file)
{
	fprintf(file,"# This is a config file for jf3.\n");
	if(calpar.calMode == 1){
		fprintf(file,"calibrate=yes\n");
		fprintf(file,"cal_parameter0=%f\n",calpar.calpar0);
		fprintf(file,"cal_parameter1=%f\n",calpar.calpar1);
		fprintf(file,"cal_parameter2=%f\n",calpar.calpar2);
		fprintf(file,"cal_unit=%s\n",calpar.calUnit);
	}else{
		fprintf(file,"calibrate=no\n");
		fprintf(file,"cal_parameter0=0.0\n");
		fprintf(file,"cal_parameter1=0.0\n");
		fprintf(file,"cal_parameter2=0.0\n");
		fprintf(file,"cal_unit=none\n");
	}
	if(rawdata.dropEmptySpectra == 1){
		fprintf(file,"discard_empty_spectra=yes\n");
	}else{
		fprintf(file,"discard_empty_spectra=no\n");
	}
	if(gui.showBinErrors == 1){
		fprintf(file,"show_bin_errors=yes\n");
	}else{
		fprintf(file,"show_bin_errors=no\n");
	}

	return 1;
}
