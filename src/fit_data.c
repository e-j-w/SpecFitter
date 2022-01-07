/* © J. Williams, 2020-2022 */

//This file contains routines for fitting displayed spectra.
//The main fit routine is startGausFit (at the bottom), which
//in turn calls other subroutines.

//external declarations
extern double evalPeakArea(const int32_t peakNum, const int32_t fitType);
extern double evalPeakAreaErr(const int32_t peakNum, const int32_t fitType);
extern double getFitChisq(const int32_t fitType);

//update the gui state while/after fitting
gboolean update_gui_fit_state(){
  switch(guiglobals.fittingSp){
    case FITSTATE_FITCOMPLETE:
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
    case FITSTATE_REFININGFIT2:
      gtk_label_set_text(revealer_info_label,"Further refining fit...");
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      break;
    case FITSTATE_REFININGFIT:
      gtk_label_set_text(revealer_info_label,"Refining fit...");
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      break;
    case FITSTATE_FITTING:
      gtk_label_set_text(revealer_info_label,"Fitting...");
      gtk_widget_show(GTK_WIDGET(fit_spinner));
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(contract_scale),FALSE);
      gtk_revealer_set_reveal_child(revealer_info_panel, TRUE);
      break;
    case FITSTATE_SETTINGPEAKS:
      gtk_label_set_text(revealer_info_label,"Right-click at approximate peak positions.");
      break;
    case FITSTATE_SETTINGLIMITS:
      gtk_widget_set_sensitive(GTK_WIDGET(open_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(append_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
      gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
      gtk_label_set_text(revealer_info_label,"Right-click to set fit region lower and upper bounds.");
      gtk_revealer_set_reveal_child(revealer_info_panel, TRUE);
      break;
    case FITSTATE_NOTFITTING:
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

gboolean print_fit_error(){

  GtkDialogFlags flags; 
  GtkWidget *message_dialog;

  //show a dialog box
  flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Fit error");
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"An error was encountered during the fit process.\nTry again using a different fit range or peak position(s).");
  gtk_dialog_run (GTK_DIALOG (message_dialog));
  gtk_widget_destroy (message_dialog);

  return FALSE; //stop running
}

gboolean print_fit_results(){

  int32_t i;
  const int32_t strSize = 1024;
  char *fitResStr = malloc((size_t)strSize);
  char fitParStr[3][50];
  GtkDialogFlags flags; 
  GtkWidget *message_dialog;

  int32_t length = 0;
  if(calpar.calMode == 1){
    getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_BGCONST]),getCalWidth((double)fitpar.fitParErr[FITPAR_BGCONST]),fitParStr[0],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_BGLIN]),getCalWidth((double)fitpar.fitParErr[FITPAR_BGLIN]),fitParStr[1],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_BGQUAD]),getCalWidth((double)fitpar.fitParErr[FITPAR_BGQUAD]),fitParStr[2],50,1,guiglobals.roundErrors);
  }else{
    getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_BGCONST],(double)fitpar.fitParErr[FITPAR_BGCONST],fitParStr[0],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_BGLIN],(double)fitpar.fitParErr[FITPAR_BGLIN],fitParStr[1],50,1,guiglobals.roundErrors);
    getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_BGQUAD],(double)fitpar.fitParErr[FITPAR_BGQUAD],fitParStr[2],50,1,guiglobals.roundErrors);
  }
  length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"Chisq/NDF: %f\n\nBackground\nA: %s, B: %s, C: %s\n\n",getFitChisq(fitpar.fitType)/(1.0*fitpar.ndf),fitParStr[0],fitParStr[1],fitParStr[2]);
  if(fitpar.fitType == 1){
    if(calpar.calMode == 1){
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_R]),getCalWidth((double)fitpar.fitParErr[FITPAR_R]),fitParStr[0],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_BETA]),getCalWidth((double)fitpar.fitParErr[FITPAR_BETA]),fitParStr[1],50,1,guiglobals.roundErrors);
    }else{
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_R],(double)fitpar.fitParErr[FITPAR_R],fitParStr[0],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_BETA],(double)fitpar.fitParErr[FITPAR_BETA],fitParStr[1],50,1,guiglobals.roundErrors);
    }
    length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"R: %s, Beta (skewness): %s\n\n",fitParStr[0],fitParStr[1]);
  }
  length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"Peaks");
  for(i=0;i<fitpar.numFitPeaks;i++){
    getFormattedValAndUncertainty(evalPeakArea(i,fitpar.fitType),evalPeakAreaErr(i,fitpar.fitType),fitParStr[0],50,1,guiglobals.roundErrors);
    if(calpar.calMode == 1){
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_POS1+(3*i)]),getCalWidth((double)fitpar.fitParErr[FITPAR_POS1+(3*i)]),fitParStr[1],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(2.35482*getCalWidth((double)fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]),2.35482*getCalWidth((double)fitpar.fitParErr[FITPAR_WIDTH1+(3*i)]),fitParStr[2],50,1,guiglobals.roundErrors);
    }else{
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_POS1+(3*i)],(double)fitpar.fitParErr[FITPAR_POS1+(3*i)],fitParStr[1],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(2.35482*(double)fitpar.fitParVal[FITPAR_WIDTH1+(3*i)],2.35482*(double)fitpar.fitParErr[FITPAR_WIDTH1+(3*i)],fitParStr[2],50,1,guiglobals.roundErrors);
    }
    int32_t len = snprintf(fitResStr+length,(uint64_t)(strSize-length),"\nPeak %i Area: %s, Centroid: %s, FWHM: %s",i+1,fitParStr[0],fitParStr[1],fitParStr[2]);
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
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"%s",fitResStr);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
      //break;
    default:
      //print to the console
      length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"\n");
      printf("%s",fitResStr);
      break;
  }

  free(fitResStr);
  return FALSE; //stop running
}

double getFWHM(double chan, double widthF, double widthG, double widthH){
  return sqrt(widthF*widthF + widthG*widthG*(chan/1000.) + widthH*widthH*(chan/1000.)*(chan/1000.));
}

//get the value of the fitted gaussian term for a given x value
long double evalGaussTerm(int peakNum, long double xval){
  long double evalG;
  if(fitpar.fixRelativeWidths){
    evalG = expl(-0.5* powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)]),2.0)/(powl(fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum],2.0)));
  }else{
    evalG = expl(-0.5* powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)]),2.0)/(powl(fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],2.0)));
  }
  //printf("peakNum: %i, xval: %f, pos: %f, width: %f, eval: %f\n",peakNum,xval,fitpar.fitParVal[FITPAR_POS1+(3*peakNum)],fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],evalG);
  return evalG;
}

