/* J. Williams, 2020-2021 */

//read data from a config file
//keepCalibration: 0=update the calibration, 1=don't change the calibration
int readConfigFile(FILE *file, int keepCalibration)
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
          guiglobals.showBinErrors = 1;
        }else{
          guiglobals.showBinErrors = 0;
        }
      }
      if(strcmp(par,"round_errors") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.roundErrors = 1;
        }else{
          guiglobals.roundErrors = 0;
        }
      }
      if(strcmp(par,"prefer_dark_theme") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.preferDarkTheme = 1;
        }else{
          guiglobals.preferDarkTheme = 0;
        }
      }
      if(strcmp(par,"use_drawing_animation") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.useZoomAnimations = 1;
        }else{
          guiglobals.useZoomAnimations = 0;
        }
      }
      if(strcmp(par,"draw_sp_labels") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.drawSpLabels = 1;
        }else{
          guiglobals.drawSpLabels = 0;
        }
      }
      if(strcmp(par,"draw_sp_comments") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.drawSpComments = 1;
        }else{
          guiglobals.drawSpComments = 0;
        }
      }
      if(strcmp(par,"draw_grid_lines") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.drawGridLines = 1;
        }else{
          guiglobals.drawGridLines = 0;
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
          guiglobals.popupFitResults = 1;
        }else{
          guiglobals.popupFitResults = 0;
        }
      }
      if(strcmp(par,"fit_weight_mode") == 0){
        unsigned char ucVal = (unsigned char)atoi(val);
        if(ucVal <= 2)
          fitpar.weightMode = ucVal;
      }
      if(strcmp(par,"fit_type") == 0){
        unsigned char ucVal = (unsigned char)atoi(val);
        if(ucVal <= 1)
          fitpar.fitType = ucVal;
      }
      if(strcmp(par,"autozoom") == 0){
        if(strcmp(val,"yes") == 0){
          guiglobals.autoZoom = 1;
        }else{
          guiglobals.autoZoom = 0;
        }
      }

      //read in calibration parameters, if a calibration has not already been applied
      //(eg. by reading in a file with the calibration defined)
      if(keepCalibration == 0){
        if(strcmp(par,"calibrate") == 0){
          if(strcmp(val,"yes") == 0){
            calpar.calMode = 1;
          }
        }
        if(strcmp(par,"cal_parameter0") == 0){
          calpar.calpar0 = (float)atof(val);
        }
        if(strcmp(par,"cal_parameter1") == 0){
          calpar.calpar1 = (float)atof(val);
        }
        if(strcmp(par,"cal_parameter2") == 0){
          calpar.calpar2 = (float)atof(val);
        }
        if(strcmp(par,"cal_unit") == 0){
          val[15] = '\0'; //truncate string
          strcpy(calpar.calUnit,val);
        }
        if(strcmp(par,"cal_unit_y") == 0){
          val[31] = '\0'; //truncate string
          strcpy(calpar.calYUnit,val);
        }
      }

    }
  }

  //check for and correct invalid parameter values
  if((calpar.calpar1 == 0.0)&&(calpar.calpar2 == 0.0)){
    calpar.calpar1=1.0;
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
    fprintf(file,"cal_unit_y=%s\n",calpar.calYUnit);
  }else{
    fprintf(file,"calibrate=no\n");
    fprintf(file,"cal_parameter0=0.0\n");
    fprintf(file,"cal_parameter1=1.0\n");
    fprintf(file,"cal_parameter2=0.0\n");
    fprintf(file,"cal_unit=none\n");
  }
  if(rawdata.dropEmptySpectra == 1){
    fprintf(file,"discard_empty_spectra=yes\n");
  }else{
    fprintf(file,"discard_empty_spectra=no\n");
  }
  if(guiglobals.showBinErrors == 1){
    fprintf(file,"show_bin_errors=yes\n");
  }else{
    fprintf(file,"show_bin_errors=no\n");
  }
  if(guiglobals.roundErrors == 1){
    fprintf(file,"round_errors=yes\n");
  }else{
    fprintf(file,"round_errors=no\n");
  }
  if(guiglobals.preferDarkTheme == 1){
    fprintf(file,"prefer_dark_theme=yes\n");
  }else{
    fprintf(file,"prefer_dark_theme=no\n");
  }
  if(guiglobals.useZoomAnimations == 1){
    fprintf(file,"use_drawing_animation=yes\n");
  }else{
    fprintf(file,"use_drawing_animation=no\n");
  }
  if(guiglobals.drawSpLabels == 1){
    fprintf(file,"draw_sp_labels=yes\n");
  }else{
    fprintf(file,"draw_sp_labels=no\n");
  }
  if(guiglobals.drawSpComments == 1){
    fprintf(file,"draw_sp_comments=yes\n");
  }else{
    fprintf(file,"draw_sp_comments=no\n");
  }
  if(guiglobals.drawGridLines == 1){
    fprintf(file,"draw_grid_lines=yes\n");
  }else{
    fprintf(file,"draw_grid_lines=no\n");
  }
  if(guiglobals.autoZoom == 1){
    fprintf(file,"autozoom=yes\n");
  }else{
    fprintf(file,"autozoom=no\n");
  }
  if(fitpar.fixRelativeWidths == 1){
    fprintf(file,"fix_relative_widths=yes\n");
  }else{
    fprintf(file,"fix_relative_widths=no\n");
  }
  if(guiglobals.popupFitResults == 1){
    fprintf(file,"popup_fit_results=yes\n");
  }else{
    fprintf(file,"popup_fit_results=no\n");
  }
  if(fitpar.fitType == 0){
    fprintf(file,"fit_type=0\n");
  }else if(fitpar.fitType == 1){
    fprintf(file,"fit_type=1\n");
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
    readConfigFile(configFile,0); //read the configuration values
    fclose(configFile);
  }else{
    printf("WARNING: Unable to read preferences to configuration file.\n");
  }
}