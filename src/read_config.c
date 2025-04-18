/* © J. Williams, 2020-2025 */

//read data from a config file
//keepCalibration: 0=update the calibration, 1=don't change the calibration
uint8_t readConfigFile(FILE *file, uint8_t keepCalibration){

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
      if(strcmp(par,"limit_centroid") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.limitCentroid = 1;
        }else{
          rawdata.dispFitPar.limitCentroid = 0;
        }
      }
      if(strcmp(par,"fix_skew_amp") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.fixSkewAmplitide = 1;
        }else{
          rawdata.dispFitPar.fixSkewAmplitide = 0;
        }
      }
      if(strcmp(par,"fix_beta") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.fixBeta = 1;
        }else{
          rawdata.dispFitPar.fixBeta = 0;
        }
      }
      if(strcmp(par,"manual_width_val") == 0){
        float wVal = (float)atof(val);
        if((wVal >= 0.0f)&&(wVal <= 100.0f)){
          rawdata.dispFitPar.manualWidthVal = wVal;
        }
      }
      if(strcmp(par,"manual_width_offset") == 0){
        float wVal = (float)atof(val);
        if((wVal >= 0.0f)&&(wVal <= 100.0f)){
          rawdata.dispFitPar.manualWidthOffset = wVal;
        }
      }
      if(strcmp(par,"limit_centroid_val") == 0){
        float cVal = (float)atof(val);
        if((cVal >= 0.0f)&&(cVal <= 100.0f)){
          rawdata.dispFitPar.limitCentroidVal = cVal;
        }
      }
      if(strcmp(par,"fix_skew_amp_val") == 0){
        float ampVal = (float)atof(val);
        if((ampVal >= 0.0f)&&(ampVal <= 100.0f)){
          rawdata.dispFitPar.fixedRVal = ampVal;
        }
      }
      if(strcmp(par,"fix_beta_val") == 0){
        float bVal = (float)atof(val);
        rawdata.dispFitPar.fixedBetaVal = bVal;
      }
      if(strcmp(par,"peak_width_method") == 0){
        uint8_t pwVal = (uint8_t)atoi(val);
        if((pwVal != PEAKWIDTHMODE_PREVIOUS)&&(pwVal != PEAKWIDTHMODE_MANUAL)){
          rawdata.dispFitPar.peakWidthMethod = pwVal;
        }else{
          rawdata.dispFitPar.peakWidthMethod = PEAKWIDTHMODE_RELATIVE;
        }
      }
      if(strcmp(par,"step_function") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.stepFunction = 1;
        }else{
          rawdata.dispFitPar.stepFunction = 0;
        }
      }
      if(strcmp(par,"force_positive_amplitude") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.forcePositivePeaks = 1;
        }else{
          rawdata.dispFitPar.forcePositivePeaks = 0;
        }
      }
      if(strcmp(par,"inflate_errors") == 0){
        if(strcmp(val,"yes") == 0){
          rawdata.dispFitPar.inflateErrors = 1;
        }else{
          rawdata.dispFitPar.inflateErrors = 0;
        }
      }
      if(strcmp(par,"fit_weight_mode") == 0){
        uint8_t ucVal = (uint8_t)atoi(val);
        if(ucVal <= 2){
          rawdata.dispFitPar.weightMode = ucVal;
        }
      }
      if(strcmp(par,"fit_type") == 0){
        uint8_t ucVal = (uint8_t)atoi(val);
        if(ucVal < FITTYPE_ENUM_LENGTH){
          rawdata.dispFitPar.fitType = ucVal;
        }
      }
      if(strcmp(par,"bg_type") == 0){
        uint8_t ucVal = (uint8_t)atoi(val);
        if(ucVal <= 2){
          rawdata.dispFitPar.bgType = ucVal;
        }
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
          calpar.calpar[0] = (float)atof(val);
        }
        if(strcmp(par,"cal_parameter1") == 0){
          calpar.calpar[1] = (float)atof(val);
        }
        if(strcmp(par,"cal_parameter2") == 0){
          calpar.calpar[2] = (float)atof(val);
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
  if((calpar.calpar[1] == 0.0)&&(calpar.calpar[2] == 0.0)){
    calpar.calpar[1]=1.0;
  }

  return 1;
}

