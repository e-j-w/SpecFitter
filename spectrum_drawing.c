int getFirstNonemptySpectrum(int numSpOpened){
  if(numSpOpened>=NSPECT){
    return -1;
  }
  int i,j;
  for(i=0;i<numSpOpened;i++){
    for(j=0;j<S32K;j++){
      if(hist[i][j]!=0.0){
        return i;
      }
    }
  }
  return -1;
}

//function setting the plotting limits for the spectrum based on the zoom level
//the plotting limits are in UNCALIBRATED units ie. channels
void getPlotLimits(){
    if(zoomLevel <= 1.0){
        //set zoomed out
        zoomLevel = 1.0;
        lowerLimit = 0;
        upperLimit = S32K - 1;
        return;
    }else if(zoomLevel > 1024.0){
        zoomLevel = 1024.0; //set maximum zoom level
    }

    int numChansToDisp = (int)(1.0*S32K/zoomLevel);
    lowerLimit = xChanFocus - numChansToDisp/2;
    lowerLimit = lowerLimit - (lowerLimit % contractFactor); //round to nearest multiple of contraction factor
    //clamp to lower limit of 0 if needed
    if(lowerLimit < 0){
        lowerLimit = 0;
        upperLimit = numChansToDisp - 1;
        return;
    }
    upperLimit = xChanFocus + numChansToDisp/2;
    upperLimit = upperLimit - (upperLimit % contractFactor); //round to nearest multiple of contraction factor
    //clamp to upper limit of S32K-1 if needed
    if(upperLimit > (S32K-1)){
        upperLimit=S32K-1;
        lowerLimit=S32K-1-numChansToDisp;
        return;
    }
}

