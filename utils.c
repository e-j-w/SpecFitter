
//get a formatted string with a value and its uncertainty, 
//properly rounded using the '20 rule' for reporting uncertainties
void getFormattedValAndUncertainty(const double val, const double err, char *str, const int strLength){
  
  if(err < 0){
    //invalid error
    snprintf(str,strLength,"Negative err!");
    return;
  }

  double errc = err;
  double valc = val;

  //get the position of the first sig fig in the error
  int sigf = 0;
  if(errc > 0){
    while(errc >= 19.5){
      errc /= 10;
      sigf++;
    }
  }else if(errc < 0){
    while(errc <= 1.95){
      errc *= 10;
      sigf--;
    }
  }else{
      sigf = 0;
  }

  //round the value based on the sig fig
  valc /= pow(10,sigf);
  valc = round(valc);

  //get the first digits of the error
  errc = floor(errc);

  if(val == 0){
    snprintf(str,strLength,"0(0)");
  }else if(fabs(val) > 0.001){
    //use normal notation
    snprintf(str,strLength,"%0.0f(%0.0f)",valc*pow(10,sigf),errc*pow(10,sigf));
  }else{
    //use scientific notation
    snprintf(str,strLength,"%0.0f(%0.0f)E%i",valc,errc,sigf);
  }

}