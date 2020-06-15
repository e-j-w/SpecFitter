/* J. Williams, 2020 */

//forward declarations
float getSpBinVal(const int dispSpNum, const int bin);
float getSpBinFitWeight(const int dispSpNum, const int bin);
double getCalVal(double val);
double evalPeakArea(int peakNum);
double evalPeakAreaErr(int peakNum);
double getFitChisq();

//update the gui state while/after fitting
gboolean update_gui_fit_state(){
  switch(guiglobals.fittingSp){
    case 5:
      gtk_widget_set_sensitive(GTK_WIDGET(open_button),TRUE);
      if(rawdata.openedSp)
        gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(contract_scale),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),TRUE);
      gtk_widget_hide(GTK_WIDGET(fit_spinner));
      gtk_revealer_set_reveal_child(revealer_info_panel, FALSE);
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw to show the fit
      break;
    case 4:
      gtk_label_set_text(revealer_info_label,"Refining fit...");
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      break;
    case 3:
      gtk_label_set_text(revealer_info_label,"Fitting...");
      gtk_widget_show(GTK_WIDGET(fit_spinner));
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(contract_scale),FALSE);
      gtk_revealer_set_reveal_child(revealer_info_panel, TRUE);
      break;
    case 2:
      gtk_label_set_text(revealer_info_label,"Right-click at approximate peak positions.");
      break;
    case 1:
      gtk_widget_set_sensitive(GTK_WIDGET(open_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(append_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      gtk_label_set_text(revealer_info_label,"Right-click to set fit region lower and upper bounds.");
      gtk_revealer_set_reveal_child(revealer_info_panel, TRUE);
      break;
    case 0:
    default:
      gtk_widget_set_sensitive(GTK_WIDGET(open_button),TRUE);
      if(rawdata.openedSp)
        gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),TRUE);
      gtk_widget_hide(GTK_WIDGET(fit_spinner));
      gtk_revealer_set_reveal_child(revealer_info_panel, FALSE);
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw to hide any fit
      break;
  }
  return FALSE; //stop running
}