//get the value of the fitted skewed gaussian term for a given x value
long double evalSkewedGaussTerm(const int32_t peakNum, const long double xval){
  long double evalG;
  if(fitpar.fixRelativeWidths){
    evalG = expl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA]) * erfcl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]) + (fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum])/(1.41421356*fitpar.fitParVal[FITPAR_BETA]) ) ;
  }else{
    evalG = expl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA]) * erfcl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]) + fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]/(1.41421356*fitpar.fitParVal[FITPAR_BETA]) ) ;
  }
  //printf("peakNum: %i, xval: %f, pos: %f, width: %f, eval: %f\n",peakNum,xval,fitpar.fitParVal[FITPAR_POS1+(3*peakNum)],fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],evalG);
  return evalG;
}

//evaluate the derivative of a gaussian peak term, needed for non-linear fits 
//derPar: 0=amplitude, 1=centroid, 2=width, 3=R, 4=beta
long double evalGaussTermDerivative(const int32_t peakNum, const long double xval, const int32_t derPar){
  long double evalGDer = evalGaussTerm(peakNum,xval);
  switch (derPar){
    /*case 4:
      evalGDer *= 0.0; //no skewness in symmetric Gaussian
      break;*/
    case 3:
      evalGDer *= -1.0*fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)];
      break;
    case 2:
      evalGDer *= fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*(1.0 - fitpar.fitParVal[FITPAR_R]);
      if(fitpar.fixRelativeWidths){
        evalGDer *= powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)]),2.0)/(powl(fitpar.fitParVal[FITPAR_WIDTH1],3.0)*powl(fitpar.relWidths[peakNum],2.0));
      }else{
        evalGDer *= powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)]),2.0)/powl(fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],3.0);
      }
      break;
    case 1:
      evalGDer *= fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*(1.0 - fitpar.fitParVal[FITPAR_R]);
      if(fitpar.fixRelativeWidths){
        evalGDer *= (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/powl(fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum],2.0);
      }else{
        evalGDer *= (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/powl(fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],2.0);
      }
      break;
    case 0:
      evalGDer *= (1.0 - fitpar.fitParVal[FITPAR_R]);
      break;
    default:
      printf("WARNING: invalid Gaussian derivative parameter (%i).\n",derPar);
      break;
  }
  return evalGDer;
}

//evaluate the derivative of a skewed gaussian peak term, needed for non-linear fits 
//derPar: 0=amplitude, 1=centroid, 2=width, 3=R, 4=beta
long double evalSkewedGaussTermDerivative(const int32_t peakNum, const long double xval, const int32_t derPar){
  long double evalGDer = 1.0;
  long double evalGDerT2;
  switch (derPar){
    case 4:
      evalGDer = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]*(xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])*evalSkewedGaussTerm(peakNum,xval)/(fitpar.fitParVal[FITPAR_BETA]*fitpar.fitParVal[FITPAR_BETA]);
      evalGDerT2 = 2.0*fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]/(2.5066*fitpar.fitParVal[FITPAR_BETA]*fitpar.fitParVal[FITPAR_BETA]);
      if(fitpar.fixRelativeWidths){
        evalGDerT2 *= expl( ((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA]) - powl( ((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum])) + ((fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum])/(1.41421356*fitpar.fitParVal[FITPAR_BETA])),2.0) ) ;
        evalGDerT2 *= fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum];
      }else{
        evalGDerT2 *= expl( ((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA]) - powl( ((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])) + ((fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_BETA])),2.0) ) ;
        evalGDerT2 *= fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)];
      }
      evalGDer = evalGDerT2 - evalGDer;
      break;
    case 3:
      evalGDer = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*evalSkewedGaussTerm(peakNum,xval);
      break;
    case 2:
      evalGDer = -2.0*fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]/1.7725;
      if(fitpar.fixRelativeWidths){
        evalGDer *= expl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA] - powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]) + (fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum])/(1.41421356*fitpar.fitParVal[FITPAR_BETA]),2.0) ) ;
        evalGDer *= ( (1.0/(1.41421356*fitpar.fitParVal[FITPAR_BETA])) - (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]) );
      }else{
        evalGDer *= expl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA] - powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]) + (fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_BETA]),2.0) ) ;
        evalGDer *= ( (1.0/(1.41421356*fitpar.fitParVal[FITPAR_BETA])) - (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]) );
      }
      break;
    case 1:
      evalGDer = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]*evalSkewedGaussTerm(peakNum,xval)/fitpar.fitParVal[FITPAR_BETA];
      evalGDerT2 = 2.0*fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]/2.5066;
      if(fitpar.fixRelativeWidths){
        evalGDerT2 *= expl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA] - powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]) + (fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum])/(1.41421356*fitpar.fitParVal[FITPAR_BETA]),2.0) ) ;
        evalGDerT2 = evalGDerT2/(fitpar.fitParVal[FITPAR_WIDTH1]*fitpar.relWidths[peakNum]);
      }else{
        evalGDerT2 *= expl( (xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA] - powl((xval-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]) + (fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_BETA]),2.0) ) ;
        evalGDerT2 = evalGDerT2/fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)];
      }
      evalGDer = evalGDerT2 - evalGDer;
      break;
    case 0:
      evalGDer = fitpar.fitParVal[FITPAR_R]*evalSkewedGaussTerm(peakNum,xval);
      break;
    default:
      printf("WARNING: invalid skewed Gaussian derivative parameter (%i).\n",derPar);
      break;
  }
  return evalGDer;
}

long double evalAllTermDerivative(const int32_t peakNum, const long double xval, const int32_t derPar, const int32_t fitType){
  long double val = evalGaussTermDerivative(peakNum,xval,derPar);
  if(fitType == 1){
    val += evalSkewedGaussTermDerivative(peakNum,xval,derPar);
  }
  return val;
}

long double evalFitBG(const long double xval){
  return fitpar.fitParVal[FITPAR_BGCONST] + xval*fitpar.fitParVal[FITPAR_BGLIN] + xval*xval*fitpar.fitParVal[FITPAR_BGQUAD];
}

long double evalFit(const long double xval, const int32_t fitType){
  int32_t i;
  long double val = evalFitBG(xval);
  for(i=0;i<fitpar.numFitPeaks;i++){
    val += fitpar.fitParVal[FITPAR_AMP1+(3*i)]*(1.0 - fitpar.fitParVal[FITPAR_R])*evalGaussTerm(i,xval);
    if(fitType == 1){
      val += fitpar.fitParVal[FITPAR_AMP1+(3*i)]*fitpar.fitParVal[FITPAR_R]*evalSkewedGaussTerm(i,xval);
    }
  }
  return val;
}

