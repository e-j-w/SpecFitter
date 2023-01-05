/* © J. Williams, 2020-2023 */

//This file contains routines for fitting displayed spectra.
//The main fit routine is startGausFit (at the bottom), which
//in turn calls other subroutines.

int eval(long double *pars, uint8_t *freepars, long double *derivs, int ichan, long double *fit, uint8_t npks, int mode);

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
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"An error was encountered during the fit process.\nTry again using a different fit range, peak position(s), or fit options.");
  gtk_dialog_run (GTK_DIALOG (message_dialog));
  gtk_widget_destroy (message_dialog);

  return FALSE; //stop running
}

gboolean print_fit_dubious(){

  GtkDialogFlags flags; 
  GtkWidget *message_dialog;

  //show a dialog box
  flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Fit failed to converge");
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"The fit failed to converge.  The quoted parameter values and errors may be incorrect.\nTry again using a different fit range, peak position(s), or fit options.");
  gtk_dialog_run (GTK_DIALOG (message_dialog));
  gtk_widget_destroy (message_dialog);

  return FALSE; //stop running
}

gboolean print_fit_results(){

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
  length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"Chisq/NDF: %Lf\n\nBackground\nA: %s, B: %s, C: %s\n\n",fitpar.chisq,fitParStr[0],fitParStr[1],fitParStr[2]);
  if(fitpar.skewed == 1){
    if(calpar.calMode == 1){
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_R]),getCalWidth((double)fitpar.fitParErr[FITPAR_R]),fitParStr[0],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_BETA]),getCalWidth((double)fitpar.fitParErr[FITPAR_BETA]),fitParStr[1],50,1,guiglobals.roundErrors);
    }else{
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_R],(double)fitpar.fitParErr[FITPAR_R],fitParStr[0],50,1,guiglobals.roundErrors);
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_BETA],(double)fitpar.fitParErr[FITPAR_BETA],fitParStr[1],50,1,guiglobals.roundErrors);
    }
    length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"R: %s, Beta (skewness): %s\n\n",fitParStr[0],fitParStr[1]);
  }
  if(fitpar.stepFunction == 1){
    if(calpar.calMode == 1){
      getFormattedValAndUncertainty(getCalVal((double)fitpar.fitParVal[FITPAR_STEP]),getCalWidth((double)fitpar.fitParErr[FITPAR_STEP]),fitParStr[0],50,1,guiglobals.roundErrors);
    }else{
      getFormattedValAndUncertainty((double)fitpar.fitParVal[FITPAR_STEP],(double)fitpar.fitParErr[FITPAR_STEP],fitParStr[0],50,1,guiglobals.roundErrors);
    }
    length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"Step: %s\n\n",fitParStr[0]);
  }
  length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"Peaks");
  for(int32_t i=0;i<fitpar.numFitPeaks;i++){
    getFormattedValAndUncertainty((double)fitpar.areaVal[i],(double)fitpar.areaErr[i],fitParStr[0],50,1,guiglobals.roundErrors);
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

  switch (guiglobals.popupFitResults){
    case 1:
      //show a dialog box with the fit results
      flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Fit results");
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"%s",fitResStr);
      gtk_dialog_run(GTK_DIALOG (message_dialog));
      gtk_widget_destroy(message_dialog);
      //print to the console
      length += snprintf(fitResStr+length,(uint64_t)(strSize-length),"\n");
      printf("%s",fitResStr);
      break;
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
long double evalGaussTerm(const int32_t peakNum, long double xChVal){
  long double xvalFit = xChVal/(1.0*drawing.contractFactor);
  long double evalG = expl(-0.5* powl((xvalFit-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)]),2.0)/(powl(fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],2.0)));
  //printf("peakNum: %i, xvalFit: %f, pos: %f, width: %f, eval: %f\n",peakNum,xvalFit,fitpar.fitParVal[FITPAR_POS1+(3*peakNum)],fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],evalG);
  return evalG;
}