gboolean print_fit_results(){

  int i;
  const int strSize = 1024;
  char *fitResStr = malloc(strSize);
  char fitParStr[3][50];
  GtkDialogFlags flags; 
  GtkWidget *message_dialog;

  int length = 0;
  if(calpar.calMode == 1){
    getFormattedValAndUncertainty(getCalVal(fitpar.fitParVal[0]),getCalVal(fitpar.fitParErr[0]),fitParStr[0],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(getCalVal(fitpar.fitParVal[1]),getCalVal(fitpar.fitParErr[1]),fitParStr[1],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(getCalVal(fitpar.fitParVal[2]),getCalVal(fitpar.fitParErr[2]),fitParStr[2],50,1,guiglobals.roundErrors);
  }else{
    getFormattedValAndUncertainty(fitpar.fitParVal[0],fitpar.fitParErr[0],fitParStr[0],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(fitpar.fitParVal[1],fitpar.fitParErr[1],fitParStr[1],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(fitpar.fitParVal[2],fitpar.fitParErr[2],fitParStr[2],50,1,guiglobals.roundErrors);
  }
  length += snprintf(fitResStr+length,strSize-length,"Chisq/NDF: %f\n\nBackground\nA: %s, B: %s, C: %s\n\nPeaks",getFitChisq()/(1.0*fitpar.ndf),fitParStr[0],fitParStr[1],fitParStr[2]);
  for(i=0;i<fitpar.numFitPeaks;i++){
    getFormattedValAndUncertainty(evalPeakArea(i),evalPeakAreaErr(i),fitParStr[0],50,1,guiglobals.roundErrors);
    if(calpar.calMode == 1){
      getFormattedValAndUncertainty(getCalVal(fitpar.fitParVal[7+(3*i)]),getCalVal(fitpar.fitParErr[7+(3*i)]),fitParStr[1],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(2.35482*getCalVal(fitpar.fitParVal[8+(3*i)]),2.35482*getCalVal(fitpar.fitParErr[8+(3*i)]),fitParStr[2],50,1,guiglobals.roundErrors);
    }else{
      getFormattedValAndUncertainty(fitpar.fitParVal[7+(3*i)],fitpar.fitParErr[7+(3*i)],fitParStr[1],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(2.35482*fitpar.fitParVal[8+(3*i)],2.35482*fitpar.fitParErr[8+(3*i)],fitParStr[2],50,1,guiglobals.roundErrors);
    }
    int len = snprintf(fitResStr+length,strSize-length,"\nPeak %i Area: %s, Centroid: %s, FWHM: %s",i+1,fitParStr[0],fitParStr[1],fitParStr[2]);
    if((len < 0)||(len >= strSize-length)){
      break;
    }
    length += len;
  }

  switch (guiglobals.popupFitResults)
  {
    case 1:
      //show a dialog box with the fit results
      flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Fit results");
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),fitResStr);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
      //break;
    default:
      //print to the console
      length += snprintf(fitResStr+length,strSize-length,"\n");
      printf(fitResStr);
      break;
  }

  free(fitResStr);
  return FALSE; //stop running
}

double getFWHM(double chan, double widthF, double widthG, double widthH){
  return sqrt(widthF*widthF + widthG*widthG*(chan/1000.) + widthH*widthH*(chan/1000.)*(chan/1000.));
}

//get the value of the fitted gaussian term for a given x value
long double evalGaussTerm(int peakNum, double xval){
  long double evalG;
  if(fitpar.fixRelativeWidths){
    evalG = exp(-0.5* pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/(pow(fitpar.fitParVal[8]*fitpar.relWidths[peakNum],2.0)));
  }else{
    evalG = exp(-0.5* pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/(pow(fitpar.fitParVal[8+(3*peakNum)],2.0)));
  }
  //printf("peakNum: %i, xval: %f, pos: %f, width: %f, eval: %f\n",peakNum,xval,fitpar.fitParVal[7+(3*peakNum)],fitpar.fitParVal[8+(3*peakNum)],evalG);
  return evalG;
}

//evaluate the derivative of a gaussian peak term, needed for non-linear fits 
//derPar: 0=amplitude, 1=centroid, 2=width
long double evalGaussTermDerivative(int peakNum, double xval, int derPar){
  long double evalGDer = evalGaussTerm(peakNum,xval);
  switch (derPar){
  case 2:
    evalGDer *= fitpar.fitParVal[6+(3*peakNum)];
    if(fitpar.fixRelativeWidths){
      evalGDer *= pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/(pow(fitpar.fitParVal[8],3.0)*pow(fitpar.relWidths[peakNum],2.0));
    }else{
      evalGDer *= pow((xval-fitpar.fitParVal[7+(3*peakNum)]),2.0)/pow(fitpar.fitParVal[8+(3*peakNum)],3.0);
    }
    break;
  case 1:
    evalGDer *= fitpar.fitParVal[6+(3*peakNum)];
    if(fitpar.fixRelativeWidths){
      evalGDer *= (xval-fitpar.fitParVal[7+(3*peakNum)])/pow(fitpar.fitParVal[8]*fitpar.relWidths[peakNum],2.0);
    }else{
      evalGDer *= (xval-fitpar.fitParVal[7+(3*peakNum)])/pow(fitpar.fitParVal[8+(3*peakNum)],2.0);
    }
    break;
  case 0:
    break;
  default:
    printf("WARNING: invalid derivative parameter (%i).\n",derPar);
    break;
  }
  return evalGDer;
}

double evalFitBG(double xval){
  return fitpar.fitParVal[0] + xval*fitpar.fitParVal[1] + xval*xval*fitpar.fitParVal[2];
}

double evalFit(double xval){
  int i;
  double val = evalFitBG(xval);
  for(i=0;i<fitpar.numFitPeaks;i++)
    val += fitpar.fitParVal[6+(3*i)]*evalGaussTerm(i,xval);
  return val;
}

double evalFitOnePeak(double xval, int peak){
  if(peak>=fitpar.numFitPeaks)
    return 0.0;
  double val = evalFitBG(xval);
  val += fitpar.fitParVal[6+(3*peak)]*evalGaussTerm(peak,xval);
  return val;
}

double evalPeakArea(int peakNum){
  return fitpar.fitParVal[6+(3*peakNum)]*fitpar.fitParVal[8+(3*peakNum)]*sqrt(2.0*G_PI)/(1.0*drawing.contractFactor);
}

double evalPeakAreaErr(int peakNum){
  double err = (fitpar.fitParErr[6+(3*peakNum)]/fitpar.fitParVal[6+(3*peakNum)])*(fitpar.fitParErr[6+(3*peakNum)]/fitpar.fitParVal[6+(3*peakNum)]);
  err += (fitpar.fitParErr[8+(3*peakNum)]/fitpar.fitParVal[8+(3*peakNum)])*(fitpar.fitParErr[8+(3*peakNum)]/fitpar.fitParVal[8+(3*peakNum)]);
  err = sqrt(err);
  err = err*evalPeakArea(peakNum);
  return err;
}

//function returns chisq evaluated for the current fit
double getFitChisq(){
  int i,j;
  double chisq = 0.;
  double f;
  double yval,xval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i;
    yval = getSpBinVal(0,i);
    //background term
    f = fitpar.fitParVal[0] + fitpar.fitParVal[1]*xval + fitpar.fitParVal[2]*xval*xval;
    //gaussian(s)
    for(j=0;j<fitpar.numFitPeaks;j++){
      f += fitpar.fitParVal[6+(3*j)]*evalGaussTerm(j,xval);
    }

    //model cannot give less than 0 counts
    /*if(f<0.)
      return BIG_NUMBER;

    //likelihood ratio chisq
    if((f>0.)&&(yval > 0.)){
      chisq += (f - yval + yval*log(yval/f));
    }else{
      chisq += f;
    }*/
    //pearson chisq
    if(f!=0.)
      chisq += (f-yval)*(f-yval)/fabs(f);
    //printf("yval = %f, f = %f, chisq = %f\n",yval,f,chisq);
    //getc(stdin);
  }
  return chisq;
}

int getParameterErrors(lin_eq_type *linEq){

  if(fitpar.fixRelativeWidths){
    if(linEq->dim != 4 + (2*fitpar.numFitPeaks)){
      printf("WARNING: trying to get parameter errors without all parameters in use!\n");
    }

    int i;

    //Calculate uncertainties from linear equation solution
    for(i=0;i<3;i++)
      fitpar.fitParErr[i]=sqrt((double)(linEq->inv_matrix[i][i]*linEq->mat_weights[i][i]));
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParErr[6+(3*i)] = sqrt((double)(linEq->inv_matrix[4+(2*i)][4+(2*i)]*linEq->mat_weights[4+(2*i)][4+(2*i)]));
      fitpar.fitParErr[7+(3*i)] = sqrt((double)(linEq->inv_matrix[5+(2*i)][5+(2*i)]*linEq->mat_weights[5+(2*i)][5+(2*i)]));
      fitpar.fitParErr[8+(3*i)] = fitpar.relWidths[i]*sqrt((double)(linEq->inv_matrix[3][3]*linEq->mat_weights[3][3]));
    }

    //add Guassian parameter errors in quadrature against Cramer–Rao lower bounds
    //ie. I'm assuming the errors on the fit parameters and the errors from
    //Poisson statistics are independent
    for(i=0;i<fitpar.numFitPeaks;i++){
      
      //Cramer–Rao lower bound variances
      //(see https://en.wikipedia.org/wiki/Gaussian_function#Gaussian_profile_estimation for an explanation)
      double aCRLB = fabs(3.0*fitpar.fitParVal[6+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[8+(3*i)]));
      double pCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));
      double wCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));

      fitpar.fitParErr[6+(3*i)] = sqrt(fitpar.fitParErr[6+(3*i)]*fitpar.fitParErr[6+(3*i)] + aCRLB);
      fitpar.fitParErr[7+(3*i)] = sqrt(fitpar.fitParErr[7+(3*i)]*fitpar.fitParErr[7+(3*i)] + pCRLB);
      fitpar.fitParErr[8+(3*i)] = sqrt(fitpar.fitParErr[8+(3*i)]*fitpar.fitParErr[8+(3*i)] + wCRLB);
    }
  }else{
    if(linEq->dim != 3 + (3*fitpar.numFitPeaks)){
      printf("WARNING: trying to get parameter errors without all parameters in use!\n");
    }

    int i;

    //Calculate uncertainties from linear equation solution
    for(i=0;i<3;i++)
      fitpar.fitParErr[i]=sqrt((double)(linEq->inv_matrix[i][i]*linEq->mat_weights[i][i]));
    for(i=0;i<(3*fitpar.numFitPeaks);i++)
      fitpar.fitParErr[6+i]=sqrt((double)(linEq->inv_matrix[3+i][3+i]*linEq->mat_weights[3+i][3+i]));

    //add Guassian parameter errors in quadrature against Cramer–Rao lower bounds
    //ie. I'm assuming the errors on the fit parameters and the errors from
    //Poisson statistics are independent
    for(i=0;i<fitpar.numFitPeaks;i++){
      
      //Cramer–Rao lower bound variances
      //(see https://en.wikipedia.org/wiki/Gaussian_function#Gaussian_profile_estimation for an explanation)
      double aCRLB = fabs(3.0*fitpar.fitParVal[6+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[8+(3*i)]));
      double pCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));
      double wCRLB = fabs(fitpar.fitParVal[8+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[6+(3*i)]));

      fitpar.fitParErr[6+(3*i)] = sqrt(fitpar.fitParErr[6+(3*i)]*fitpar.fitParErr[6+(3*i)] + aCRLB);
      fitpar.fitParErr[7+(3*i)] = sqrt(fitpar.fitParErr[7+(3*i)]*fitpar.fitParErr[7+(3*i)] + pCRLB);
      fitpar.fitParErr[8+(3*i)] = sqrt(fitpar.fitParErr[8+(3*i)]*fitpar.fitParErr[8+(3*i)] + wCRLB);
    }
  }

  return 1;
}