long double evalFitOnePeak(const long double xval, const int32_t peak, const int32_t fitType){
  if(peak>=fitpar.numFitPeaks)
    return 0.0;
  long double val = evalFitBG(xval);
  val += fitpar.fitParVal[FITPAR_AMP1+(3*peak)]*(1.0 - fitpar.fitParVal[FITPAR_R])*evalGaussTerm(peak,xval);
  if(fitType == 1)
    val += fitpar.fitParVal[FITPAR_AMP1+(3*peak)]*fitpar.fitParVal[FITPAR_R]*evalSkewedGaussTerm(peak,xval);
  return val;
}

double evalSymGaussArea(const int32_t peakNum){
  //use Guassian integral
  long double area = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*(1.0 - fitpar.fitParVal[FITPAR_R])*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*sqrt(2.0*G_PI)/(1.0*drawing.contractFactor);
  return (double)area;
}

double evalSkewedGaussArea(const int32_t peakNum){
  //use definite integral of skewed Gaussian wrt x, taken
  //from -inf to inf (which collapses erf and erfc terms)
  long double area = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*fitpar.fitParVal[FITPAR_R]*fitpar.fitParVal[FITPAR_BETA]*expl(-2.0*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]/(4.0*fitpar.fitParVal[FITPAR_BETA]*fitpar.fitParVal[FITPAR_BETA]));
  return (double)area;
}

double evalPeakArea(const int32_t peakNum, const int32_t fitType){
  double area = evalSymGaussArea(peakNum);
  if(fitType == 1){
    area += evalSkewedGaussArea(peakNum);
  }
  return area;
}

double evalPeakAreaErr(int peakNum, const int32_t fitType){
  //propagate uncertainty through the expression in the function evalSymGaussArea()
  long double err = (fitpar.fitParErr[FITPAR_AMP1+(3*peakNum)]/fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)])*(fitpar.fitParErr[FITPAR_AMP1+(3*peakNum)]/fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]);
  err += (fitpar.fitParErr[FITPAR_WIDTH1+(3*peakNum)]/fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])*(fitpar.fitParErr[FITPAR_WIDTH1+(3*peakNum)]/fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]);
  err += (fitpar.fitParErr[FITPAR_R]/(1.0 - fitpar.fitParVal[FITPAR_R]))*(fitpar.fitParErr[FITPAR_R]/(1.0 - fitpar.fitParVal[FITPAR_R]));
  err = sqrtl(err);
  err = err*evalSymGaussArea(peakNum);
  if(fitType == 1){
    //propagate uncertainty through the expression in the function evalSkewedGaussArea()
    long double errsk = (fitpar.fitParErr[FITPAR_WIDTH1+(3*peakNum)]/fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)])*(fitpar.fitParErr[FITPAR_WIDTH1+(3*peakNum)]/fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]);
    errsk += (fitpar.fitParErr[FITPAR_BETA]/fitpar.fitParVal[FITPAR_BETA])*(fitpar.fitParErr[FITPAR_BETA]/fitpar.fitParVal[FITPAR_BETA]);
    errsk *= fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]/fitpar.fitParVal[FITPAR_BETA]*fitpar.fitParVal[FITPAR_BETA];
    errsk *= 0.5; //abs(constant) in the exponential term of evalSkewedGaussArea()
    errsk += (fitpar.fitParErr[FITPAR_AMP1+(3*peakNum)]/fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)])*(fitpar.fitParErr[FITPAR_AMP1+(3*peakNum)]/fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]);
    errsk += (fitpar.fitParErr[FITPAR_R]/fitpar.fitParVal[FITPAR_R])*(fitpar.fitParErr[FITPAR_R]/fitpar.fitParVal[FITPAR_R]);
    errsk += (fitpar.fitParErr[FITPAR_BETA]/fitpar.fitParVal[FITPAR_BETA])*(fitpar.fitParErr[FITPAR_BETA]/fitpar.fitParVal[FITPAR_BETA]);
    errsk = sqrtl(errsk);
    errsk = errsk*evalSkewedGaussArea(peakNum);
    //add all errors in quadrature
    err = sqrtl(err*err + errsk*errsk);
  }
  return (double)err;
}

//function returns chisq evaluated for the current fit
double getFitChisq(const int32_t fitType){
  int32_t i,j;
  long double chisq = 0.;
  long double f;
  double yval,xval;
  for (i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){
    xval = i;
    yval = getSpBinVal(0,i);
    //background term
    f = fitpar.fitParVal[FITPAR_BGCONST] + fitpar.fitParVal[FITPAR_BGLIN]*xval + fitpar.fitParVal[FITPAR_BGQUAD]*xval*xval;
    //gaussian(s)
    for(j=0;j<fitpar.numFitPeaks;j++){
      f += fitpar.fitParVal[FITPAR_AMP1+(3*j)]*(1.0 - fitpar.fitParVal[FITPAR_R])*evalGaussTerm(j,xval);
      if(fitType == 1){
        f += fitpar.fitParVal[FITPAR_AMP1+(3*j)]*fitpar.fitParVal[FITPAR_R]*evalSkewedGaussTerm(j,xval);
      }
        
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
      chisq += (f-yval)*(f-yval)/fabsl(f);
    //printf("yval = %f, f = %f, chisq = %f\n",yval,f,chisq);
    //getc(stdin);
  }
  return (double)chisq;
}

uint8_t getParameterErrors(lin_eq_type *linEq){

  uint8_t i;

  //Calculate uncertainties from linear equation solution
  if(linEq->dim != (uint8_t)(6 + (3*fitpar.numFitPeaks))){
    printf("WARNING: trying to get parameter errors without all parameters in use!\n");
  }
  for(i=0;i<(uint8_t)(linEq->dim);i++){
    if(fitpar.fixPar[i] == 0){
      //printf("i: %i, inv: %10.4Lf, weight: %10.4Lf\n",i,linEq->inv_matrix[i][i],linEq->mat_weights[i][i]);
      fitpar.fitParErr[i]=sqrtl(fabsl(linEq->inv_matrix[i][i]*linEq->mat_weights[i][i]));
    }else{
      fitpar.fitParErr[i]=0.;
    }
  }

  if(fitpar.fixRelativeWidths){ 
    for(i=1;i<fitpar.numFitPeaks;i++){
      fitpar.fitParErr[FITPAR_WIDTH1+(3*i)] = fitpar.relWidths[i]*sqrtl(fabsl(linEq->inv_matrix[8][8]*linEq->mat_weights[8][8]));
    }
  }

  //add Guassian parameter errors in quadrature against Cramer–Rao lower bounds
  //ie. I'm assuming the errors on the fit parameters and the errors from
  //Poisson statistics are independent
  for(i=0;i<fitpar.numFitPeaks;i++){
    //Cramer–Rao lower bound variances
    //(see https://en.wikipedia.org/wiki/Gaussian_function#Gaussian_profile_estimation for an explanation)
    long double aCRLB = fabsl(3.0*fitpar.fitParVal[FITPAR_AMP1+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]));
    long double pCRLB = fabsl(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]/(sqrt(2.0*G_PI)*fitpar.fitParVal[FITPAR_AMP1+(3*i)]));
    long double wCRLB = fabsl(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]/(2.0*sqrt(2.0*G_PI)*fitpar.fitParVal[FITPAR_AMP1+(3*i)]));

    fitpar.fitParErr[FITPAR_AMP1+(3*i)] = sqrtl(fitpar.fitParErr[FITPAR_AMP1+(3*i)]*fitpar.fitParErr[FITPAR_AMP1+(3*i)] + aCRLB);
    fitpar.fitParErr[FITPAR_POS1+(3*i)] = sqrtl(fitpar.fitParErr[FITPAR_POS1+(3*i)]*fitpar.fitParErr[FITPAR_POS1+(3*i)] + pCRLB);
    fitpar.fitParErr[FITPAR_WIDTH1+(3*i)] = sqrtl(fitpar.fitParErr[FITPAR_WIDTH1+(3*i)]*fitpar.fitParErr[FITPAR_WIDTH1+(3*i)] + wCRLB);
  }

  return 1;
}