//get the value of the fitted skewed gaussian term for a given x value
long double evalSkewedGaussTerm(const int32_t peakNum, const long double xChVal){
  long double xvalFit = xChVal/(1.0*drawing.contractFactor);
  long double evalG = expl((xvalFit-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/fitpar.fitParVal[FITPAR_BETA]) * erfcl( (xvalFit-fitpar.fitParVal[FITPAR_POS1+(3*peakNum)])/(1.41421356*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]) + fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]/(1.41421356*fitpar.fitParVal[FITPAR_BETA]) ) ;
  //printf("peakNum: %i, xvalFit: %f, pos: %f, width: %f, eval: %f\n",peakNum,xvalFit,fitpar.fitParVal[FITPAR_POS1+(3*peakNum)],fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)],evalG);
  return evalG;
}

long double evalFitBG(const long double xChVal){
  long double xvalFit = xChVal - (long double)fitpar.fitMidCh;
  return fitpar.fitParVal[FITPAR_BGCONST] + xvalFit*fitpar.fitParVal[FITPAR_BGLIN] + xvalFit*xvalFit*fitpar.fitParVal[FITPAR_BGQUAD];
}

//gets the y value for a single fitted peak, sans background
//based on eval from RadWare gf3_subs.c
long double evalFitOnePeakNoBG(const long double xChVal, const int32_t peak){
  
  if(peak>=fitpar.numFitPeaks)
    return 0.0;

  long double pkVal = 0.;
  long double h, r, r1, r2, u, u1, u2, u3, u5, u7,  w, x, x1, y, y1, z, beta;
  uint8_t notail = 0;
  
  if(fitpar.fitParFree[3] == 0 && fitpar.fitParVal[3] == 0.f){
    notail = 1;
  }else{
    notail = 0;
    y = fitpar.fitParVal[peak*3 + 7] / (fitpar.fitParVal[4] * 3.33021838);
    if(y > 4.f){
      y1 = 0.f;
      notail = 1;
    }else{
      y1 = erfcl(y);
    }
  }

  x1 = xChVal;
  x = x1 - fitpar.fitParVal[peak*3 + 6];
  long double width  = fitpar.fitParVal[peak*3 + 7]; // normalization factor of 2.35482 omitted due to differences in how RadWare and this program report FWHM
  h      = fitpar.fitParVal[peak*3 + 8];
  w = x / (width*1.41421356);
  if(fabsl(w) > 4.f){
    u1 = 0.f;
    u3 = 0.f;
    if(x < 0.f) u3 = 2.f;
  }else{
    u1 = expl(-w*w);
    u3 = erfcl(w);
  }
  if(notail){
    /* notail = true; pure gaussians only */
    u = u1 + fitpar.fitParVal[5] * u3 / 200.;
    pkVal += h * u;
  }else{
    r = fitpar.fitParVal[3] / 100.f;
    r1 = 1.f - r;
    beta = fitpar.fitParVal[4];
    z = w + y;
    if((r2 = x / beta, fabsl(r2)) > 12.){
      u5 = 0.f;
      u7 = 0.f;
    }else{
      u7 = expl(x / beta) / y1;
      if(fabsl(z) > 4.f){
        u5 = 0.f;
        if (z < 0.f) u5 = 2.f;
      }else{
        u5 = erfcl(z);
      }
    }
    u2 = u7 * u5;
    u = r1 * u1 + r * u2 + fitpar.fitParVal[5] * u3 / 200.f;
    pkVal += h * u;
  }
  
  return pkVal;
}

long double evalFit(const long double xChVal){
  int32_t i;
  long double val = evalFitBG(xChVal);
  for(i=0;i<fitpar.numFitPeaks;i++){
    val += evalFitOnePeakNoBG(xChVal,i);
  }
  return val;
}

long double evalFitOnePeak(const long double xChVal, const int32_t peak){
  
  if(peak>=fitpar.numFitPeaks)
    return 0.0;

  return evalFitBG(xChVal) + evalFitOnePeakNoBG(xChVal,peak);
}

double evalSymGaussArea(const int32_t peakNum){
  //use Guassian integral
  long double area = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*(1.0 - (fitpar.fitParVal[FITPAR_R]/100.0))*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*sqrt(2.0*G_PI)/(1.0*drawing.contractFactor);
  return (double)area;
}