//setup sums for the non-linearized fit
//using a CURFIT-like method
//see eq. 2.4.14, 2.4.15, pg. 47 J. Wolberg 
//'Data Analysis Using the Method of Least Squares'
//returns 1 if successful
int setupFitSums(lin_eq_type *linEq, double flambda){

  int i,j,k;
  long double cmatrix[MAX_DIM][MAX_DIM];
  memset(linEq->matrix,0,sizeof(linEq->matrix));
  memset(linEq->vector,0,sizeof(linEq->vector));
  memset(linEq->solution,0,sizeof(linEq->solution));
  memset(linEq->inv_matrix,0,sizeof(linEq->inv_matrix));
  memset(cmatrix,0,sizeof(cmatrix));
  long double xval,weight,ydiff;
  int peakNum, peakNum2, parNum, parNum2;

  if(fitpar.fixRelativeWidths){
    linEq->dim = 4 + (2*fitpar.numFitPeaks);
    long double widthDerSum;

    for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){

      xval = (long double)(i);
      ydiff = getSpBinVal(0,i) - evalFit(xval);

      if(fitpar.weightMode == 0){
        weight = getSpBinFitWeight(0,i);
      }else if(fitpar.weightMode == 1){
        weight = evalFit(xval);
      }else{
        weight = 1.;
      }

      if(weight != 0){

        widthDerSum = 0.;
        
        if(weight < 1.)
          weight = 1.;
      
        //parameters 0-2: background
        linEq->matrix[0][0] += 1./weight;
        linEq->matrix[0][1] += xval/weight;
        linEq->matrix[0][2] += xval*xval/weight;
        linEq->matrix[1][2] += xval*xval*xval/weight;
        linEq->matrix[2][2] += xval*xval*xval*xval/weight;
        linEq->vector[0] += ydiff/weight;
        linEq->vector[1] += ydiff*xval/weight;
        linEq->vector[2] += ydiff*xval*xval/weight;

        //parameter 3: width
        for(j=0;j<fitpar.numFitPeaks;j++){
          widthDerSum += evalGaussTermDerivative(j,xval,2);
        }
        linEq->matrix[0][3] += widthDerSum/weight;
        linEq->matrix[1][3] += xval*widthDerSum/weight;
        linEq->matrix[2][3] += xval*xval*widthDerSum/weight;
        linEq->matrix[3][3] += widthDerSum*widthDerSum/weight;
        linEq->vector[3] += ydiff*widthDerSum/weight;
      
        //parameters 4 and above: amplitudes, positions
        for(j=4;j<linEq->dim;j++){
          peakNum = (int)((j-4)/2);
          parNum = j % 2;
          linEq->matrix[0][j] += evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->matrix[1][j] += xval*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->matrix[2][j] += xval*xval*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->matrix[3][j] += widthDerSum*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->vector[j] += ydiff*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          //printf("j: %i, j-4/2: %i, jmod2: %i\n",j,(int)(j-4)/2,j % 2);
          for(k=4;k<linEq->dim;k++){
            peakNum2 = (int)((k-4)/2);
            parNum2 = k % 2;
            linEq->matrix[j][k] += evalGaussTermDerivative(peakNum,xval,parNum)*evalGaussTermDerivative(peakNum2,xval,parNum2)/weight;
          }
        }
      }
      
    }
    linEq->matrix[1][0] = linEq->matrix[0][1];
    linEq->matrix[2][0] = linEq->matrix[0][2];
    linEq->matrix[3][0] = linEq->matrix[0][3];
    linEq->matrix[1][1] = linEq->matrix[0][2];
    linEq->matrix[2][1] = linEq->matrix[1][2];
    linEq->matrix[3][1] = linEq->matrix[1][3];
    linEq->matrix[3][2] = linEq->matrix[2][3];
    for(i=4;i<linEq->dim;i++){
      linEq->matrix[i][0] = linEq->matrix[0][i];
      linEq->matrix[i][1] = linEq->matrix[1][i];
      linEq->matrix[i][2] = linEq->matrix[2][i];
      linEq->matrix[i][3] = linEq->matrix[3][i];
    }

  }else{
    linEq->dim = 3 + (3*fitpar.numFitPeaks);

    for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){

      xval = (double)(i);
      ydiff = getSpBinVal(0,i) - evalFit(xval);
      
      if(fitpar.weightMode == 0){
        weight = getSpBinFitWeight(0,i);
      }else if(fitpar.weightMode == 1){
        weight = evalFit(xval);
      }else{
        weight = 1.;
      }

      if(weight != 0){

        //if(weight < 1.)
        //  weight = 1.;
      
        //parameters 0-2: background
        linEq->matrix[0][0] += 1./weight;
        linEq->matrix[0][1] += xval/weight;
        linEq->matrix[0][2] += xval*xval/weight;
        linEq->matrix[1][2] += xval*xval*xval/weight;
        linEq->matrix[2][2] += xval*xval*xval*xval/weight;
        linEq->vector[0] += ydiff/weight;
        linEq->vector[1] += ydiff*xval/weight;
        linEq->vector[2] += ydiff*xval*xval/weight;

        //parameters 3 and above: amplitudes, positions, widths
        for(j=3;j<linEq->dim;j++){
          peakNum = (int)((j-3)/3);
          parNum = j % 3;
          linEq->matrix[0][j] += evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->matrix[1][j] += xval*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->matrix[2][j] += xval*xval*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          linEq->vector[j] += ydiff*evalGaussTermDerivative(peakNum,xval,parNum)/weight;
          //printf("j: %i, j-3/3: %i, jmod3: %i\n",j,(int)(j-3)/3,j % 3);
          for(k=3;k<linEq->dim;k++){
            peakNum2 = (int)((k-3)/3);
            parNum2 = k % 3;
            linEq->matrix[j][k] += evalGaussTermDerivative(peakNum,xval,parNum)*evalGaussTermDerivative(peakNum2,xval,parNum2)/weight;
            //printf("weight: %f, j: %i, k: %i, evalgaussder j: %f, evalgaussder k: %f\n",weight,j,k,evalGaussTermDerivative((int)(j-3)/3,xval,j % 3),evalGaussTermDerivative((int)(k-3)/3,xval,k % 3));
          }
        }
      }
      
    }
    linEq->matrix[1][0] = linEq->matrix[0][1];
    linEq->matrix[2][0] = linEq->matrix[0][2];
    linEq->matrix[1][1] = linEq->matrix[0][2];
    linEq->matrix[2][1] = linEq->matrix[1][2];
    for(i=3;i<linEq->dim;i++){
      linEq->matrix[i][0] = linEq->matrix[0][i];
      linEq->matrix[i][1] = linEq->matrix[1][i];
      linEq->matrix[i][2] = linEq->matrix[2][i];
    }
  }

  /*printf("Orig Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }*/

  //check if matrix has zeroes
  for(i=0;i<linEq->dim;i++){
    if(linEq->matrix[i][i] == 0.){
      printf("WARNING: matrix element %i is zero, cannot solve.\n",i);
      return 0;
    }
  } 

  //modify the curvature matrix
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      if(i!=j){
        linEq->mat_weights[i][j] = 1.0/sqrt(linEq->matrix[i][i]*linEq->matrix[j][j]);
        cmatrix[i][j] = linEq->matrix[i][j]*linEq->mat_weights[i][j];
      }  
    }
    linEq->mat_weights[i][i] = 1.0/sqrt(linEq->matrix[i][i]*linEq->matrix[i][i]);
    cmatrix[i][i] = flambda + 1.0;
  }

  memcpy(linEq->matrix,cmatrix,sizeof(linEq->matrix));

  /*printf("Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }
  printf("Weight Matrix\n");
  for(i=0;i<linEq->dim;i++){
    for(j=0;j<linEq->dim;j++){
      printf("%10.4Lf ",linEq->mat_weights[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<linEq->dim;i++){
    printf("%10.4Lf ",linEq->vector[i]);
  }
  printf("\n");*/

  return 1;

}