//write current settings to a config file
uint8_t writeConfigFile(FILE *file){

  fprintf(file,"# This is a config file for SpecFitter.\n");
  if(calpar.calMode == 1){
    fprintf(file,"calibrate=yes\n");
    fprintf(file,"cal_parameter0=%f\n",calpar.calpar[0]);
    fprintf(file,"cal_parameter1=%f\n",calpar.calpar[1]);
    fprintf(file,"cal_parameter2=%f\n",calpar.calpar[2]);
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
  fprintf(file,"manual_width_val=%f\n",rawdata.dispFitPar.manualWidthVal);
  fprintf(file,"manual_width_offset=%f\n",rawdata.dispFitPar.manualWidthOffset);
  if(rawdata.dispFitPar.limitCentroid == 1){
    fprintf(file,"limit_centroid=yes\n");
    fprintf(file,"limit_centroid_val=%f\n",rawdata.dispFitPar.limitCentroidVal);
  }else{
    fprintf(file,"limit_centroid=no\n");
  }
  if(rawdata.dispFitPar.fixSkewAmplitide == 1){
    fprintf(file,"fix_skew_amp=yes\n");
    fprintf(file,"fix_skew_amp_val=%f\n",rawdata.dispFitPar.fixedRVal);
  }else{
    fprintf(file,"fix_skew_amp=no\n");
  }
  if(rawdata.dispFitPar.fixBeta == 1){
    fprintf(file,"fix_beta=yes\n");
    fprintf(file,"fix_beta_val=%f\n",rawdata.dispFitPar.fixedBetaVal);
  }else{
    fprintf(file,"fix_beta=no\n");
  }
  if(rawdata.dispFitPar.peakWidthMethod < PEAKWIDTHMODE_ENUM_LENGTH){
    fprintf(file,"peak_width_method=%u\n",rawdata.dispFitPar.peakWidthMethod);
  }
  if(rawdata.dispFitPar.stepFunction == 1){
    fprintf(file,"step_function=yes\n");
  }else{
    fprintf(file,"step_function=no\n");
  }
  if(rawdata.dispFitPar.forcePositivePeaks == 1){
    fprintf(file,"force_positive_amplitude=yes\n");
  }else{
    fprintf(file,"force_positive_amplitude=no\n");
  }
  if(rawdata.dispFitPar.inflateErrors == 1){
    fprintf(file,"inflate_errors=yes\n");
  }else{
    fprintf(file,"inflate_errors=no\n");
  }
  if(rawdata.dispFitPar.bgType < 3){
    fprintf(file,"bg_type=%u\n",rawdata.dispFitPar.bgType);
  }
  if(rawdata.dispFitPar.fitType < FITTYPE_ENUM_LENGTH){
    fprintf(file,"fit_type=%u\n",rawdata.dispFitPar.fitType);
  }
  if(rawdata.dispFitPar.weightMode < FITWEIGHT_ENUM_LENGTH){
    fprintf(file,"fit_weight_mode=%u\n",rawdata.dispFitPar.weightMode);
  }

  return 1;
}


void updateConfigFile(){
  char dirPath[256];
  strcpy(dirPath,"");
  strcat(dirPath,getenv("HOME"));
  strcat(dirPath,"/.config/specfitter/specfitter.conf");
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
  strcat(dirPath,"/.config/specfitter/specfitter.conf");
  FILE *configFile = fopen(dirPath, "r");
  if(configFile != NULL){
    readConfigFile(configFile,0); //read the configuration values
    fclose(configFile);
  }else{
    printf("WARNING: Unable to read preferences to configuration file.\n");
  }
}