double evalSkewedGaussArea(const int32_t peakNum){
  //use definite integral of skewed Gaussian wrt x, taken
  //from -inf to inf (which collapses erf and erfc terms)
  long double area = fitpar.fitParVal[FITPAR_AMP1+(3*peakNum)]*(fitpar.fitParVal[FITPAR_R]/100.0)*fitpar.fitParVal[FITPAR_BETA]*expl(-2.0*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]*fitpar.fitParVal[FITPAR_WIDTH1+(3*peakNum)]/(4.0*fitpar.fitParVal[FITPAR_BETA]*fitpar.fitParVal[FITPAR_BETA]));
  return (double)area;
}

//Evaluate proprties of fitted peaks
//based on gffin from RadWare gf3_subs.c
void evalPeakAreas(){
  long double a, d, r, y, r1, eb, eh, er, ew, bet;
  int   i, ic;
  long double pkwidth;

  /* calc. areas, centroids and errors */
  r = fitpar.fitParVal[3] / 50.f;
  r1 = 1.f - r * .5f;
  bet = fitpar.fitParVal[4];
  if(bet == 0.){
    bet = 0.001; //handle symmetric peak case
  }
  for(i = 0; i < fitpar.numFitPeaks; ++i){
    ic = i*3 + FITPAR_WIDTH1;
    pkwidth = fitpar.fitParVal[ic] * 2.35482;
    y = pkwidth / (bet * 3.33021838f);
    if (y > 4.0f) {
      d = 0.0f;
    } else {
      d = expl(-y * y) / erfcl(y);
    }
    a = r * bet * d + pkwidth * 1.06446705f * r1;
    fitpar.areaVal[i] = a * fitpar.fitParVal[ic + 1];
    eh = a * fitpar.fitParErr[ic + 1];
    er = (bet * 2.f * d - pkwidth * 1.06446705f) * fitpar.fitParErr[3] / 100.f;
    eb = r * d * (y * 2.f * y + 1.f - d * 1.12837917f * y) * fitpar.fitParErr[4];
    ew = (r1 * 1.06446705f + r * .600561216f * d *
	  (d / 1.77245385f - y)) * fitpar.fitParErr[ic];
    fitpar.areaErr[i] = sqrtl(eh * eh + fitpar.fitParVal[ic+1] * fitpar.fitParVal[ic+1] * (er * er + eb * eb + ew * ew));
  }
} 

