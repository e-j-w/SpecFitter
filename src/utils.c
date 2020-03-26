/* J. Williams, 2020 */

//get a formatted string with a value and its uncertainty, 
//properly rounded using the '20 rule' for reporting uncertainties
void getFormattedValAndUncertainty(const double val, const double err, char *str, const int strLength, const int showErr){

  if(err < 0){
    //invalid error
    snprintf(str,strLength,"Negative err!");
    return;
  }

  //printf("val: %f, err: %f\n",val,err);

  double errc = err;
  double valc = val;

  //get the position of the first sig fig in the error
  int sigf = 0;
  if(fabs(errc) > 1.95){
    while(fabs(errc) >= 19.5){
      errc /= 10;
      sigf++;
      if(sigf > 100){
        printf("WARNING: stuck on getFormattedValAndUncertainty loop 1.\n");
        break; //safety
      }
    }
  }else if((fabs(errc) < 1.95)&&(errc != 0)){
    while(fabs(errc) <= 1.95){
      errc *= 10;
      sigf--;
      if(sigf > 100){
        printf("WARNING: stuck on getFormattedValAndUncertainty loop 2.\n");
        break; //safety
      }
    }
  }else{
      sigf = 0;
  }

  //round the value based on the sig fig
  valc /= pow(10,sigf);
  valc = round(valc);

  //get the first digits of the error
  errc = round(errc);

  //printf("sigf: %i, errc = %f\n",sigf,errc);

  if(val == 0){
    if(showErr)
      snprintf(str,strLength,"0(0)");
    else
      snprintf(str,strLength,"0");
  }else if(fabs(val) > 0.001){
    //use normal notation
    if(sigf < 0){
      if(showErr)
        snprintf(str,strLength,"%0.*f(%.0f)",-1*sigf,valc*pow(10,sigf),errc);
      else
        snprintf(str,strLength,"%0.*f",-1*sigf,valc*pow(10,sigf));
    }else{
      if(showErr)
        snprintf(str,strLength,"%0.0f(%.0f)",valc*pow(10,sigf),errc*pow(10,sigf));
      else
        snprintf(str,strLength,"%0.0f",valc*pow(10,sigf));
    }
  }else{
    //use scientific notation
    if(showErr)
      snprintf(str,strLength,"%0.0f(%.0f)E%i",valc,errc,sigf);
    else
      snprintf(str,strLength,"%0.0fE%i",valc,sigf);
  }

}

void getFormattedYAxisVal(const double val, const double axisMinVal, const double axisMaxVal, char *str, const int strLength){

  if(val == 0){
    snprintf(str,strLength,"0");
    return;
  }

  if((axisMaxVal - axisMinVal) >= 15){
    //large axis range
    if(fabs(val) < 100000){
      //print value rounded to nearest integer
      snprintf(str,strLength,"%0.0f",val);
    }else{
      //big number, use scientific notation
      double valc = val;
      int sigf = 0;
      while(fabs(valc) >= 10.0){
        valc /= 10;
        sigf++;
      }
      if(sigf < 100){
        snprintf(str,strLength,"%0.1fe%i",valc,sigf);
      }else{
        //REALLY big number...
        snprintf(str,strLength,"%0.0fe%i",valc,sigf);
      }
    }    
  }else if((axisMaxVal - axisMinVal) >= 1.5){
    //small axis range, use one decimal place
    snprintf(str,strLength,"%0.1f",val);
  }else{
    //axis range is very small, use scientific notation
    if(fabs(val) >= 1){
      double valc = val;
      int sigf = 0;
      while(fabs(valc) >= 10.0){
        valc /= 10;
        sigf++;
      }
      snprintf(str,strLength,"%0.1fe%i",valc,sigf);
    }else{
      double valc = val;
      int sigf = 0;
      while(fabs(valc) < 1.0){
        valc *= 10;
        sigf++;
      }
      snprintf(str,strLength,"%0.1fe-%i",valc,sigf);
    }
  }
}

//oomVal = order-of-magnitude value, multiplicative factor describing an order of magnitude, typically 10, can use this to calculate sig figs in different bases
int getNSigf(const double val, const double oomVal){
  int sigf = 0;
  double valc = val;

  //printf("val: %f\n",val);

  if(oomVal <= 1.){
    printf("Invalid order-of-magnitude value (%f), what are you doing?!\n",oomVal);
    return 0;
  }

  if(val == 0){
    return 0;
  }

  if(val > 1){
    while(!(valc < oomVal)){
      valc /= oomVal;
      sigf++;
    }
  }else if(val < 1){
    while(val < 1.){
      valc *= oomVal;
      sigf--;
    }
  }else{
    return 0;
  }

  return sigf;
}