void modifyLinEqFlambda(lin_eq_type *linEq, const double flambda){
  int32_t i;
  //modify the curvature matrix
  for(i=0;i<(int32_t)(linEq->dim);i++){
    linEq->matrix[i][i] = flambda + 1.0;
  }

  /*printf("\nModifying flambda to %e, matrix\n", flambda);
  int32_t j;
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=0;j<(int32_t)(linEq->dim);j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }
  printf("Weight Matrix\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=0;j<(int32_t)(linEq->dim);j++){
      printf("%10.4Lf ",linEq->mat_weights[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    printf("%10.4Lf ",linEq->vector[i]);
  }
  printf("\n\n");*/

}

//setup sums for the non-linearized fit
//using a CURFIT-like method
//see eq. 2.4.14, 2.4.15, pg. 47 J. Wolberg 
//'Data Analysis Using the Method of Least Squares'
//returns 1 if successful
uint8_t setupFitSums(lin_eq_type *linEq, const double flambda, const int32_t fitType){

  int32_t i,j,k;
  long double cmatrix[MAX_DIM][MAX_DIM];
  memset(linEq->matrix,0,sizeof(linEq->matrix));
  memset(linEq->vector,0,sizeof(linEq->vector));
  memset(linEq->solution,0,sizeof(linEq->solution));
  memset(linEq->inv_matrix,0,sizeof(linEq->inv_matrix));
  memset(linEq->mat_weights,0,sizeof(linEq->mat_weights));
  memset(cmatrix,0,sizeof(cmatrix));
  long double xval,weight,ydiff;
  long double rDerSum = 0.;
  long double betaDerSum = 0.;
  long double widthDerSum = 0.;
  int32_t peakNum, peakNum2, parNum, parNum2;

  linEq->dim = (uint8_t)(6 + (3*fitpar.numFitPeaks));

  for(i=fitpar.fitStartCh;i<=fitpar.fitEndCh;i+=drawing.contractFactor){

    xval = (long double)(i);
    ydiff = getSpBinVal(0,i) - evalFit(xval,fitType);

    switch(fitpar.weightMode){
      case FITWEIGHT_DATA:
        weight = getSpBinFitWeight(0,i);
        break;
      case FITWEIGHT_FIT:
        weight = evalFit(xval,fitType);
        break;
      case FITWEIGHT_NONE:
      default:
        weight = 1.; //no weighting
        break;
    }

    if(weight < 0.){
      weight=fabsl(weight);
    }

    if(weight != 0){

      //parameters 0-2: background (always used)
      linEq->matrix[0][0] += 1./weight;
      linEq->matrix[0][1] += xval/weight;
      linEq->matrix[0][2] += xval*xval/weight;
      linEq->matrix[1][1] += xval*xval/weight;
      linEq->matrix[1][2] += xval*xval*xval/weight;
      linEq->matrix[2][2] += xval*xval*xval*xval/weight;
      linEq->vector[0] += ydiff/weight;
      linEq->vector[1] += ydiff*xval/weight;
      linEq->vector[2] += ydiff*xval*xval/weight;

      if(fitType == 1){
        //parameters 3 and 4: R and beta
        rDerSum = 0.;
        betaDerSum = 0.;
        for(j=0;j<fitpar.numFitPeaks;j++){
          rDerSum += evalAllTermDerivative(j,xval,3,fitType);
          betaDerSum += evalSkewedGaussTermDerivative(j,xval,4);
        }
        linEq->matrix[0][3] += rDerSum/weight;
        linEq->matrix[1][3] += xval*rDerSum/weight;
        linEq->matrix[2][3] += xval*xval*rDerSum/weight;
        linEq->matrix[3][3] += rDerSum*rDerSum/weight;
        linEq->matrix[0][4] += betaDerSum/weight;
        linEq->matrix[1][4] += xval*betaDerSum/weight;
        linEq->matrix[2][4] += xval*xval*betaDerSum/weight;
        linEq->matrix[3][4] += rDerSum*betaDerSum/weight;
        linEq->matrix[4][4] += betaDerSum*betaDerSum/weight;
        linEq->vector[3] += ydiff*rDerSum/weight;
        linEq->vector[4] += ydiff*betaDerSum/weight;
      }    

      if(fitpar.fixRelativeWidths){
        //parameter 8: width
        widthDerSum = 0.;
        for(j=0;j<fitpar.numFitPeaks;j++){
          widthDerSum += evalAllTermDerivative(j,xval,2,fitType);
        }
        linEq->matrix[0][8] += widthDerSum/weight;
        linEq->matrix[1][8] += xval*widthDerSum/weight;
        linEq->matrix[2][8] += xval*xval*widthDerSum/weight;
        if(fitType == 1){
          linEq->matrix[3][8] += rDerSum*widthDerSum/weight;
          linEq->matrix[4][8] += betaDerSum*widthDerSum/weight;
        }
        linEq->matrix[8][8] += widthDerSum*widthDerSum/weight;
        linEq->vector[8] += ydiff*widthDerSum/weight;
      
        //parameters 7, 8, 10, 11, 13, 14... : amplitudes, positions
        for(j=6;j<(int32_t)(linEq->dim);j++){
          peakNum = (int)((j-6)/3);
          parNum = j % 3;
          if(parNum < 2){
            linEq->matrix[0][j] += evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            linEq->matrix[1][j] += xval*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            linEq->matrix[2][j] += xval*xval*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            if(fitType == 1){
              linEq->matrix[3][j] += rDerSum*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
              linEq->matrix[4][j] += betaDerSum*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            }
            if(j>=8){
              linEq->matrix[8][j] += widthDerSum*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            }
            linEq->vector[j] += ydiff*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            //printf("j: %i, j-4/2: %i, jmod2: %i\n",j,(int)(j-6)/2,j % 2);
            for(k=6;k<=j;k++){
              peakNum2 = (int)((k-6)/3);
              parNum2 = k % 3;
              if(parNum2 < 2){
                linEq->matrix[k][j] += evalAllTermDerivative(peakNum,xval,parNum,fitType)*evalAllTermDerivative(peakNum2,xval,parNum2,fitType)/weight;
              }
            }
          }
          
        }
      }else{ //fitpar.fixRelativeWidths = 0
        //parameters 7 and above: amplitudes, positions, widths
        for(j=6;j<(int32_t)(linEq->dim);j++){
          peakNum = (int)((j-6)/3);
          parNum = j % 3;
          linEq->matrix[0][j] += evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
          linEq->matrix[1][j] += xval*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
          linEq->matrix[2][j] += xval*xval*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
          if(fitType == 1){
            linEq->matrix[3][j] += rDerSum*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
            linEq->matrix[4][j] += betaDerSum*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
          }
          linEq->vector[j] += ydiff*evalAllTermDerivative(peakNum,xval,parNum,fitType)/weight;
          //printf("j: %i, j-3/3: %i, jmod3: %i\n",j,(int)(j-5)/3,j % 3);
          for(k=6;k<=j;k++){
            peakNum2 = (int)((k-6)/3);
            parNum2 = k % 3;
            linEq->matrix[k][j] += evalAllTermDerivative(peakNum,xval,parNum,fitType)*evalAllTermDerivative(peakNum2,xval,parNum2,fitType)/weight;
            //printf("weight: %f, j: %i, k: %i, evalgaussder j: %f, evalgaussder k: %f\n",weight,j,k,evalAllTermDerivative((int)(j-5)/3,xval,j % 3),evalAllTermDerivative((int)(k-5)/3,xval,k % 3));
          }
        }
      }

    }

  }

  //mirror the matrix
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=(i+1);j<(int32_t)(linEq->dim);j++){
      linEq->matrix[j][i] = linEq->matrix[i][j];
    }
  }

  /*printf("Orig Matrix\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=0;j<(int32_t)(linEq->dim);j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }*/

  //check if matrix has zeroes
  for(i=0;i<(int32_t)(linEq->dim);i++){
    if(fitpar.fixPar[i] == 0){
      if(linEq->matrix[i][i] == 0.){
        printf("WARNING: matrix element %i is zero, cannot solve.\n",i);
        return 0;
      }
    }
  } 

  //modify the curvature matrix
  for(i=0;i<(int32_t)(linEq->dim);i++){
    if(fitpar.fixPar[i] == 0){
      for(j=0;j<(int32_t)(linEq->dim);j++){
        if(fitpar.fixPar[j] == 0){
          if(i!=j){
            linEq->mat_weights[i][j] = 1.0/sqrtl(linEq->matrix[i][i]*linEq->matrix[j][j]);
            cmatrix[i][j] = linEq->matrix[i][j]*linEq->mat_weights[i][j];
          }
        }
      }
      linEq->mat_weights[i][i] = 1.0/sqrtl(linEq->matrix[i][i]*linEq->matrix[i][i]);
    }
  }

  memcpy(linEq->matrix,cmatrix,sizeof(linEq->matrix));

  modifyLinEqFlambda(linEq,flambda);

  /*printf("Matrix\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=0;j<(int32_t)(linEq->dim);j++){
      printf("%10.4Lf ",linEq->matrix[i][j]);
    }
    printf("\n");
  }
  printf("Weight Matrix\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    for(j=0;j<(int32_t)(linEq->dim);j++){
      printf("%10.4Lf ",linEq->mat_weights[i][j]);
    }
    printf("\n");
  }
  printf("Vector\n");
  for(i=0;i<(int32_t)(linEq->dim);i++){
    printf("%10.4Lf ",linEq->vector[i]);
  }
  printf("\n");*/
  //getc(stdin);

  return 1;

}