//evaluate the fit for the current parameter values
//based on eval from RadWare gf3_subs.c
int eval(long double *pars, uint8_t *freepars, long double *derivs, int ichan, long double *fit, uint8_t npks, int mode){

  long double a, h, r, r1, r2, u, u1, u2, u3, u5, u6, u7, u8, w, x, x1, z; 
  static long double y[15], y1[15], y2[15], beta, width;
  static int i, notail;

  if(mode == -9){
    /* mode = -9 ; initialise, i.e calculate NOTAIL,Y,Y1,Y2 */
    notail = 1;
    if(freepars[3] == 0 && pars[3] == 0.f) return 0;
    notail = 0;
    for(i = 0; i < npks; ++i){
      y[i] = pars[i*3 + 7] / (pars[4] * 3.33021838);
      if(y[i] > 4.f){
        y1[i] = 0.f;
        notail = 1;
      }else{
        y1[i] = erfcl(y[i]);
        y2[i] = (expl(-y[i] * y[i]) * 1.12837917 / y1[i]);
      }
    }
    return 0;
  }
  x = (double)(ichan - fitpar.fitMidCh);
  *fit = pars[0] + pars[1]*x + pars[2]*x*x;
  if(mode >= 1){
    derivs[1] = x;
    derivs[2] = x*x;
    derivs[3] = 0.f;
    derivs[4] = 0.f;
    derivs[5] = 0.f;
  }
  x1 = (double)(ichan);
  for(i = 0; i < npks; ++i){
    x = x1 - pars[i*3 + 6];
    width  = pars[i*3 + 7]; // normalization factor of 2.35482 omitted due to differences in how RadWare and this program report FWHM
    h      = pars[i*3 + 8];
    w = x / (width*1.41421356);
    if(fabsl(w) > 4.f){
      u1 = 0.f;
      u3 = 0.f;
      if(x < 0.f) u3 = 2.f;
    }else{
      u1 = expl(-w*w);
      u3 = erfcl(w);
    }
    if(mode == -1){
      /* mode = -1; calculate background only */
      *fit += h * pars[5] * u3 / 200.;
      continue;
    }
    if(notail){
      /* notail = true; pure gaussians only */
      u = u1 + pars[5] * u3 / 200.;
      *fit += h * u;
      /* calculate derivs only for mode.ge.1 */
      if(mode >= 1){
        derivs[5] += h * u3 / 200.;
        a = u1 * (w + pars[5] / 354.49077) * 2.;
        derivs[i*3 + 6] = h * a / (width * 1.41421356);
        derivs[i*3 + 7] = h * w * a * 1.41421356 / (width * 2.35482);
        derivs[i*3 + 8] = u;
      }
      continue;
    }
    r = pars[3] / 100.f;
    r1 = 1.f - r;
    beta = pars[4];
    z = w + y[i];
    if((r2 = x / beta, fabsl(r2)) > 12.){
      u5 = 0.f;
      u6 = 0.f;
      u7 = 0.f;
    }else{
      u7 = expl(x / beta) / y1[i];
      if(fabsl(z) > 4.f){
        u5 = 0.f;
        if (z < 0.f) u5 = 2.f;
        u6 = 0.f;
      }else{
        u5 = erfcl(z);
        u6 = expl(-z * z) * 1.12837917;
      }
    }
    u2 = u7 * u5;
    u = r1 * u1 + r * u2 + pars[5] * u3 / 200.f;
    *fit += h * u;
    /* calculate derivs only for mode.ge.1 */
    if(mode >= 1){
      u8 = u5 * y2[i];
      derivs[3] += h * (u2 - u1) / 100.;
      derivs[4] += r * h * u7 * (y[i] * (u6 - u8) - u5 * x / beta) / beta;
      derivs[5] += h * u3 / 200.f;
      a = u1 * (r1 * w + pars[5] / 354.49077) * 2.f;
      derivs[i*3 + 6] = h * (a + r*u7*(u6 - u5*2.f*y[i])) / (width*1.41421356);
      derivs[i*3 + 7] = h * (w*a + r*u7*(u6 * (w - y[i]) + u8*y[i])) * 1.41421356f / (width*2.35482);
      derivs[i*3 + 8] = u;
    }
  }
  return 0;
}