//function which specifies constraining conditions for peak fit parameters
int areParsValid(){
  int i;
  int fitRange = fitpar.fitEndCh - fitpar.fitStartCh;
  for(i=0;i<fitpar.numFitPeaks;i+=3){
    
    if(fitpar.fitParVal[7+(3*i)] < fitpar.fitStartCh){
      return 0;
    }
    if(fitpar.fitParVal[7+(3*i)] > fitpar.fitEndCh){
      return 0;
    }
    if(fabs(fitpar.fitParVal[7+(3*i)] - fitpar.fitPeakInitGuess[i]) > (fitRange)/2.){
      return 0;
    }
    if(fabs(fitpar.fitParVal[8+(3*i)]) > (fitRange)/2.){
      return 0;
    }
    if(getSpBinVal(0,fitpar.fitPeakInitGuess[i]) > 0){
      if(fitpar.fitParVal[6+(3*i)] < 0.){
        return 0;
      }
    }else{
      if(fitpar.fitParVal[6+(3*i)] > 0.){
        return 0;
      }
    }
  }
  return 1;
}


//non-linearized fitting
//return value: number of iterations performed (if fit not converged), -1 (if fit converged)
int nonLinearizedGausFit(const unsigned int numIter, const double convergenceFrac, lin_eq_type *linEq){

  int i;
  int iterCurrent = 0;
  int conv = 0; //converged?
  int lmCount = 0; //counter if at a chisq local minimum
  int peakNum, parNum;

  double iterStartChisq, iterEndChisq;
  double flamda = .001;

  double prevFitParVal[6+(3*MAX_FIT_PK)]; //storage for previous iteration fit parameters

  while(iterCurrent < numIter){

    iterStartChisq = getFitChisq();
    memcpy(prevFitParVal,fitpar.fitParVal,sizeof(fitpar.fitParVal));

    /*printf("\nFit iteration %i - A: %f, B: %f, C: %f\n",iterCurrent, fitpar.fitParVal[0],fitpar.fitParVal[1],fitpar.fitParVal[2]);
    for(i=0;i<fitpar.numFitPeaks;i++){
      printf("A%i: %f, P%i: %f, W%i: %f\n",i+1,fitpar.fitParVal[6+(3*i)],i+1,fitpar.fitParVal[7+(3*i)],i+1,fitpar.fitParVal[8+(3*i)]);
    }
    printf("chisq: %f\n",iterStartChisq);
    printf("\n");*/

    if(!(setupFitSums(linEq,flamda))){
      //the return value being less than the requested number of iterations indicates a failure
      return iterCurrent; 
    }
    if(!(solve_lin_eq(linEq,1))){
      //the return value being less than the requested number of iterations indicates a failure
      return iterCurrent; 
    }else{
      iterCurrent++;
      conv=1;

      /*printf("Solution\n");
      for(i=0;i<linEq->dim;i++){
        printf("%10.4Lf ",linEq->solution[i]);
      }
      printf("\n");*/

      //assign parameter values
      for(i=0;i<3;i++){
        if((fitpar.fitParVal[i]!=0.)&&(fabs(linEq->solution[i]/fitpar.fitParVal[i]) > convergenceFrac)){
          //printf("frac %i: %f\n",i,fabs(linEq->solution[i]/fitpar.fitParVal[i]));
          conv=0;
        }
        fitpar.fitParVal[i] += linEq->solution[i];
        //printf("par %i: %f\n",i,fitpar.fitParVal[i]);
      }
      if(fitpar.fixRelativeWidths){
        for(i=0;i<fitpar.numFitPeaks;i++){
          if((fitpar.fitParVal[8+(3*i)]!=0.)&&(fabs(fitpar.relWidths[i]*linEq->solution[3]/fitpar.fitParVal[8+(3*i)]) > convergenceFrac)){
            conv=0;
          }
          fitpar.fitParVal[8+(3*i)] += fitpar.relWidths[i]*linEq->solution[3];
          //printf("par %i: %f\n",6+i,fitpar.fitParVal[6+i]);
        }
        for(i=0;i<(2*fitpar.numFitPeaks);i++){
          peakNum = (int)(i/2);
          parNum = i % 2;
          if((fitpar.fitParVal[6+(3*peakNum)+parNum]!=0.)&&(fabs(linEq->solution[4+i]/fitpar.fitParVal[6+(3*peakNum)+parNum]) > convergenceFrac)){
            conv=0;
          }
          fitpar.fitParVal[6+(3*peakNum)+parNum] += linEq->solution[4+i];
          //printf("par %i: %f\n",6+i,fitpar.fitParVal[6+i]);
        }
      }else{
        for(i=0;i<(3*fitpar.numFitPeaks);i++){
          if((fitpar.fitParVal[6+i]!=0.)&&(fabs(linEq->solution[3+i]/fitpar.fitParVal[6+i]) > convergenceFrac)){
            //printf("frac %i: %f\n",3+i,fabs(linEq->solution[3+i]/fitpar.fitParVal[6+i]));
            conv=0;
          }
          fitpar.fitParVal[6+i] += linEq->solution[3+i];
          //printf("par %i: %f\n",6+i,fitpar.fitParVal[6+i]);
        }
      }
      
      

      //check chisq, if it increased change value of flambda and try again
      iterEndChisq = getFitChisq();
      //printf("Start chisq: %f, end chisq: %f\n",iterStartChisq,iterEndChisq);

      if((iterEndChisq!=iterEndChisq)||((iterEndChisq > iterStartChisq)&&(iterEndChisq > 0.))){
        if(flamda < 2.0){
          flamda *= 2.0;
          //revert fit parameters
          memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
        }else{
          flamda /= 10.;
        }
      }else if(((iterStartChisq-iterEndChisq)/iterStartChisq) < convergenceFrac) {
        lmCount++;
        flamda /= 10.;
      }else{
        lmCount = 0;
        flamda /= 10.;
      }

      if(areParsValid() == 0){
        //revert fit parameters
        memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
      }

      //check convergence condition
      if(conv == 1){
        return -1;
      }
      //getc(stdin);

    }
  }

  return iterCurrent;
}