//function which specifies constraining conditions for peak fit parameters
uint8_t areParsValid(const int32_t fitType){
  int32_t i;
  int32_t fitRange = fitpar.fitEndCh - fitpar.fitStartCh;
  for(i=0;i<fitpar.numFitPeaks;i+=3){
    
    if(fitpar.fitParVal[FITPAR_POS1+(3*i)] < fitpar.fitStartCh){
      return 0;
    }
    if(fitpar.fitParVal[FITPAR_POS1+(3*i)] > fitpar.fitEndCh){
      return 0;
    }
    if(fabsl(fitpar.fitParVal[FITPAR_POS1+(3*i)] - fitpar.fitPeakInitGuess[i]) > (fitRange)/2.){
      return 0;
    }
    if(fabsl(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]) > (fitRange)/2.){
      return 0;
    }else if(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] <= 0.){
      return 0; //cannot have 0 or negative width
    }
    if(getSpBinVal(0,(int)fitpar.fitPeakInitGuess[i]) > 0){
      if(fitpar.fitParVal[FITPAR_AMP1+(3*i)] < 0.){
        return 0;
      }
    }else{
      if(fitpar.fitParVal[FITPAR_AMP1+(3*i)] > 0.){
        return 0;
      }
    }
  }
  if(fitType == 1){
    if(fitpar.fixPar[FITPAR_R]==0){
      if(fitpar.fitParVal[FITPAR_R]!=fitpar.fitParVal[FITPAR_R]){
        return 0;
      }
      if((fitpar.fitParVal[FITPAR_R] < -1.0)||(fitpar.fitParVal[FITPAR_R] > 1.0)){
        return 0;
      }
    }
    if(fitpar.fixPar[FITPAR_BETA]==0){
      if(fitpar.fitParVal[FITPAR_BETA]!=fitpar.fitParVal[FITPAR_BETA]){
        return 0;
      }
      if(fitpar.fitParVal[FITPAR_BETA] < 0.0){
        return 0;
      }

    }    
  }
  return 1;
}