//fitter
//based on fitter from RadWare gf3_subs.c
//which itself is a modified version of the 'CURFIT' algorithm
//removed features: fixing relative peak positions
void performGausFit(){

  lin_eq_type linEq;
  linEq.dim = 0;

  int maxits = 300; //fitter iterations
  long double alpha[MAX_DIM][MAX_DIM], ddat;
  long double diff, beta[MAX_DIM], b[MAX_DIM], delta[MAX_DIM], fixed[MAX_DIM], ers[MAX_DIM];
  long double chisq1, flamda, dat, fit, r1;
  int i, j, k, l, m, conv = 0, nits, test, nextp[MAX_DIM];
  int miw=0, nip1=0;
  long double derivs[MAX_DIM];
  uint8_t npars, nfp;

  /* linEq.dim   = no. of independent (non-fixed) pars
     nfp = no. of fixed pars
     npars = total no. of pars = 3 * no.of peaks + 6
     fitpar.ndf   = no. of degrees of freedom */
  npars = (uint8_t)(6 + (3*fitpar.numFitPeaks));
  nfp = (uint8_t)(npars - fitpar.numFreePar);
  linEq.dim = (uint8_t)(npars - nfp);
  for(i = 6; i < npars; ++i){
    fixed[i] = (double)fitpar.fitParFree[i];
  }
  /* set up fixed relative widths */
  uint8_t rwfixed = 0;
  if(fitpar.fixRelativeWidths){
    uint8_t niw = 0;
    //loop over width parameters for all peaks
    for(j = 7; j < npars; j += 3){
      if(fitpar.fitParFree[j] == 1) miw = j;
      /* miw = highest fitted (non-fixed) width par. no. */
      niw = (uint8_t)(niw + fitpar.fitParFree[j]);
    }
    /* niw = no. of fitted (non-fixed) widths */
    if(niw > 1){
      for(j = 7; j <= miw; j += 3){
	      fitpar.fitParFree[j] = 0;
      }
      rwfixed = 1;
      linEq.dim = (uint8_t)(linEq.dim - niw + 1);
      nip1 = linEq.dim;
    }
  }
  fitpar.ndf = (int)((fitpar.fitEndCh - fitpar.fitStartCh)/(1.0*drawing.contractFactor)) - linEq.dim;
  if(fitpar.ndf < 1){
    printf("Not enough degrees of freedom to fit!\n");
    goto QUIT;
  }
  if(linEq.dim < 2){
    printf("Too many fixed parameters.\n");
    goto QUIT;
  }
  /* set up array nextp, pointing to free pars */
  k = 0;
  for(j = 0; j < npars; ++j){
    if(fitpar.fitParFree[j] != 0) nextp[k++] = j;
  }
  if(rwfixed) nextp[k++] = miw;
  if(k != linEq.dim){
    //probably the number of free parameters supplied is incorrect
    printf("FIT ERROR: Number of fit parameters is incorrect (%i %u).\n",k,linEq.dim);
    for(l=0;l<npars;l++){
      printf("%u\n",fitpar.fitParFree[l]);
    }
    goto QUIT;
  }
  /* initialise for fitting */
  flamda = .001f;
  nits = 0;
  test = 0;
  derivs[0] = 1.f;
  for(i = 0; i < npars; ++i){
    fitpar.fitParErr[i] = 0.f;
    b[i] = fitpar.fitParVal[i];
  }

  /* evaluate fit, alpha & beta matrices, & chisq */
 NEXT_ITERATION:
  for(j = 0; j < linEq.dim; ++j){
    beta[j] = 0.f;
    for(k = 0; k <= j; ++k){
      alpha[k][j] = 0.f;
    }
  }
  chisq1 = 0.f;
  eval(fitpar.fitParVal, fitpar.fitParFree, derivs, 0, &fit, fitpar.numFitPeaks, -9);

  for(i = fitpar.fitStartCh; i <= fitpar.fitEndCh; i += drawing.contractFactor){
    eval(fitpar.fitParVal, fitpar.fitParFree, derivs, i, &fit, fitpar.numFitPeaks, 1);
    diff = getSpBinVal(0,i) - fit;
    /* weight with fit/data/none */
    if(fitpar.weightMode == FITWEIGHT_FIT){
      dat = fit;
    }else if(fitpar.weightMode == FITWEIGHT_DATA){
      dat = getSpBinFitWeight(0,i);
    }else if(fitpar.weightMode == FITWEIGHT_NONE){
      dat = 1.;
    }
    if (dat < 1.f) dat = 1.f;
    ddat = (double) dat;
    chisq1 += diff * diff / dat;
    if(rwfixed){
      for (k = 7; k < miw; k += 3) {
        derivs[miw] += fixed[k] * derivs[k];
      }
    }
    for(l = 0; l < linEq.dim; ++l){
      j = nextp[l];
      beta[l] += diff * derivs[j] / dat;
      for(m = 0; m <= l; ++m){
        alpha[m][l] += (double)derivs[j] * (double)derivs[nextp[m]] / ddat;
      }
    }
  }
  chisq1 /= (float)(fitpar.ndf);
  /* invert modified curvature matrix to find new parameters */
 INVERT_MATRIX:
  for(j = 0; j < linEq.dim; ++j){
    if (alpha[j][j] * alpha[j][j] == 0.) {
      printf("Cannot - diagonal element no. %d equal to zero.\n", j);
      goto QUIT;
    }
  }
  linEq.matrix[0][0] = flamda + 1.f;
  for(j = 1; j < linEq.dim; ++j){
    for(k = 0; k < j; ++k){
      linEq.matrix[k][j] = alpha[k][j] / sqrtl(alpha[j][j] * alpha[k][k]);
      linEq.matrix[j][k] = linEq.matrix[k][j];
    }
    linEq.matrix[j][j] = flamda + 1.f;
  }
  linEq.dim = linEq.dim;
  //invert matrix
  if(get_inv(&linEq)==0){
    printf("Couldn't invert matrix.\n");
    goto QUIT;
  }
  if(!test){
    for(j = 0; j < linEq.dim; ++j){
      delta[j] = 0.f;
      for(k = 0; k < linEq.dim; ++k){
        delta[j] += beta[k] * linEq.inv_matrix[k][j] / sqrtl(alpha[j][j] * alpha[k][k]);
      }
    }
    /* calculate new par. values */
    for(l = 0; l < linEq.dim; ++l){
      j = nextp[l];
      b[j] = fitpar.fitParVal[j] + delta[l];
    }
    if(rwfixed && (nip1>0)){
      for(j = 7; j <= miw - 3; j += 3){
        b[j] = fitpar.fitParVal[j] + fixed[j] * delta[nip1-1];
      }
    }
    for(j = 7; j <= npars; j += 3){
      if (b[j] < 0.0f) b[j] = fabsl(b[j]);
    }
    /* if chisq increased, increase flamda and try again */
    fitpar.chisq = 0.;
    eval(b, fitpar.fitParFree, derivs, fitpar.fitParFree[3], &fit, fitpar.numFitPeaks, -9);
    for(i = fitpar.fitStartCh; i <= fitpar.fitEndCh; i += drawing.contractFactor){
      eval(b, fitpar.fitParFree, derivs, i, &fit, fitpar.numFitPeaks, 0);
      diff = getSpBinVal(0,i) - fit;
      /* weight with fit/data/none */
      if(fitpar.weightMode == FITWEIGHT_FIT){
        dat = fit;
      }else if(fitpar.weightMode == FITWEIGHT_DATA){
        dat = getSpBinFitWeight(0,i);
      }else if(fitpar.weightMode == FITWEIGHT_NONE){
        dat = 1.;
      }
      if(dat < 1.f) dat = 1.f;
      fitpar.chisq += diff * diff / dat;
    }
    fitpar.chisq /= (long double)(fitpar.ndf);
    if(fitpar.chisq > chisq1 && flamda < 2.f){
      flamda *= 10.f;
      goto INVERT_MATRIX;
    }
  }
  /* evaluate parameters and errors
     test for convergence */
  conv = 1;
  for(j = 0; j < linEq.dim; ++j){
    if(linEq.inv_matrix[j][j] < 0.) linEq.inv_matrix[j][j] = 0.;
    ers[j] = sqrtl(linEq.inv_matrix[j][j] / alpha[j][j]) * sqrtl(flamda + 1.f);
    if((r1 = delta[j], fabsl(r1)) >= ers[j] / 100.f) conv = 0;
  }
  if(!test){
    for(j = 0; j < npars; ++j){
      fitpar.fitParVal[j] = b[j];
    }
    flamda /= 10.f;
    ++nits;
    if (!conv && nits < maxits) goto NEXT_ITERATION;
    /* re-do matrix inversion with FLAMDA=0
       to calculate errors */
    flamda = 0.f;
    test = 1;
    goto INVERT_MATRIX;
  }
  
  /* list data and exit */
  for(l = 0; l < linEq.dim; ++l){
    fitpar.fitParErr[nextp[l]] = ers[l];
  }
  if(rwfixed && (nip1>0)){
    for (j = 7; j <= miw; j += 3) {
      fitpar.fitParFree[j] = (uint8_t)(fixed[j]);
      fitpar.fitParErr[j] = fixed[j] * ers[nip1 - 1];
    }
  }

  evalPeakAreas(); //get areas/errors

  printf("Fitted chs %d to %d, %d peaks\n%d indept. pars, %d degrees of freedom, weight mode %u\n", fitpar.fitStartCh, fitpar.fitEndCh, fitpar.numFitPeaks, linEq.dim, fitpar.ndf, fitpar.weightMode);
  if(rwfixed) printf("Relative widths fixed.\n");

  if(conv){
    printf(" %d iterations,  Chisq/d.o.f.= %.3Lf\n", nits, fitpar.chisq);
    guiglobals.fittingSp = FITSTATE_FITCOMPLETE;
    g_idle_add(update_gui_fit_state,NULL);
    g_idle_add(print_fit_results,NULL);
    return;
  }
  printf("Failed to converge after %d iterations,  Chisq/d.o.f.= %.3Lf\nWARNING: do not believe quoted parameter values!\n", nits, fitpar.chisq);
  if(rwfixed){
    for(i = 6; i < npars; ++i){
      fitpar.fitParFree[i] = (uint8_t)(fixed[i]);
    }
  }
  guiglobals.fittingSp = FITSTATE_FITCOMPLETE;
  g_idle_add(update_gui_fit_state,NULL);
  g_idle_add(print_fit_dubious,NULL);
  return;

 QUIT:
  if(rwfixed){
    for(i = 6; i < npars; ++i){
      fitpar.fitParFree[i] = (uint8_t)(fixed[i]);
    }
  }
  guiglobals.fittingSp = FITSTATE_NOTFITTING;
  g_idle_add(update_gui_fit_state,NULL);
  g_idle_add(print_fit_error,NULL);

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
  int32_t ctr;
  float lowWindowVal, highWindowVal, filterVal;
  float minFilterVal = (float)BIG_NUMBER;
  float maxFilterVal = -1.0f*(float)BIG_NUMBER;
  float minChVal = -1; //negative value indicates bound not found yet
  float maxChVal = -1; //negative value indicates bound not found yet

  //scan forward, find minimum
  ctr = 0;
  for(int32_t i=0;i<(halfSearchLength-windowSize);i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(int32_t j=0;j<windowSize;j++){
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
  for(int32_t i=0;i>(windowSize-halfSearchLength);i--){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(int32_t j=0;j<windowSize;j++){
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
  float lowWindowVal, highWindowVal, filterVal;
  float minFilterVal = (float)BIG_NUMBER;
  float maxFilterVal = -1.0f*(float)BIG_NUMBER;
  float minCentroidVal = centroidInit;
  float maxCentroidVal = centroidInit;
  float centroidVal = centroidInit;

  //first get maximum positive and negative slope
  for(int32_t i=0;i<((2*halfSearchLength)-windowSize);i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(int32_t j=0;j<windowSize;j++){
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
  for(int32_t i=(int32_t)minCentroidVal; i<=(int32_t)maxCentroidVal; i++){
    lowWindowVal = 0.;
    highWindowVal = 0.;
    for(int32_t j=0;j<windowSize;j++){
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
  
  for(int32_t i=0;i<fitpar.numFitPeaks;i++){
    if(i!=centInd){
      if(fabs(fitpar.fitPeakInitGuess[centInd] - fitpar.fitPeakInitGuess[i])<dist){
        return 1;
      }
    }
  }
  return 0;
}

int startGausFit(){

  guiglobals.fittingSp = FITSTATE_FITTING;
  g_idle_add(update_gui_fit_state,NULL);

  memset(fitpar.fitParErr,0,sizeof(fitpar.fitParErr));

  //width parameters
  fitpar.widthFGH[0] = 3.;
  fitpar.widthFGH[1] = 2.;
  fitpar.widthFGH[2] = 0.;

  //free background fit parameters and assign initial guesses for background
  switch(fitpar.bgType){
    case 0:
      //constant
      fitpar.fitParVal[FITPAR_BGCONST] = (getSpBinVal(0,fitpar.fitStartCh) + getSpBinVal(0,fitpar.fitEndCh))/2.0;
      fitpar.fitParVal[FITPAR_BGLIN] = 0.0;
      fitpar.fitParVal[FITPAR_BGQUAD] = 0.0;
      fitpar.fitParFree[0] = 1;
      fitpar.numFreePar = (uint8_t)(fitpar.numFreePar+1);
      break;
    case 1:
      //linear
      fitpar.fitParVal[FITPAR_BGCONST] = (getSpBinVal(0,fitpar.fitStartCh) + getSpBinVal(0,fitpar.fitEndCh))/2.0;
      fitpar.fitParVal[FITPAR_BGLIN] = (getSpBinVal(0,fitpar.fitEndCh) - getSpBinVal(0,fitpar.fitStartCh))/(float)(fitpar.fitEndCh - fitpar.fitStartCh);
      fitpar.fitParVal[FITPAR_BGQUAD] = 0.0;
      fitpar.fitParFree[0] = 1;
      fitpar.fitParFree[1] = 1;
      fitpar.numFreePar = (uint8_t)(fitpar.numFreePar+2);
      break;
    case 2:
    default:
      //quadratic
      fitpar.fitParVal[FITPAR_BGCONST] = (getSpBinVal(0,fitpar.fitStartCh) + getSpBinVal(0,fitpar.fitEndCh))/2.0;
      fitpar.fitParVal[FITPAR_BGLIN] = (getSpBinVal(0,fitpar.fitEndCh) - getSpBinVal(0,fitpar.fitStartCh))/(float)(fitpar.fitEndCh - fitpar.fitStartCh);
      fitpar.fitParVal[FITPAR_BGQUAD] = 0.0;
      fitpar.fitParFree[0] = 1;
      fitpar.fitParFree[1] = 1;
      fitpar.fitParFree[2] = 1;
      fitpar.numFreePar = (uint8_t)(fitpar.numFreePar+3);
      break;
  }

  //assign initial guesses for non-linear params
  for(int32_t i=0;i<fitpar.numFitPeaks;i++){
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
    for(int32_t i=1;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] = fitpar.fitParVal[FITPAR_WIDTH1]*(getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482)/firstWidthInitGuess;
    }
  }else{
    for(int32_t i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[FITPAR_WIDTH1+(3*i)] = widthGuess(fitpar.fitPeakInitGuess[i],getFWHM(fitpar.fitPeakInitGuess[i],fitpar.widthFGH[0],fitpar.widthFGH[1],fitpar.widthFGH[2])/2.35482);
    }
  }

  //set up skewed Gaussian if needed
  if(fitpar.skewed){
    //free parameters
    fitpar.fitParVal[FITPAR_R] = 10; //R
    fitpar.fitParFree[FITPAR_R] = 1; //R
    fitpar.fitParVal[FITPAR_BETA] = fitpar.fitParVal[FITPAR_WIDTH1]; //beta
    fitpar.fitParFree[FITPAR_BETA] = 1; //beta
    fitpar.numFreePar = (uint8_t)(fitpar.numFreePar+2);
  }

  //set up step function if needed
  if(fitpar.stepFunction){
    fitpar.fitParVal[FITPAR_STEP] = 0.1; //step function
    fitpar.fitParFree[FITPAR_STEP] = 1; //step function
    fitpar.numFreePar = (uint8_t)(fitpar.numFreePar+1);
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
  
  //printf("Initial guesses: %f %f %f %f %f %f %f %f\n",fitpar.fitParVal[FITPAR_BGCONST],fitpar.fitParVal[FITPAR_BGLIN],fitpar.fitParVal[FITPAR_BGQUAD],fitpar.fitParVal[FITPAR_R],fitpar.fitParVal[FITPAR_BETA],fitpar.fitParVal[FITPAR_AMP1],fitpar.fitParVal[FITPAR_POS1],fitpar.fitParVal[FITPAR_WIDTH1]);

  if (g_thread_try_new("fit_thread", performGausFitThreaded, NULL, NULL) == NULL){
    printf("WARNING: Couldn't initialize thread for fit, will try on the main thread.\n");
    performGausFit(); //try non-threaded fit
  }
  
  return 1;
}