void on_toggle_autoscale(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    autoScale=1;
  else
    autoScale=0;
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

//dragging gesture for spectra (left<->right panning)
void on_spectrum_drag_begin(GtkGestureDrag *gesture, gdouble start_x, gdouble start_y, gpointer user_data)
{
  dragstartul=upperLimit;
  dragstartll=lowerLimit;
  //printf("Drag started! dragstartll=%i, dragstartul=%i\n",dragstartll,dragstartul);
}
void on_spectrum_drag_update(GtkGestureDrag *gesture, gdouble offset_x, gdouble offset_y, gpointer user_data)
{
  //printf("Drag updated!\n");
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  upperLimit = dragstartul - ((2.*offset_x/(dasize.width))*(upperLimit-lowerLimit));
  lowerLimit = dragstartll - ((2.*offset_x/(dasize.width))*(upperLimit-lowerLimit));
  xChanFocus = lowerLimit + (upperLimit - lowerLimit)/2.;
  //printf("lowerlimit = %i, upperlimit = %i, width = %i, focus = %i\n",lowerLimit,upperLimit,dasize.width,xChanFocus);
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

//function handling mouse wheel scrolling to zoom the displayed spectrum
void on_spectrum_scroll(GtkWidget *widget, GdkEventScroll *e)
{
  if(e->direction == 1){
    //printf("Scrolling down at %f %f!\n",e->x,e->y);
    zoomLevel *= 0.5; 
  }else{
    //printf("Scrolling up at %f %f!\n",e->x,e->y);
    zoomLevel *= 2.0;
    GdkRectangle dasize;  // GtkDrawingArea size
    GdkWindow *wwindow = gtk_widget_get_window(widget);
    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry (wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
    xChanFocus = lowerLimit + (e->x/dasize.width)*(upperLimit - lowerLimit);
  }
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

//get the bin position in the histogram plot
float getXPos(int bin, float clip_x1, float clip_x2){
	return clip_x1 + (clip_x2-clip_x1)*0.1 + 0.9*(bin*(clip_x2-clip_x1)/(upperLimit-lowerLimit));
}

float getYPos(float val, float minVal, float maxVal, float clip_y1, float clip_y2){

  if(autoScale){
      scaleLevelMax = maxVal*1.1;
      scaleLevelMin = minVal;
  }
  float pos = clip_y1 + (clip_y2-clip_y1)*0.1 + 0.9*(((val - scaleLevelMin)/(scaleLevelMax - scaleLevelMin))*(clip_y2-clip_y1));
  if(pos < clip_y1 + (clip_y2-clip_y1)*0.1)
    pos = clip_y1 + (clip_y2-clip_y1)*0.1;
	return pos;
}

//axis tick drawing
float getAxisXPos(int axisVal, float clip_x1, float clip_x2){
  int calLowerLimit = lowerLimit;
  int calUpperLimit = upperLimit;
  if(calMode==1){
    //calibrate
    calLowerLimit = calpar0 + calpar1*lowerLimit + calpar2*lowerLimit*lowerLimit;
    calUpperLimit = calpar0 + calpar1*upperLimit + calpar2*upperLimit*upperLimit;
  }
  if((axisVal < calLowerLimit)||(axisVal >= calUpperLimit))
    return -1; //value is off the visible axis
  
  return (clip_x2-clip_x1)*0.1 + 0.9*(clip_x2-clip_x1)*(axisVal - calLowerLimit)/(calUpperLimit - calLowerLimit);
}
void drawXAxisTick(int axisVal, cairo_t *cr, float clip_x1, float clip_x2, float clip_y1, float clip_y2){
  int axisPos = getAxisXPos(axisVal,clip_x1,clip_x2);
  if(axisPos >= 0){
    //axis position is valid
    cairo_move_to (cr, axisPos, (clip_y1-clip_y2)*0.1);
    cairo_line_to (cr, axisPos, (clip_y1-clip_y2)*0.08);
    char tickLabel[20];
    sprintf(tickLabel,"%i",axisVal); //set string for label
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, 13);
    cairo_move_to(cr, axisPos - extents.width/2., (clip_y1-clip_y2)*0.05);
    cairo_show_text(cr, tickLabel);
  }
}
float getAxisYPos(float axisVal, float clip_y1, float clip_y2){
  
  return (clip_y1-clip_y2)*0.1 + 0.9*(clip_y1-clip_y2)*(axisVal - scaleLevelMin)/(scaleLevelMax - scaleLevelMin);
}
void drawYAxisTick(float axisVal, cairo_t *cr, float clip_x1, float clip_x2, float clip_y1, float clip_y2){
  if((axisVal < scaleLevelMin)||(axisVal >= scaleLevelMax)){
    //printf("axisval:%f,scalemin:%f,scalemax:%f\n",axisVal,scaleLevelMin,scaleLevelMax);
    return; //invalid axis value
  }
  if((axisVal!=0.0)&&(fabs(axisVal - 0) < (scaleLevelMax - scaleLevelMin)/20.0)){
    //tick is too close to zero, don't draw
    return;
  }
  float axisPos = getAxisYPos(axisVal,clip_y1,clip_y2);
  if(axisPos <= 0){
    //axis position is valid
    cairo_move_to (cr,  (clip_x2-clip_x1)*0.1, axisPos);
    cairo_line_to (cr,  (clip_x2-clip_x1)*0.09, axisPos);
    char tickLabel[20];
    sprintf(tickLabel,"%0.1f",axisVal); //set string for label
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, 13);
    cairo_move_to(cr, (clip_x2-clip_x1)*0.08 - extents.width, axisPos + extents.height/2.);
    cairo_show_text(cr, tickLabel);
  }
}

//get the x range of the plot in terms of x axis units, 
//taking into account whether or not a calibration is in use
int getPlotRangeXUnits(){
  int calLowerLimit = lowerLimit;
  int calUpperLimit = upperLimit;
  if(calMode==1){
    //calibrate
    calLowerLimit = calpar0 + calpar1*lowerLimit + calpar2*lowerLimit*lowerLimit;
    calUpperLimit = calpar0 + calpar1*upperLimit + calpar2*upperLimit*upperLimit;
  }
  return calUpperLimit - calLowerLimit;
}

//draw a spectrum
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{

	if(!openedSp){
		return;
	}

	if (glob_multiPlots[0] >= NSPECT)
	{
		printf("Spectrum number too high (%i)!\n", glob_multiPlots[0]);
		return;
	}

  int i,j;
	GdkRectangle dasize;  // GtkDrawingArea size
  gdouble clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
  GdkWindow *wwindow = gtk_widget_get_window(widget);

  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);

  // Draw the background colour
  //cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  //cairo_paint(cr);

  // transform the coordinate system
  cairo_translate(cr, 0.0, dasize.height); //so that the origin is at the lower left
  cairo_scale(cr, 1.0, -1.0); //so that positive y values go up

  // Determine the data points to calculate (ie. those in the clipping zone
  cairo_clip_extents(cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
  cairo_set_line_width(cr, 2.0);

  getPlotLimits(); //setup the x range to plot over

  //get the maximum/minimum y values of the displayed region
  float maxVal = SMALL_NUMBER;
  float minVal = BIG_NUMBER;
  for(i=0;i<(upperLimit-lowerLimit-1);i+=contractFactor){
    float currentVal = 0.;
    for(j=0;j<contractFactor;j++){
      currentVal += hist[glob_multiPlots[0]][lowerLimit+i+j];
    }
    if(currentVal > maxVal){
        maxVal = currentVal;
    }
    if(currentVal < minVal){
        minVal = currentVal;
    }
  }
  //printf("maxVal = %f, minVal = %f\n",maxVal,minVal);

	//draw the actual histogram
	for(i=0;i<(upperLimit-lowerLimit-1);i+=contractFactor){
    float currentVal = 0.;
    float nextVal = 0.;
    for(j=0;j<contractFactor;j++){
      currentVal += hist[glob_multiPlots[0]][lowerLimit+i+j];
      nextVal += hist[glob_multiPlots[0]][lowerLimit+i+j+contractFactor];
    }
		//printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,clip_x1,clip_x2), hist[glob_multiPlots[0]][lowerLimit+i],hist[glob_multiPlots[0]][lowerLimit+i],clip_x1,clip_x2);
		cairo_move_to (cr, getXPos(i,clip_x1,clip_x2), getYPos(currentVal,minVal,maxVal,clip_y1,clip_y2));
		cairo_line_to (cr, getXPos(i+contractFactor,clip_x1,clip_x2), getYPos(currentVal,minVal,maxVal,clip_y1,clip_y2));
		cairo_line_to (cr, getXPos(i+contractFactor,clip_x1,clip_x2), getYPos(nextVal,minVal,maxVal,clip_y1,clip_y2));
	}
  cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
  cairo_stroke(cr);

  // draw axes
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  cairo_move_to (cr, (clip_x2-clip_x1)*0.1, (clip_y2-clip_y1)*0.1);
  cairo_line_to (cr, (clip_x2-clip_x1)*0.1, clip_y2);
  cairo_move_to (cr, (clip_x2-clip_x1)*0.1, (clip_y2-clip_y1)*0.1);
  cairo_line_to (cr, clip_x2, (clip_y2-clip_y1)*0.1);
  cairo_stroke(cr);

  //draw ticks
  cairo_scale(cr, 1.0, -1.0); //remove axis inversion, so that text is the right way up
  if(getPlotRangeXUnits() > 20000){
    for(i=0;i<S32K;i+=5000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 10000){
    for(i=0;i<S32K;i+=2000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 5000){
    for(i=0;i<S32K;i+=1000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 3000){
    for(i=0;i<S32K;i+=500){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 2000){
    for(i=0;i<S32K;i+=300){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 1000){
    for(i=0;i<S32K;i+=200){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 500){
    for(i=0;i<S32K;i+=100){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 200){
    for(i=0;i<S32K;i+=50){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else if(getPlotRangeXUnits() > 50){
    for(i=0;i<S32K;i+=20){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else{
    for(i=0;i<S32K;i+=5){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }
  if(scaleLevelMax - scaleLevelMin > 10.0){
    for(i=0;i<10;i++){
      drawYAxisTick(scaleLevelMin + (scaleLevelMax - scaleLevelMin)*i/10.0, cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }else{
    for(i=0;i<(int)(scaleLevelMax - scaleLevelMin);i++){
      drawYAxisTick(scaleLevelMin + (scaleLevelMax - scaleLevelMin)*i/(scaleLevelMax - scaleLevelMin), cr, clip_x1, clip_x2, clip_y1, clip_y2);
    }
  }
  drawYAxisTick(0.0, cr, clip_x1, clip_x2, clip_y1, clip_y2); //always draw the zero label on the y axis
  cairo_stroke(cr);

  //draw the zero line if applicable
  if((scaleLevelMin < 0.0) && (scaleLevelMax > 0.0)){
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_move_to (cr, (clip_x2-clip_x1)*0.1, getAxisYPos(0.0,clip_y1,clip_y2));
    cairo_line_to (cr, clip_x2, getAxisYPos(0.0,clip_y1,clip_y2));
    cairo_stroke(cr);
  }

  //draw axis labels
  cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  char axisLabel[16];
  cairo_text_extents_t extents; //for getting dimensions needed to center text labels
  //x axis
  if(calMode == 0){
    sprintf(axisLabel,"Channel #"); //set string for label
  }else{
    strcpy(axisLabel,calUnit); //set label to calibrated units
  }
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, 14);
  cairo_move_to(cr, (clip_x2-clip_x1)*0.55 - (extents.width/2), (clip_y1-clip_y2)*0.005);
  cairo_show_text(cr, axisLabel);
  //y axis
  sprintf(axisLabel,"Counts"); //set string for label
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, 14);
  cairo_move_to(cr, (clip_x2-clip_x1)*0.015, (clip_y1-clip_y2)*0.5);
  cairo_save(cr); //store the context before the rotation
  cairo_rotate(cr, 1.5*3.14159);
  cairo_translate(cr, (clip_x2-clip_x1)*0.015, -1.0*((clip_y1-clip_y2)*0.5)); //so that the origin is at the lower left
  cairo_show_text(cr, axisLabel);
  cairo_stroke(cr);
  cairo_restore(cr); //recall the unrotated context


  return;
}