//non-linearized fitting
//return value: number of iterations performed (if fit not converged), -1 (if fit converged)
int32_t nonLinearizedGausFit(const uint8_t numIter, const double convergenceFrac, lin_eq_type *linEq, const int32_t fitType){

  int32_t i;
  int32_t iterCurrent = 0;
  int32_t conv = 0; //converged?
  int32_t lmCount = 0; //counter if at a chisq local minimum

  double iterStartChisq, iterEndChisq;
  double flambda = .001;

  long double prevFitParVal[6+(3*MAX_FIT_PK)]; //storage for previous iteration fit parameters

  while(iterCurrent < numIter){

    iterStartChisq = getFitChisq(fitType);
    memcpy(prevFitParVal,fitpar.fitParVal,sizeof(fitpar.fitParVal));

    /*printf("\nFit iteration %i - A: %Lf, B: %Lf, C: %Lf\n",iterCurrent, fitpar.fitParVal[FITPAR_BGCONST],fitpar.fitParVal[FITPAR_BGLIN],fitpar.fitParVal[FITPAR_BGQUAD]);
    for(i=0;i<fitpar.numFitPeaks;i++){
      printf("A%i: %Lf, P%i: %Lf, W%i: %Lf\n",i+1,fitpar.fitParVal[FITPAR_AMP1+(3*i)],i+1,fitpar.fitParVal[FITPAR_POS1+(3*i)],i+1,fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]);
    }
    printf("chisq: %f\n",iterStartChisq);
    printf("\n");*/

    if(!(setupFitSums(linEq,flambda,fitType))){
      //the return value being less than the requested number of iterations indicates a failure
      return iterCurrent; 
    }

    int32_t doneIter = 0;
    while(doneIter != 1){

      if(doneIter == -1){
        if(flambda == 0.){
          flambda = .001;
        }
        //revert fit parameters
        memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
        //modify the matrix
        modifyLinEqFlambda(linEq,flambda);
      }

      if(!(solve_lin_eq(linEq,1))){
        //the return value being less than the requested number of iterations indicates a failure
        return iterCurrent; 
      }else{
        iterCurrent++;
        conv=1;

        /*printf("Inv Matrix\n");
        int32_t j;
        for(i=0;i<linEq->dim;i++){
          for(j=0;j<linEq->dim;j++){
            printf("%10.4Lf ",linEq->inv_matrix[i][j]);
          }
          printf("\n");
        }*/

        /*printf("Solution\n");
        for(i=0;i<linEq->dim;i++){
          printf("%10.4Lf ",linEq->solution[i]);
        }
        printf("\n");*/

        //assign parameter values
        for(i=0;i<(int32_t)(linEq->dim);i++){
          if(fitpar.fixPar[i] == 0){
            if((fitpar.fitParVal[i]!=0.)&&(fabsl(linEq->solution[i]/fitpar.fitParVal[i]) > convergenceFrac)){
              //printf("frac %i: %f\n",i,fabs(linEq->solution[i]/fitpar.fitParVal[i]));
              conv=0;
            }
            fitpar.fitParVal[i] += linEq->solution[i];
            //printf("par %i: %f\n",i,fitpar.fitParVal[i]);
          }
        }

        if(fitpar.fixRelativeWidths){
          for(i=0;i<fitpar.numFitPeaks;i++){
            if((fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]!=0.)&&(fabsl(fitpar.relWidths[i]*linEq->solution[8]/fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]) > convergenceFrac)){
              conv=0;
            }
            fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] += fitpar.relWidths[i]*linEq->solution[8];
            //printf("par %i: %f\n",6+i,fitpar.fitParVal[FITPAR_AMP1+i]);
          }
        }

        //check chisq, if it increased change value of flambda and try again
        iterEndChisq = getFitChisq(fitType);
        //printf("Start chisq: %f, end chisq: %f\n",iterStartChisq,iterEndChisq);

        if(areParsValid(fitType) != 0){
          if((iterEndChisq!=iterEndChisq)||((iterEndChisq > iterStartChisq)&&(iterEndChisq > 0.))){
            if(flambda < 2.0){
              flambda *= 2.0;
              doneIter = -1;
            }else{
              flambda /= 10.;
              doneIter = 1;
            }
          }else if(((iterStartChisq-iterEndChisq)/iterStartChisq) < convergenceFrac) {
            lmCount++;
            flambda /= 10.;
            doneIter = 1;
          }else{
            lmCount = 0;
            flambda /= 10.;
            doneIter = 1;
          }
        }else if(conv == 1){ //check convergence condition
          //printf("\nConverged!\n");
          return -1;
        }else{
          if(flambda < 2.0){
            flambda *= 2.0;
            doneIter = -1;
          }else{
            //revert fit parameters
            memcpy(fitpar.fitParVal,prevFitParVal,sizeof(fitpar.fitParVal));
            flambda /= 10.;
            doneIter = 1;
          }
        }
        
      }

    }

  }

  return iterCurrent;
}


