/* J. Williams, 2020 */

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
				strncpy(par,tok,sizeof(par)-1);

				//handle values which might include the '=' sign
				if(strcmp(par,"cal_unit") == 0){
					tok = strtok(NULL,"");
					if(tok != NULL){
						strncpy(val,tok,sizeof(val)-1);
					}
				}else{
					tok = strtok(NULL,"=");
					if(tok != NULL){
						strncpy(val,tok,sizeof(val)-1);
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
			if(strcmp(par,"round_errors") == 0){
				if(strcmp(val,"yes") == 0){
					gui.roundErrors = 1;
				}else{
					gui.roundErrors = 0;
				}
			}
			if(strcmp(par,"prefer_dark_theme") == 0){
				if(strcmp(val,"yes") == 0){
					gui.preferDarkTheme = 1;
				}else{
					gui.preferDarkTheme = 0;
				}
			}
			if(strcmp(par,"draw_sp_labels") == 0){
				if(strcmp(val,"yes") == 0){
					gui.drawSpLabels = 1;
				}else{
					gui.drawSpLabels = 0;
				}
			}
			if(strcmp(par,"fix_relative_widths") == 0){
				if(strcmp(val,"yes") == 0){
					fitpar.fixRelativeWidths = 1;
				}else{
					fitpar.fixRelativeWidths = 0;
				}
			}
			if(strcmp(par,"popup_fit_results") == 0){
				if(strcmp(val,"yes") == 0){
					gui.popupFitResults = 1;
				}else{
					gui.popupFitResults = 0;
				}
			}
			if(strcmp(par,"fit_weight_mode") == 0){
				int intVal = atoi(val);
				if((intVal >= 0)&&(intVal <= 2))
					fitpar.weightMode = intVal;
			}
			if(strcmp(par,"autozoom") == 0){
				if(strcmp(val,"yes") == 0){
					gui.autoZoom = 1;
				}else{
					gui.autoZoom = 0;
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
				val[15] = '\0'; //truncate string
				strcpy(calpar.calUnit,val);
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
	if(gui.roundErrors == 1){
		fprintf(file,"round_errors=yes\n");
	}else{
		fprintf(file,"round_errors=no\n");
	}
	if(gui.preferDarkTheme == 1){
		fprintf(file,"prefer_dark_theme=yes\n");
	}else{
		fprintf(file,"prefer_dark_theme=no\n");
	}
	if(gui.drawSpLabels == 1){
		fprintf(file,"draw_sp_labels=yes\n");
	}else{
		fprintf(file,"draw_sp_labels=no\n");
	}
	if(gui.autoZoom == 1){
		fprintf(file,"autozoom=yes\n");
	}else{
		fprintf(file,"autozoom=no\n");
	}
	if(fitpar.fixRelativeWidths == 1){
		fprintf(file,"fix_relative_widths=yes\n");
	}else{
		fprintf(file,"fix_relative_widths=no\n");
	}
	if(gui.popupFitResults == 1){
		fprintf(file,"popup_fit_results=yes\n");
	}else{
		fprintf(file,"popup_fit_results=no\n");
	}
	if(fitpar.weightMode == 0){
		fprintf(file,"fit_weight_mode=0\n");
	}else if(fitpar.weightMode == 1){
		fprintf(file,"fit_weight_mode=1\n");
	}else if(fitpar.weightMode == 2){
		fprintf(file,"fit_weight_mode=2\n");
	}

	return 1;
}


void updateConfigFile(){
  char dirPath[256];
  strcpy(dirPath,"");
	strcat(dirPath,getenv("HOME"));
	strcat(dirPath,"/.config/jf3/jf3.conf");
  FILE *configFile = fopen(dirPath, "w");
  if(configFile != NULL){
    writeConfigFile(configFile); //write the default configuration values
    fclose(configFile);
  }else{
    printf("WARNING: Unable to write preferences to configuration file.\n");
  }
}

void updatePrefsFromConfigFile(){
  char dirPath[256];
  strcpy(dirPath,"");
	strcat(dirPath,getenv("HOME"));
	strcat(dirPath,"/.config/jf3/jf3.conf");
  FILE *configFile = fopen(dirPath, "r");
  if(configFile != NULL){
    readConfigFile(configFile); //read the configuration values
    fclose(configFile);
  }else{
    printf("WARNING: Unable to read preferences to configuration file.\n");
  }
}