
void on_spectrum_scroll(GtkWidget *widget, GdkEventScroll *e)
{

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *wwindow = gtk_widget_get_window(widget);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  xChanFocus = lowerLimit + (e->x/dasize.width)*(upperLimit - lowerLimit);

  if(e->direction == 1){
    //printf("Scrolling down at %f %f!\n",e->x,e->y);
    if(zoomLevel < 10.0){
      zoomLevel -= 1.0;
    }else if(zoomLevel < 50.0){
      zoomLevel -= 2.0;
    }else{
      zoomLevel -= 5.0;
    }
    
  }else{
    //printf("Scrolling up at %f %f!\n",e->x,e->y);
    if(zoomLevel < 10.0){
      zoomLevel += 1.0;
    }else if(zoomLevel < 50.0){
      zoomLevel += 2.0;
    }else{
      zoomLevel += 5.0;
    }
  }
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

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
        return clip_y1 + ((val/(maxVal*1.1))*(clip_y2-clip_y1));
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