//fitting routine
void performGausFit(){
  int32_t i;
  lin_eq_type linEq;

  //initially fix skew parameters (first fit symmetric shape, then vary these after)
  fitpar.fitParVal[FITPAR_R] = 0.0; //unused in this fit
  fitpar.fitParVal[FITPAR_BETA] = 0.0; //unused in this fit
  fitpar.fixPar[FITPAR_R] = 1; //fix unused parameter at zero
  fitpar.fixPar[FITPAR_BETA] = 1; //fix unused parameter at zero

  //do non-linearized fit
  uint8_t numNLIterTry = 50;
  int32_t numNLIter = nonLinearizedGausFit(numNLIterTry, 0.001, &linEq,0);
  if(numNLIter >= numNLIterTry){
    //printf("Fit did not converge after %i iterations.  Continuing...\n",numNLIter);
    guiglobals.fittingSp = FITSTATE_REFININGFIT;
    g_idle_add(update_gui_fit_state,NULL);
    numNLIterTry = 100;
    numNLIter = nonLinearizedGausFit(numNLIterTry, 0.001, &linEq,0);
  }

  if(numNLIter == -1){
    if(fitpar.fitType == 0){
      printf("Non-linear fit converged.\n");
    }
    //fitpar.errFound = getParameterErrors(&linEq);
  }else if(numNLIter < numNLIterTry){
    printf("WARNING: failed fit, iteration %i.\n",numNLIter);
    guiglobals.fittingSp = FITSTATE_NOTFITTING;
    g_idle_add(update_gui_fit_state,NULL);
    g_idle_add(print_fit_error,NULL);
    return;
  }

  //for skewed Guassian, allow R and beta to vary
  if(fitpar.fitType == 1){
    guiglobals.fittingSp = FITSTATE_REFININGFIT2;
    g_idle_add(update_gui_fit_state,NULL);
    fitpar.fitParVal[FITPAR_R] = 0.05;
    fitpar.fitParVal[FITPAR_BETA] = fitpar.fitParVal[FITPAR_WIDTH1] / 2.0;
    fitpar.fixPar[FITPAR_R] = 0; //unfix the R parameter
    fitpar.fixPar[FITPAR_BETA] = 0; //unfix the beta parameter
    numNLIterTry = 100;
    numNLIter = nonLinearizedGausFit(numNLIterTry, 0.001, &linEq, fitpar.fitType);

    if(numNLIter == -1){
      printf("Non-linear fit converged.\n");
    }else if(numNLIter < numNLIterTry){
      printf("WARNING: failed fit, iteration %i.\n",numNLIter);
      guiglobals.fittingSp = FITSTATE_NOTFITTING;
      g_idle_add(update_gui_fit_state,NULL);
      g_idle_add(print_fit_error,NULL);
      return;
    }
  }

  //get fit parameter uncertainties
  fitpar.errFound = 0;
  modifyLinEqFlambda(&linEq,0.);
  if(solve_lin_eq(&linEq,1)){
    fitpar.errFound = getParameterErrors(&linEq);
  }

  /*printf("Matrix\n");
  int32_t j;
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
  for(i=0;i<fitpar.numFitPeaks;i++){
    if(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] < 0.){
      fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] = fabsl(fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]);
    }
  }
  
  guiglobals.fittingSp = FITSTATE_FITCOMPLETE;
  g_idle_add(update_gui_fit_state,NULL);
  g_idle_add(print_fit_results,NULL);

}
gpointer performGausFitThreaded(){
  performGausFit();
  g_thread_exit(NULL);
  return NULL;
}


//do some math (assuming a Gaussian peak shape) to get a better initial estimate of the peak width
long double widthGuess(const double centroidCh, const double widthInit){

  int32_t windowSize = 5;
  int32_t halfSearchLength = 100;
  int32_t scanPastLength = 10;
  int32_t i,j,ctr;
  float lowWindowVal, highWindowVal, filterVal;
  float minFilterVal = (float)BIG_NUMBER;
  float maxFilterVal = -1.0f*(float)BIG_NUMBER;
  float minChVal = -1; //negative value indicates bound not found yet
  float maxChVal = -1; //negative value indicates bound not found yet

  //scan forward, find minimum
  ctr = 0;
  for(i=0;i<(halfSearchLength-windowSize);i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(j=0;j<windowSize;j++){
      lowWindowVal += getSpBinVal(0,(int)centroidCh+(drawing.contractFactor*(i + j)));
      highWindowVal += getSpBinVal(0,(int)centroidCh+(drawing.contractFactor*(i + j + windowSize)));
    }
    filterVal = highWindowVal - lowWindowVal;
    if(filterVal < minFilterVal){
      minChVal = (float)(centroidCh+(drawing.contractFactor*i));
      minFilterVal = filterVal;
      ctr=0;
    }else{
      ctr++;
    }
    //printf("i=%i, ch=%f, lowwindow=%f, highwindow=%f, filterVal=%f\n",i,centroidCh+(drawing.contractFactor*i),lowWindowVal,highWindowVal,filterVal);
    if(ctr>=scanPastLength){
      break;
    }
    if(i==(halfSearchLength-windowSize-1)){
      //end of the search window
      minChVal = -1;
      break;
    }
  }

  //scan backward, find maximum
  ctr = 0;
  for(i=0;i>(windowSize-halfSearchLength);i--){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(j=0;j<windowSize;j++){
      lowWindowVal += getSpBinVal(0,(int)centroidCh+(drawing.contractFactor*(i - j - windowSize)));
      highWindowVal += getSpBinVal(0,(int)centroidCh+(drawing.contractFactor*(i - j)));
    }
    filterVal = highWindowVal - lowWindowVal;
    if(filterVal > maxFilterVal){
      maxChVal = (float)(centroidCh+(drawing.contractFactor*i));
      maxFilterVal = filterVal;
      ctr=0;
    }else{
      ctr++;
    }
    //printf("i=%i, ch=%f, lowwindow=%f, highwindow=%f, filterVal=%f\n",i,centroidCh+(drawing.contractFactor*i),lowWindowVal,highWindowVal,filterVal);
    if(ctr>=scanPastLength){
      break;
    }
    if(i==(windowSize-halfSearchLength+1)){
      //end of the search window
      maxChVal = -1;
      break;
    }
  }

  //check that neither bound is invalid
  if(minChVal <= centroidCh){
    minChVal = -1;
  }
  if(maxChVal >= centroidCh){
    maxChVal = -1;
  }
  if(minChVal == maxChVal){
    minChVal = -1;
    maxChVal = -1;
  }

  if((minChVal>=0)&&(maxChVal>=0)){
    //both bounds valid
    //printf("Both bounds valid, centroid=%f, lower=%f, upper=%f, width=%f\n",centroidCh,minChVal,maxChVal,(minChVal-maxChVal)/2.35482);
    return (minChVal-maxChVal)/2.35482;
  }else if(minChVal>=0){
    //only lower bound valid
    //printf("Lower bound invalid, centroid=%f, upper=%f, width=%f\n",centroidCh,minChVal,(minChVal-centroidCh)/1.1774);
    return (minChVal-centroidCh)/1.1774;
  }else if(maxChVal>=0){
    //only upper bound valid
    //printf("Upper bound invalid, centroid=%f, lower=%f, width=%f\n",centroidCh,maxChVal,(centroidCh-maxChVal)/1.1774);
    return (centroidCh-maxChVal)/1.1774;
  }

  //only get here if neither bound is valid

  //assume width is at least 2 bins
  if(widthInit < drawing.contractFactor*2.)
    return drawing.contractFactor*2.;

  return widthInit; //give up

}

//use a trapezoidal filter to determine the best peak location in the window
float centroidGuess(const float centroidInit){
  int32_t windowSize = 5;
  int32_t halfSearchLength = 10;
  int32_t i,j;
  float lowWindowVal, highWindowVal, filterVal;
  float minFilterVal = (float)BIG_NUMBER;
  float maxFilterVal = -1.0f*(float)BIG_NUMBER;
  float minCentroidVal = centroidInit;
  float maxCentroidVal = centroidInit;
  float centroidVal = centroidInit;

  //first get maximum positive and negative slope
  for(i=0;i<((2*halfSearchLength)-windowSize);i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(j=0;j<windowSize;j++){
      lowWindowVal += getSpBinVal(0,(int)centroidInit+(drawing.contractFactor*(i - halfSearchLength + j)));
      highWindowVal += getSpBinVal(0,(int)centroidInit+(drawing.contractFactor*(i - halfSearchLength + j + windowSize)));
    }
    filterVal = highWindowVal - lowWindowVal;
    if(filterVal < minFilterVal){
      minCentroidVal = (float)i;
      minFilterVal = filterVal;
    }
    if(filterVal > maxFilterVal){
      maxCentroidVal = (float)i;
      maxFilterVal = filterVal;
    }
    //printf("centroidVal: %i, filterVal: %f\n",i,filterVal);
  }
  //printf("minCentroidVal: %f, maxCentroidVal: %f\n",minCentroidVal,maxCentroidVal);

  //then get slope closest to zero between the maximum positive and negative slopes
  minFilterVal = (float)BIG_NUMBER;
  if(minCentroidVal > maxCentroidVal){
    //swap values so that minCentroidVal is the smaller of the two
    float swapVal = minCentroidVal;
    minCentroidVal = maxCentroidVal;
    maxCentroidVal = swapVal;
  }
  for(i=(int)minCentroidVal; i<=(int)maxCentroidVal; i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(j=0;j<windowSize;j++){
      lowWindowVal += getSpBinVal(0,(int)centroidInit+(drawing.contractFactor*(i - halfSearchLength + j)));
      highWindowVal += getSpBinVal(0,(int)centroidInit+(drawing.contractFactor*(i - halfSearchLength + j + windowSize)));
    }
    filterVal = fabsf(highWindowVal - lowWindowVal);
    if(filterVal < minFilterVal){
      centroidVal = centroidInit + (float)(i + windowSize - halfSearchLength);
      minFilterVal = filterVal;
    }
  }

  //printf("Centroid guess: %f\n",centroidVal);
  return centroidVal;
}

int isCentroidNearOthers(const int32_t centInd, const float dist){
  int32_t i;
  for(i=0;i<fitpar.numFitPeaks;i++){
    if(i!=centInd){
      if(fabs(fitpar.fitPeakInitGuess[centInd] - fitpar.fitPeakInitGuess[i])<dist){
        return 1;
      }
    }
  }
  return 0;
}

int startGausFit(){
  
  fitpar.ndf = (int)((fitpar.fitEndCh - fitpar.fitStartCh)/(1.0*drawing.contractFactor)) - (3+(3*(int)fitpar.numFitPeaks));
  if(fitpar.ndf <= 0){
    printf("Not enough degrees of freedom to fit!\n");
    return 0;
  }

  guiglobals.fittingSp = FITSTATE_FITTING;
  g_idle_add(update_gui_fit_state,NULL);

  int32_t i;
  fitpar.errFound = 0;

  memset(fitpar.fixPar,0,sizeof(fitpar.fixPar));
  memset(fitpar.fitParErr,0,sizeof(fitpar.fitParErr));

  //width parameters
  fitpar.widthFGH[0] = 3.;
  fitpar.widthFGH[1] = 2.;
  fitpar.widthFGH[2] = 0.;

  //assign initial guesses for background
  fitpar.fitParVal[FITPAR_BGCONST] = (getSpBinVal(0,fitpar.fitStartCh) + getSpBinVal(0,fitpar.fitEndCh))/2.0;
  fitpar.fitParVal[FITPAR_BGLIN] = (getSpBinVal(0,fitpar.fitEndCh) - getSpBinVal(0,fitpar.fitStartCh))/(float)(fitpar.fitEndCh - fitpar.fitStartCh);
  fitpar.fitParVal[FITPAR_BGQUAD] = 0.0;

  //assign initial guesses for non-linear params
  for(i=0;i<fitpar.numFitPeaks;i++){
    if(isCentroidNearOthers(i,10.0)==0){
      fitpar.fitPeakInitGuess[i] = centroidGuess(fitpar.fitPeakInitGuess[i]); //guess peak positions
    }
    fitpar.fitParVal[FITPAR_AMP1+(3*i)] = getSpBinVal(0,(int)fitpar.fitPeakInitGuess[i]) - fitpar.fitParVal[FITPAR_BGCONST] - fitpar.fitParVal[FITPAR_BGLIN]*fitpar.fitPeakInitGuess[i];
    fitpar.fitParVal[FITPAR_POS1+(3*i)] = fitpar.fitPeakInitGuess[i];
  }

  //fix relative widths if required
  if(fitpar.fixRelativeWidths){
    printf("Fitting with relative peak widths fixed.\n");
    double firstWidthInitGuess = getFWHM(fitpar.fitPeakInitGuess[0],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482;
    fitpar.fitParVal[FITPAR_WIDTH1] = widthGuess(fitpar.fitPeakInitGuess[0],firstWidthInitGuess);
    //printf("width guess: %f\n",fitpar.fitParVal[FITPAR_WIDTH1]);
    for(i=1;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] = fitpar.fitParVal[FITPAR_WIDTH1]*(getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482)/firstWidthInitGuess;
      fitpar.fixPar[FITPAR_WIDTH1+(3*i)] = 2; //mark these parameters as fixed relative
    }
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.relWidths[i] = fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]/fitpar.fitParVal[FITPAR_WIDTH1];
      //printf("Rel width %i: %f\n",i+1,fitpar.relWidths[i]);
    }
  }else{
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] = widthGuess(fitpar.fitPeakInitGuess[i],getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482);
    }
  }

  switch(fitpar.weightMode){
    case FITWEIGHT_DATA:
      printf("Weighting using data.\n");
      break;
    case FITWEIGHT_FIT:
      printf("Weighting using fit function.\n");
      break;
    case FITWEIGHT_NONE:
    default:
      printf("No weighting for fit.\n");
      break;
  }

  fitpar.fitParVal[FITPAR_RESERVED1] = 0.0; //unused parameter
  fitpar.fixPar[FITPAR_RESERVED1] = 1; //fix unused parameter at zero
  
  //printf("Initial guesses: %f %f %f %f %f %f %f %f\n",fitpar.fitParVal[FITPAR_BGCONST],fitpar.fitParVal[FITPAR_BGLIN],fitpar.fitParVal[FITPAR_BGQUAD],fitpar.fitParVal[FITPAR_R],fitpar.fitParVal[FITPAR_BETA],fitpar.fitParVal[FITPAR_AMP1],fitpar.fitParVal[FITPAR_POS1],fitpar.fitParVal[FITPAR_WIDTH1]);

  if (g_thread_try_new("fit_thread", performGausFitThreaded, NULL, NULL) == NULL){
    printf("WARNING: Couldn't initialize thread for fit, will try on the main thread.\n");
    performGausFit(); //try non-threaded fit
  }
  
  
  return 1;
}
