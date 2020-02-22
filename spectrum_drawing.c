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

//function setting the plotting limits for the spectrum based on the zoom level
void getPlotLimits(){
    if(zoomLevel <= 1.0){
        //set zoomed out
        zoomLevel = 1.0;
        lowerLimit = 0;
        upperLimit = S32K - 1;
        return;
    }else if(zoomLevel > 1000.0){
        zoomLevel = 1000.0; //set maximum zoom level
    }

    int numChansToDisp = (int)(1.0*S32K/zoomLevel);
    lowerLimit = xChanFocus - numChansToDisp/2;
    //clamp to lower limit of 0 if needed
    if(lowerLimit < 0){
        lowerLimit = 0;
        upperLimit = numChansToDisp - 1;
        return;
    }
    upperLimit = xChanFocus + numChansToDisp/2;
    //clamp to upper limit of S32K-1 if needed
    if(upperLimit > (S32K-1)){
        upperLimit=S32K-1;
        lowerLimit=S32K-1-numChansToDisp;
        return;
    }
}


//get the bin position in the histogram plot
float getXPos(int bin, float clip_x1, float clip_x2){
	return clip_x1 + (bin*(clip_x2-clip_x1)/(upperLimit-lowerLimit));
}

float getYPos(float val, float minVal, float maxVal, float clip_y1, float clip_y2){
    if(autoScale){
        scaleLevel = maxVal*1.1;
    }
	return clip_y1 + ((val/scaleLevel)*(clip_y2-clip_y1));
}

//draw a spectrum
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{

	if(openedSp){
		gtk_label_set_text(status_label, (const gchar *)"Spectrum loaded.");
	}else{
		gtk_label_set_text(status_label, (const gchar *)"No spectrum loaded.");
		return;
	}

	if (dispSp >= NSPECT)
	{
		printf("Spectrum number too high (%i)!\n", dispSp);
		return;
	}

    int i;
	GdkRectangle dasize;  // GtkDrawingArea size
    gdouble clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
    GdkWindow *wwindow = gtk_widget_get_window(widget);

    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);

    // Draw the background colour
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // transform the coordinate system
    cairo_translate(cr, 0.0, dasize.height); //so that the origin is at the lower left
    cairo_scale(cr, 1.0, -1.0); //so that positive y values go up

    // Determine the data points to calculate (ie. those in the clipping zone
    cairo_clip_extents(cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    cairo_set_line_width(cr, 3.0);

    getPlotLimits(); //setup the x range to plot over

    //get the maximum/minimum values of the displayed region
    float maxVal = SMALL_NUMBER;
    float minVal = BIG_NUMBER;
    for(i=0;i<(upperLimit-lowerLimit-1);i++){
        if(hist[dispSp][lowerLimit+i] > maxVal){
            maxVal = hist[dispSp][lowerLimit+i];
        }
        if(hist[dispSp][lowerLimit+i] < minVal){
            minVal = hist[dispSp][lowerLimit+i];
        }
    }

	//draw histogram
	for(i=0;i<(upperLimit-lowerLimit-1);i++){
		//printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,clip_x1,clip_x2), hist[dispSp][lowerLimit+i],hist[dispSp][lowerLimit+i],clip_x1,clip_x2);
		cairo_move_to (cr, getXPos(i,clip_x1,clip_x2), getYPos(hist[dispSp][lowerLimit+i],minVal,maxVal,clip_y1,clip_y2));
		cairo_line_to (cr, getXPos(i+1,clip_x1,clip_x2), getYPos(hist[dispSp][lowerLimit+i],minVal,maxVal,clip_y1,clip_y2));
		cairo_line_to (cr, getXPos(i+1,clip_x1,clip_x2), getYPos(hist[dispSp][lowerLimit+i+1],minVal,maxVal,clip_y1,clip_y2));
	}
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_stroke (cr);

    /*// Draw axes
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_move_to (cr, clip_x1, 0.0);
    cairo_line_to (cr, clip_x2, 0.0);
    cairo_move_to (cr, 0.0, clip_y1);
    cairo_line_to (cr, 0.0, clip_y2);
    cairo_stroke (cr);*/

    return;
}