//fitting routine
void performGausFit(){
  int i;
  lin_eq_type linEq;

  //do non-linearized fit
  int numNLIterTry = 50;
  int numNLIter = nonLinearizedGausFit(numNLIterTry, 0.0001, &linEq);
  if(numNLIter == -1){
    printf("Non-linear fit converged.\n");
    fitpar.errFound = getParameterErrors(&linEq);
  }else if (numNLIter >= numNLIterTry){
    printf("Fit did not converge after %i iterations.  Continuing...\n",numNLIter);
    guiglobals.fittingSp = 4;
    g_idle_add(update_gui_fit_state,NULL);
    nonLinearizedGausFit(numNLIterTry*10, 0.0001, &linEq);
  }else{
    printf("WARNING: failed fit, iteration %i.\n",numNLIter);
    guiglobals.fittingSp = 0;
    g_idle_add(update_gui_fit_state,NULL);
    return;
  }

  //get fit parameter uncertainties
  fitpar.errFound = 0;
  if(setupFitSums(&linEq,0.)){
    if(solve_lin_eq(&linEq,1)){
      fitpar.errFound = getParameterErrors(&linEq);
    }
  }

  /*printf("Matrix\n");
  int j;
  for(i=0;i<linEq.dim;i++){
    for(j=0;j<linEq.dim;j++){
      printf("%Lf ",linEq.matrix[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<linEq.dim;i++){
    printf("%Lf ",linEq.vector[i]);
  }
  printf("\n");*/
  //getc(stdin);

  //make sure widths are positive
  for(i=0;i<fitpar.numFitPeaks;i+=3){
    if(fitpar.fitParVal[8+(3*i)] < 0.){
      fitpar.fitParVal[8+(3*i)] = fabs(fitpar.fitParVal[8+(3*i)]);
    }
  }
  
  guiglobals.fittingSp = 5;
  g_idle_add(update_gui_fit_state,NULL);
  g_idle_add(print_fit_results,NULL);

}
gpointer performGausFitThreaded(){
  performGausFit();
  g_thread_exit(NULL);
  return NULL;
}


//do some math (assuming a Gaussian peak shape) to get a better initial estimate of the peak width
double widthGuess(const double centroidCh, const double widthInit){

  double centroidYVal = getSpBinVal(0,centroidCh);
  centroidYVal -= fitpar.fitParVal[0] + fitpar.fitParVal[1]*centroidCh;
  double HWInit = 2.35482*widthInit/2.; //initial half-width
  double FWHighEdgeVal = (getSpBinVal(0,centroidCh+HWInit) + getSpBinVal(0,centroidCh+HWInit+drawing.contractFactor) + getSpBinVal(0,centroidCh+HWInit-drawing.contractFactor))/3.;
  FWHighEdgeVal -= fitpar.fitParVal[0] + fitpar.fitParVal[1]*(centroidCh+HWInit);
  double FWLowEdgeVal = (getSpBinVal(0,centroidCh-HWInit) + getSpBinVal(0,centroidCh-HWInit+drawing.contractFactor) + getSpBinVal(0,centroidCh-HWInit-drawing.contractFactor))/3.;
  FWLowEdgeVal -= fitpar.fitParVal[0] + fitpar.fitParVal[1]*(centroidCh-HWInit);
  
  if(centroidYVal != 0.){
    //get average ratio of half-values from centroid y-value
    double highRatio = fabs(centroidYVal/FWHighEdgeVal);
    double lowRatio = fabs(centroidYVal/FWLowEdgeVal);
    double avgRatio = 0.;
    if((highRatio > 1.)&&(lowRatio > 1.)){
      avgRatio = (fabs(centroidYVal/FWHighEdgeVal) + fabs(centroidYVal/FWLowEdgeVal))/2.;
    }else if(highRatio > 1.){
      avgRatio = highRatio;
    }else{
      avgRatio = lowRatio;
    }
    if(avgRatio > 1.){
      //printf("Width guess: %f\n",HWInit*2./(2.*sqrt(2.*log(avgRatio))));
      return HWInit*2./(2.*sqrt(2.*log(avgRatio)));
    }  
  }

  //assume width is at least 2 bins
  if(widthInit < drawing.contractFactor*2.)
    return drawing.contractFactor*2.;

  return widthInit; //give up

}

int startGausFit(){

  fitpar.ndf = (int)((fitpar.fitEndCh - fitpar.fitStartCh)/(1.0*drawing.contractFactor)) - (3+(3*fitpar.numFitPeaks));
  if(fitpar.ndf <= 0){
    printf("Not enough degrees of freedom to fit!\n");
    return 0;
  }

  guiglobals.fittingSp = 3;
  g_idle_add(update_gui_fit_state,NULL);

  int i;
  fitpar.errFound = 0;

  //width parameters
  fitpar.widthFGH[0] = 3.;
  fitpar.widthFGH[1] = 2.;
  fitpar.widthFGH[2] = 0.;

  //assign initial guesses for background
  fitpar.fitParVal[0] = (getSpBinVal(0,fitpar.fitStartCh) + getSpBinVal(0,fitpar.fitEndCh))/2.0;
  fitpar.fitParVal[1] = (getSpBinVal(0,fitpar.fitEndCh) - getSpBinVal(0,fitpar.fitStartCh))/(fitpar.fitEndCh - fitpar.fitStartCh);
  fitpar.fitParVal[2] = 0.0;

  //assign initial guesses for non-linear params
  for(i=0;i<fitpar.numFitPeaks;i++){
    fitpar.fitParVal[6+(3*i)] = getSpBinVal(0,fitpar.fitPeakInitGuess[i]) - fitpar.fitParVal[0] - fitpar.fitParVal[1]*fitpar.fitPeakInitGuess[i];
    fitpar.fitParVal[7+(3*i)] = fitpar.fitPeakInitGuess[i];
  }

  //fix relative widths if required
  if(fitpar.fixRelativeWidths){
    printf("Fitting with relative peak widths fixed.\n");
    double firstWidthInitGuess = getFWHM(fitpar.fitPeakInitGuess[0],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
    fitpar.fitParVal[8] = widthGuess(fitpar.fitPeakInitGuess[0],firstWidthInitGuess);
    //printf("width guess: %f\n",fitpar.fitParVal[8]);
    for(i=1;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[8+(3*i)] = fitpar.fitParVal[8]*(getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482)/firstWidthInitGuess;
    }
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.relWidths[i] = fitpar.fitParVal[8+(3*i)]/fitpar.fitParVal[8];
      //printf("Rel width %i: %f\n",i+1,fitpar.relWidths[i]);
    }
  }else{
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[8+(3*i)] = widthGuess(fitpar.fitPeakInitGuess[i],getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482);
    }
  }

  if(fitpar.weightMode == 0){
    printf("Weighting using data.\n");
  }else if(fitpar.weightMode == 1){
    printf("Weighting using fit function.\n");
  }

  //printf("Initial guesses: %f %f %f %f %f %f\n",fitpar.fitParVal[0],fitpar.fitParVal[1],fitpar.fitParVal[2],fitpar.fitParVal[6],fitpar.fitParVal[7],fitpar.fitParVal[8]);
  if (g_thread_try_new("fitthread", performGausFitThreaded, NULL, NULL) == NULL){
    printf("WARNING: Couldn't initialize thread for fit, will try on the main thread.\n");
    performGausFit(); //try non-threaded fit
  }
  
  return 1;
}
