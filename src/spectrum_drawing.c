/* © J. Williams, 2020-2023 */

//This file contains routines for drawing spectra using Cairo.
//The main routine is drawSpectrum (near the bottom), helper 
//subroutines are above it.

#define XORIGIN 85.0 //x-origin of drawn spectra
#define YORIGIN 45.0 //y-origin of drawn spectra

//forward declarations
void on_fit_fit_button_clicked();

//set the default text color, depending on the theme
void setTextColor(cairo_t *cr){
  if(guiglobals.preferDarkTheme){
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
  }else{
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
  }
}
//set the default text color, depending on the theme
void setGridLineColor(cairo_t *cr){
  if(guiglobals.preferDarkTheme){
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
  }else{
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
  } 
}

//converts cursor position units to channel units on the displayed spectrum
//return value is float to allow sub-channel prescision, cast it to int32_t if needed
float getCursorChannel(const double cursorx, const double cursory){
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > XORIGIN)&&(cursory < (dasize.height - YORIGIN))){
    
    float cursorChan = (float)(drawing.lowerLimit + (((cursorx)-XORIGIN)/(dasize.width-XORIGIN))*(drawing.upperLimit - drawing.lowerLimit));
    //printf("chan: %f\n",cursorChan);
    return cursorChan;
    //return cursorChan - fmod(cursorChan,drawing.contractFactor);
  }
  return -1; //cursor not over spectrum
}

//converts cursor position units to y-value on the displayed spectrum
//this is the value on the displayed y-axis, at the cursor postion
float getCursorYVal(const double cursorx, const double cursory){
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > XORIGIN)&&(cursory < (dasize.height - YORIGIN))){
    float cursorVal;
    switch(drawing.multiplotMode){
      case MULTIPLOT_SUMMED:
      case MULTIPLOT_NONE:
        //single plot mode
        if(drawing.logScale){
          if(drawing.scaleLevelMin[0] > 0){
            cursorVal = powf(10.0f,(float)((dasize.height-YORIGIN - cursory)*log10(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/(dasize.height-YORIGIN))) + drawing.scaleLevelMin[0];
          }else{
            cursorVal = powf(10.0f,(float)((dasize.height-YORIGIN - cursory)*log10(drawing.scaleLevelMax[0])/(dasize.height-YORIGIN)));
          }
        }else{
          cursorVal = drawing.scaleLevelMax[0] - (float)(((cursory)/(dasize.height-YORIGIN)))*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0]);
          //printf("cursorVal: %f, scaleLevelMin: %f, scaleLevelMax: %f\n",cursorVal,drawing.scaleLevelMin[0],drawing.scaleLevelMax[0]);
        }
        break;
      default:
        return 0; //not implemented
        break;
    }
    //printf("cursory: %f, cursorVal: %f, scaleMax: %f, scaleMin: %f\n",cursory,cursorVal,drawing.scaleLevelMax[0],drawing.scaleLevelMin[0]);
    return cursorVal;
    //return cursorChan - fmod(cursorChan,drawing.contractFactor);
  }
  return 0; //cursor not over spectrum
}

//get the index of the comment at which the cursor is over
//return -1 if no comment is at the cursor position
//some shameless magic numbers used to map channel and y-values to comment indicator size
int getCommentAtCursor(const double cursorx, const double cursory){

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > XORIGIN)&&(cursory < ((double)dasize.height - YORIGIN))){
    float cursorCh = getCursorChannel(cursorx, cursory);
    float cursorYVal = getCursorYVal(cursorx, cursory);
    //printf("cursorCh: %f, cursorYVal: %f\n",cursorCh,cursorYVal);
    switch(drawing.multiplotMode){
      case MULTIPLOT_SUMMED:
        //sum plot mode
        for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
          if(rawdata.chanCommentView[i] == 1){
            if(rawdata.chanCommentSp[i] == drawing.displayedView){
              //check proximity to channel
              if(fabsf((float)rawdata.chanCommentCh[i] - cursorCh) < (30.0f*((float)(drawing.upperLimit - drawing.lowerLimit)/(float)dasize.width))){
                //check proximity to y-val
                float chYVal = rawdata.chanCommentVal[i];
                if(chYVal < drawing.scaleLevelMin[0]){
                  chYVal = drawing.scaleLevelMin[0];
                }else if(chYVal > drawing.scaleLevelMax[0]){
                  chYVal = drawing.scaleLevelMax[0];
                }
                if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/(float)dasize.height)){
                  return i; //this comment is close
                }
              }
            }
          }
        }
        break;
      case MULTIPLOT_NONE:
        //single plot mode
        if(drawing.displayedView == -1){
          for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i] == 0){
              if(rawdata.chanCommentSp[i] == drawing.multiPlots[0]){
                //check proximity to channel
                if(fabsf((float)rawdata.chanCommentCh[i] - cursorCh) < (30.0f*((float)(drawing.upperLimit - drawing.lowerLimit)/(float)dasize.width))){
                  //check proximity to y-val
                  float chYVal = rawdata.chanCommentVal[i];
                  if(chYVal < drawing.scaleLevelMin[0]){
                    chYVal = drawing.scaleLevelMin[0];
                  }else if(chYVal > drawing.scaleLevelMax[0]){
                    chYVal = drawing.scaleLevelMax[0];
                  }
                  if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/(float)dasize.height)){
                    return i; //this comment is close
                  }
                }
              }
            }
          }
        }else{
          //scaled single spectrum view
          for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i] == 1){
              if(rawdata.chanCommentSp[i] == drawing.displayedView){
                //check proximity to channel
                if(fabsf((float)rawdata.chanCommentCh[i] - cursorCh) < (30.0f*((float)(drawing.upperLimit - drawing.lowerLimit)/(float)dasize.width))){
                  //check proximity to y-val
                  float chYVal = rawdata.chanCommentVal[i];
                  if(chYVal < drawing.scaleLevelMin[0]){
                    chYVal = drawing.scaleLevelMin[0];
                  }else if(chYVal > drawing.scaleLevelMax[0]){
                    chYVal = drawing.scaleLevelMax[0];
                  }
                  if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/(float)dasize.height)){
                    return i; //this comment is close
                  }
                }
              }
            }
          }
        }
        
        break;
      default:
        break;
    }
  }
  return -1; //cursor not over a comment, or comments not implemented for the drawing mode
}


//function setting the plotting limits for the spectrum based on the zoom level
//the plotting limits are in UNCALIBRATED units ie. channels
void setPlotLimits(){
  if(drawing.zoomLevel <= 1.0){
    //set zoomed out
    drawing.zoomLevel = 1.0;
    drawing.lowerLimit = 0;
    drawing.upperLimit = S32K - 1;
    return;
  }else if(drawing.zoomLevel > 1024.0){
    drawing.zoomLevel = 1024.0; //set maximum zoom level
  }

  if(drawing.zoomFocusFrac > 1.0){
    drawing.zoomFocusFrac = 1.0;
  }else if(drawing.zoomFocusFrac < 0.0){
    drawing.zoomFocusFrac = 0.0;
  }

  int32_t numChansToDisp = (int)(1.0*S32K/drawing.zoomLevel);
  drawing.lowerLimit = drawing.xChanFocus - (int)((float)numChansToDisp*drawing.zoomFocusFrac);
  drawing.lowerLimit = drawing.lowerLimit - (drawing.lowerLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
  //clamp to lower limit of 0 if needed
  if(drawing.lowerLimit < 0){
    drawing.lowerLimit = 0;
    drawing.upperLimit = numChansToDisp - 1;
    return;
  }
  drawing.upperLimit = drawing.xChanFocus + (int)(numChansToDisp*(1.0 - drawing.zoomFocusFrac));
  drawing.upperLimit = drawing.upperLimit - (drawing.upperLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
  //clamp to upper limit of S32K-1 if needed
  if(drawing.upperLimit > (S32K-1)){
    drawing.upperLimit=S32K-1;
    drawing.lowerLimit=S32K-1-numChansToDisp;
    return;
  }
}

//zoom to the non-zero region of the spectrum
void autoZoom(){
  if(drawing.multiplotMode == MULTIPLOT_NONE){
    for(int32_t i=0;i<S32K;i++){
      if(rawdata.hist[drawing.multiPlots[0]][i] != 0.){
        drawing.lowerLimit = i;
        break;
      }
    }
    for(int32_t i=S32K-1;i>=0;i--){
      if(rawdata.hist[drawing.multiPlots[0]][i] != 0.){
        drawing.upperLimit = i;
        break;
      }
    }
    drawing.xChanFocus = (int)((drawing.upperLimit + drawing.lowerLimit)/2.0);
    drawing.zoomFocusFrac = 0.5;
    int32_t numChansToDisp = drawing.upperLimit - drawing.lowerLimit;
    if(numChansToDisp > 0){
      drawing.zoomLevel = (float)(1.0*S32K/numChansToDisp);
    }

    //obey zoom limits
    if(drawing.zoomLevel < 1.0){
      drawing.zoomLevel = 1.0;
    }else if(drawing.zoomLevel > 1024.0){
      drawing.zoomLevel = 1024.0; //set maximum zoom level
    }
      
  }
  //printf("lowerLimit: %i, upperLimit: %i, xChanFocus: %i, zoomLevel: %f\n",drawing.lowerLimit,drawing.upperLimit,drawing.xChanFocus, drawing.zoomLevel);
}

gboolean zoom_y_callback(){

  if(drawing.zoomingSpY == 0){
    return G_SOURCE_REMOVE;
  }

  gint64 frameTime = gdk_frame_clock_get_frame_time(frameClock);
  float linFac = 0.00001f*(float)(frameTime-drawing.zoomYLastFrameTime);
  float diffFac = 0.00002f*(float)(frameTime-drawing.zoomYLastFrameTime);
  int32_t i;
  for(i=0;i<drawing.numMultiplotSp;i++){
    if(drawing.scaleToLevelMax[i] == drawing.scaleLevelMin[i]){
      drawing.scaleLevelMax[i] = drawing.scaleLevelMin[i];
    }else if(drawing.scaleLevelMax[i] > drawing.scaleToLevelMax[i]){
      drawing.scaleLevelMax[i] -= diffFac*(drawing.scaleLevelMax[i]-drawing.scaleToLevelMax[i]) + linFac*fabsf(drawing.scaleLevelMax[i]);
      if(drawing.scaleLevelMax[i] <= drawing.scaleToLevelMax[i]){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
      }
    }else if(drawing.scaleLevelMax[i] < drawing.scaleToLevelMax[i]){
      drawing.scaleLevelMax[i] += diffFac*(drawing.scaleToLevelMax[i]-drawing.scaleLevelMax[i]) + linFac*fabsf(drawing.scaleLevelMax[i]);
      if(drawing.scaleLevelMax[i] >= drawing.scaleToLevelMax[i]){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
      }
    }
    if(drawing.scaleLevelMin[i] < drawing.scaleToLevelMin[i]){
      drawing.scaleLevelMin[i] += diffFac*(drawing.scaleToLevelMin[i]-drawing.scaleLevelMin[i]) + linFac*fabsf(drawing.scaleLevelMin[i]);
      if(drawing.scaleLevelMin[i] >= drawing.scaleToLevelMin[i]){
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }else if(drawing.scaleLevelMin[i] > drawing.scaleToLevelMin[i]){
      drawing.scaleLevelMin[i] -= diffFac*(drawing.scaleLevelMin[i]-drawing.scaleToLevelMin[i]) + linFac*fabsf(drawing.scaleLevelMin[i]);
      if(drawing.scaleLevelMin[i] <= drawing.scaleToLevelMin[i]){
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }
  }
  for(i=0;i<drawing.numMultiplotSp;i++){
    //printf("scaleMin: %f, scaleToMin: %f,   scaleMax: %f, scaleToMax: %f\n",drawing.scaleLevelMin[i],drawing.scaleToLevelMin[i],drawing.scaleLevelMax[i],drawing.scaleToLevelMax[i]);
    if((drawing.scaleLevelMax[i] != drawing.scaleToLevelMax[i])||(drawing.scaleLevelMin[i] != drawing.scaleToLevelMin[i])){
      drawing.zoomYLastFrameTime=frameTime;
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
      return G_SOURCE_CONTINUE;
    }
  }
  //printf("Finished y zoom.\n");
  drawing.zoomingSpY = 0; //finished zooming
  drawing.zoomYLastFrameTime=0;
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  return G_SOURCE_REMOVE;
}

gboolean zoom_in_tick_callback(){
  gint64 frameTime = gdk_frame_clock_get_frame_time(frameClock);
  //printf("Zooming x in, frame time %li\n",frameTime-drawing.zoomXLastFrameTime);
  drawing.zoomLevel *= 1.0f + 0.000009f*(float)(frameTime-drawing.zoomXLastFrameTime);
  if((drawing.zoomLevel > drawing.zoomToLevel)||(drawing.zoomLevel > 1024.0)){
    drawing.zoomLevel = drawing.zoomToLevel;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    drawing.zoomingSpX = 0;
    drawing.zoomXLastFrameTime=0;
    return G_SOURCE_REMOVE;
  }
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  drawing.zoomXLastFrameTime=frameTime;
  return G_SOURCE_CONTINUE;
}

gboolean zoom_out_tick_callback(){
  gint64 frameTime = gdk_frame_clock_get_frame_time(frameClock);
  //printf("Zooming x out\n");
  drawing.zoomLevel *= 1.0f - 0.000008f*(float)(frameTime-drawing.zoomXLastFrameTime);
  if((drawing.zoomLevel < drawing.zoomToLevel)||(drawing.zoomLevel < 1.0)){
    drawing.zoomLevel = drawing.zoomToLevel;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    drawing.zoomingSpX = 0;
    drawing.zoomXLastFrameTime=0;
    return G_SOURCE_REMOVE;
  }
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  drawing.zoomXLastFrameTime=frameTime;
  return G_SOURCE_CONTINUE;
}

//zoom in on the spectrum, not following the cursor (for use with keyboard shortcut)
void on_zoom_in_x(){
  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }
  if(guiglobals.useZoomAnimations){
    drawing.zoomToLevel = drawing.zoomLevel * 2.0f;
    if(drawing.zoomToLevel > 1024.0)
      drawing.zoomToLevel = 1024.0;
    drawing.zoomFocusFrac = 0.5;
    drawing.zoomingSpX = 1;
    drawing.zoomXStartFrameTime = gdk_frame_clock_get_frame_time(frameClock);
    drawing.zoomXLastFrameTime = drawing.zoomXStartFrameTime;
    gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_in_tick_callback, NULL, NULL);
  }else{
    drawing.zoomLevel *= 2.0f;
    if(drawing.zoomLevel > 1024.0)
      drawing.zoomLevel = 1024.0;
    drawing.zoomFocusFrac = 0.5;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }
}

void on_zoom_out_x(){
  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }
  if(guiglobals.useZoomAnimations){
    drawing.zoomToLevel = drawing.zoomLevel * 0.5f;
    if(drawing.zoomToLevel < 1.0)
      drawing.zoomToLevel = 1.0;
    drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
    drawing.zoomFocusFrac = 0.5;
    drawing.zoomingSpX = 1;
    drawing.zoomXStartFrameTime = gdk_frame_clock_get_frame_time(frameClock);
    drawing.zoomXLastFrameTime = drawing.zoomXStartFrameTime;
    gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_out_tick_callback, NULL, NULL);
  }else{
    drawing.zoomLevel *= 0.5f;
    if(drawing.zoomLevel < 1.0)
      drawing.zoomLevel = 1.0;
    drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
    drawing.zoomFocusFrac = 0.5;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }
}

//function handling mouse wheel scrolling to zoom the displayed spectrum
void on_spectrum_scroll(GtkWidget *widget, GdkEventScroll *e){
  if(!rawdata.openedSp){
    return;
  }

  if(e->x < XORIGIN){
    //out of plot range
    return;
  }

  //get scroll direction
  //printf("direction: %i, delta_x: %f, delta_y: %f\n",e->direction,e->delta_x,e->delta_y);
  if((e->direction == GDK_SCROLL_UP)||(e->direction == GDK_SCROLL_DOWN)){
    guiglobals.scrollDir = e->direction;
  }else if(e->direction == GDK_SCROLL_SMOOTH){
    if(e->delta_y==1.000){
      //mouse wheel down
      guiglobals.scrollDir = GDK_SCROLL_DOWN;
      guiglobals.accSmoothScrollDelta = 1.0;
    }else if(e->delta_y==-1.000){
      //mouse wheel up
      guiglobals.scrollDir = GDK_SCROLL_UP;
      guiglobals.accSmoothScrollDelta = 1.0;
    }else if(e->delta_y>0.0){
      //touchpad scroll down
      if(guiglobals.scrollDir != GDK_SCROLL_DOWN){
        guiglobals.scrollDir = GDK_SCROLL_DOWN;
        return; //skip a frame to account for spurious touchpad inputs
      }else{
        guiglobals.accSmoothScrollDelta = guiglobals.accSmoothScrollDelta + 0.25*e->delta_y;
      }
    }else if(e->delta_y<0.0){
      //touchpad scroll up
      if(guiglobals.scrollDir != GDK_SCROLL_UP){
        guiglobals.scrollDir = GDK_SCROLL_UP;
        return; //skip a frame to account for spurious touchpad inputs
      }else{
        guiglobals.accSmoothScrollDelta = guiglobals.accSmoothScrollDelta - 0.25*e->delta_y;
      }
    }else if((e->delta_x==0.0)&&(e->delta_y==0.0)){
      //GTK bug lol (https://bugzilla.gnome.org/show_bug.cgi?id=675959)
      if(drawing.zoomLevel > 1.0){
        guiglobals.scrollDir = GDK_SCROLL_UP;
        guiglobals.accSmoothScrollDelta = 1.0;
      }
    }else{
      guiglobals.accSmoothScrollDelta = 0.;
      return;
    }
  }else{
    return;
  }

  if((guiglobals.scrollDir == GDK_SCROLL_DOWN)&&(drawing.zoomLevel > 1.0)){
    //printf("Scrolling down at %f %f!\n",e->x,e->y);
    on_zoom_out_x();
    //handle zooming that follows cursor
    GdkRectangle dasize;  // GtkDrawingArea size
    GdkWindow *wwindow = gtk_widget_get_window(widget);
    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
    if(guiglobals.useZoomAnimations){
      if(drawing.zoomingSpX){
        return;
      }
      if(e->direction == GDK_SCROLL_SMOOTH){
        if(guiglobals.accSmoothScrollDelta < 0.25){
          return;
        }
        //printf("zoom out: %f\n",guiglobals.accSmoothScrollDelta);
        drawing.zoomToLevel = drawing.zoomLevel * (float)(1.0/guiglobals.accSmoothScrollDelta);
        guiglobals.accSmoothScrollDelta = 0.0; //reset
      }else{
        drawing.zoomToLevel = drawing.zoomLevel * 0.5f;
      }
      if((drawing.upperLimit - drawing.lowerLimit)>0){
        drawing.xChanFocus = drawing.lowerLimit + (int)(((e->x)-XORIGIN)/(dasize.width-XORIGIN)*(drawing.upperLimit - drawing.lowerLimit));
        drawing.zoomFocusFrac = (float)((drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit));
      }else{
        drawing.zoomFocusFrac = 0.5f;
      }
      drawing.zoomingSpX = 1;
      drawing.zoomXLastFrameTime = gdk_frame_clock_get_frame_time(frameClock);
      gtk_widget_add_tick_callback(widget, zoom_out_tick_callback, NULL, NULL);
    }else{
      if((drawing.upperLimit - drawing.lowerLimit)>0){
        drawing.xChanFocus = drawing.lowerLimit + (int)(((e->x)-XORIGIN)/(dasize.width-XORIGIN)*(drawing.upperLimit - drawing.lowerLimit));
        drawing.zoomFocusFrac = (float)((drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit));
      }else{
        drawing.zoomFocusFrac = 0.5f;
      }
      drawing.zoomLevel *= 0.5f;
      gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }else if((guiglobals.scrollDir != GDK_SCROLL_DOWN)&&(drawing.zoomLevel < 1024.0)){
    //handle zooming that follows cursor
    //printf("Scrolling up at %f %f!\n",e->x,e->y);
    GdkRectangle dasize;  // GtkDrawingArea size
    GdkWindow *wwindow = gtk_widget_get_window(widget);
    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
    if(guiglobals.useZoomAnimations){
      if(drawing.zoomingSpX){
        return;
      }
      if(e->direction == GDK_SCROLL_SMOOTH){
        if(guiglobals.accSmoothScrollDelta < 0.25){
          return;
        }
        //printf("zoom in: %f\n",guiglobals.accSmoothScrollDelta);
        drawing.zoomToLevel = drawing.zoomLevel * (float)(1.0 + guiglobals.accSmoothScrollDelta);
        guiglobals.accSmoothScrollDelta = 0.0; //reset
      }else{
        drawing.zoomToLevel = drawing.zoomLevel * 2.0f;
      }
      if((drawing.upperLimit - drawing.lowerLimit)>0){
        drawing.xChanFocus = drawing.lowerLimit + (int)(((e->x)-XORIGIN)/(dasize.width-XORIGIN)*(drawing.upperLimit - drawing.lowerLimit));
        drawing.zoomFocusFrac = (float)((drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit));
      }else{
        drawing.zoomFocusFrac = 0.5f;
      }
      drawing.zoomingSpX = 1;
      drawing.zoomXLastFrameTime = gdk_frame_clock_get_frame_time(frameClock);
      gtk_widget_add_tick_callback(widget, zoom_in_tick_callback, NULL, NULL);
    }else{
      if((drawing.upperLimit - drawing.lowerLimit)>0){
        drawing.xChanFocus = drawing.lowerLimit + (int)(((e->x)-XORIGIN)/(dasize.width-XORIGIN)*(drawing.upperLimit - drawing.lowerLimit));
        drawing.zoomFocusFrac = (float)((drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit));
      }else{
        drawing.zoomFocusFrac = 0.5f;
      }
      drawing.zoomLevel *= 2.0f;
      gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }
  
}

void on_spectrum_click(GtkWidget *widget, GdkEventButton *event){
  
  if(widget==NULL){
    printf("on_spectrum_click - no widget.\n");
    return;
  }

  if((event->type == GDK_BUTTON_PRESS) && (event->button == 3)){
    //right mouse button being pressed
    float cursorChan = getCursorChannel(event->x, event->y);
    switch(guiglobals.fittingSp){
      case FITSTATE_FITCOMPLETE:
        //fit being displayed, clear it on right click
        guiglobals.fittingSp = FITSTATE_NOTFITTING;
        update_gui_fit_state();
        break;
      case FITSTATE_SETTINGPEAKS:
        //setup peak positions
        if(fitpar.numFitPeaks < MAX_FIT_PK){
          if((cursorChan >= fitpar.fitStartCh)&&(cursorChan <= fitpar.fitEndCh)){
            fitpar.fitPeakInitGuess[fitpar.numFitPeaks] = cursorChan - 0.5f;
            printf("Fitting peak at channel %f\n",fitpar.fitPeakInitGuess[fitpar.numFitPeaks]);
            gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),TRUE);
            fitpar.numFitPeaks++;
          }
        }
        if(fitpar.numFitPeaks >= MAX_FIT_PK){
          printf("Maximum number of fit peaks specified.\n");
          fitpar.numFitPeaks = MAX_FIT_PK;
          startGausFit(); //force fit to proceed
        }
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
        break;
      case FITSTATE_SETTINGLIMITS:
        //setup fitting limit
        if(fitpar.fitEndCh < 0){
          if(fitpar.fitStartCh < 0){
            fitpar.fitStartCh = (int)cursorChan;
          }else{
            if(cursorChan > fitpar.fitStartCh){
              fitpar.fitEndCh = (int)cursorChan;
            }else if(cursorChan < fitpar.fitStartCh){
              fitpar.fitEndCh = fitpar.fitStartCh; //swap
              fitpar.fitStartCh = (int)cursorChan;
            }
          }
        }
        //check if both limits have been set
        if((fitpar.fitStartCh >= 0)&&(fitpar.fitEndCh >=0)){
          printf("Fit limits: channel %i through %i\n",fitpar.fitStartCh,fitpar.fitEndCh);
          if((fitpar.fitType == FITTYPE_BGONLY)||(fitpar.fitType == FITTYPE_SUMREGION)){
            //background only or sum region fit, start the fit right away
            on_fit_fit_button_clicked(); //gui.c
          }else{
            guiglobals.fittingSp = FITSTATE_SETTINGPEAKS;
          }
          update_gui_fit_state();
        }
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
        break;
      case FITSTATE_NOTFITTING:
      default:
        break;
    }
  }else if((event->type == GDK_DOUBLE_BUTTON_PRESS) && (event->button == 1)){
    //double click
    if(rawdata.openedSp){
      float cursorChan, cursorYVal;
      cursorChan = getCursorChannel(event->x, event->y);
      cursorYVal = getCursorYVal(event->x, event->y);
      if(cursorChan >= 0){
        //user has double clicked on the displayed spectrum
        switch(drawing.multiplotMode){
          case MULTIPLOT_SUMMED:
            //summed single plot
            //offer option to create a new summed spectrum and comment on that
            guiglobals.commentEditInd = getCommentAtCursor(event->x, event->y);
            if((guiglobals.commentEditInd >= 0)&&(guiglobals.commentEditInd < NCHCOM)){
              guiglobals.commentEditMode=0;
              gtk_window_set_title(comment_window,"Edit Comment");
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_widget_set_visible(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_entry_set_text(comment_entry, rawdata.chanComment[guiglobals.commentEditInd]);
              gtk_button_set_label(comment_ok_button,"Apply");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }else{
              if(rawdata.numChComments >= NCHCOM){
                printf("Cannot add any more comments.\n");
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"The maximum number of comments has been reached.  Older comments must be deleted before more can be added.");
                gtk_dialog_run(GTK_DIALOG(message_dialog));
                gtk_widget_destroy(message_dialog);
                return;
              }
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),FALSE);
              //setup comment data
              rawdata.chanCommentVal[(int)rawdata.numChComments] = cursorYVal;
              rawdata.chanCommentCh[(int)rawdata.numChComments] = (int)cursorChan;
              rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
              if(drawing.displayedView >= 0){
                rawdata.chanCommentSp[(int)rawdata.numChComments] = (uint8_t)drawing.displayedView;
                gtk_button_set_label(comment_ok_button,"Apply");
              }else if(drawing.displayedView == -2){
                //this is a view that hasn't been saved yet
                if(rawdata.numViews >= MAXNVIEWS){
                  printf("Cannot add any more views.\n");
                  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                  GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Adding a comment would require the current view to be saved, however the maximum number of custom views has been reached.  Older custom views must be deleted before more can be added.");
                  gtk_dialog_run(GTK_DIALOG(message_dialog));
                  gtk_widget_destroy(message_dialog);
                  return;
                }
                rawdata.chanCommentSp[(int)rawdata.numChComments] = rawdata.numViews;
                gtk_button_set_label(comment_ok_button,"Save View and Apply");
              }else{
                printf("WARNING: undefined view state, not displaying edit window.\n");
              }
              guiglobals.commentEditMode=0;
              gtk_window_set_title(comment_window,"Add Comment");
              gtk_widget_set_visible(GTK_WIDGET(remove_comment_button),FALSE);
              gtk_entry_set_text(comment_entry, "");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }
            //re-enable comment display
            guiglobals.drawSpComments=1;
            //show the window to edit the comment
            gtk_window_present(comment_window);
            break;
          case MULTIPLOT_NONE:
            //single plot being displayed
            //open a dialog for the user to write a comment
            //printf("Double click on channel %f, value %f\n",cursorChan,cursorYVal);
            guiglobals.commentEditInd = getCommentAtCursor(event->x, event->y);
            if((guiglobals.commentEditInd >= 0)&&(guiglobals.commentEditInd < NCHCOM)){
              guiglobals.commentEditMode=0;
              gtk_window_set_title(comment_window,"Edit Comment");
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_widget_set_visible(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_entry_set_text(comment_entry, rawdata.chanComment[guiglobals.commentEditInd]);
              gtk_button_set_label(comment_ok_button,"Apply");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }else{
              if(rawdata.numChComments >= NCHCOM){
                printf("Cannot add any more comments.\n");
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"The maximum number of comments has been reached.  Older comments must be deleted before more can be added.");
                gtk_dialog_run(GTK_DIALOG(message_dialog));
                gtk_widget_destroy(message_dialog);
                return;
              }
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),FALSE);
              //setup comment data
              rawdata.chanCommentVal[(int)rawdata.numChComments] = cursorYVal;
              rawdata.chanCommentCh[(int)rawdata.numChComments] = (int)cursorChan;
              if(drawing.displayedView == -1){
                //commenting on a raw spectrum, not a view
                rawdata.chanCommentSp[(int)rawdata.numChComments] = drawing.multiPlots[0];
                rawdata.chanCommentView[(int)rawdata.numChComments] = 0;
              }else if(drawing.displayedView >= 0){
                //commenting on a saved view
                rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
                rawdata.chanCommentSp[(int)rawdata.numChComments] = (uint8_t)drawing.displayedView;
                gtk_button_set_label(comment_ok_button,"Apply");
              }else if (drawing.displayedView == -2){
                //commenting on a view that hasn't been saved yet
                if(rawdata.numViews >= MAXNVIEWS){
                  printf("Cannot add any more views.\n");
                  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                  GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Adding a comment would require the current view to be saved, however the maximum number of custom views has been reached.  Older custom views must be deleted before more can be added.");
                  gtk_dialog_run(GTK_DIALOG(message_dialog));
                  gtk_widget_destroy(message_dialog);
                  return;
                }
                rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
                rawdata.chanCommentSp[(int)rawdata.numChComments] = rawdata.numViews;
                gtk_button_set_label(comment_ok_button,"Save View and Apply");
              }else{
                printf("WARNING: undefined view state, not displaying edit window.\n");
              }
              guiglobals.commentEditMode=0;
              gtk_window_set_title(comment_window,"Add Comment");
              gtk_widget_set_visible(GTK_WIDGET(remove_comment_button),FALSE);
              gtk_entry_set_text(comment_entry, "");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }
            //re-enable comment display
            guiglobals.drawSpComments=1;
            //show the window to edit the comment
            gtk_window_present(comment_window); 
            break;
          default:
            break;
        }
      }
    }
  }
}

//take action when a mouse button is released
void on_spectrum_unclick(){
  if(guiglobals.draggingSp == 1){
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }
  guiglobals.draggingSp = 0;
}

void on_spectrum_cursor_motion(GtkWidget *widget, GdkEventMotion *event){

  if(widget==NULL){
    printf("WARNING: on_spectrum_cursor_motion - no widget.\n");
    return;
  }

  if(!rawdata.openedSp){
    return;
  }

  //printf("Cursor pos: %f %f\n",event->x,event->y);
  if (event->state & GDK_BUTTON1_MASK){
    //left mouse button being pressed
    if(guiglobals.draggingSp == 0){
      //start drag
      guiglobals.draggingSp = 1;
      if(guiglobals.drawSpCursor == 1)
        guiglobals.drawSpCursor = 0; //hide vertical cursor while dragging
      guiglobals.dragstartul=drawing.upperLimit;
      guiglobals.dragstartll=drawing.lowerLimit;
      guiglobals.dragStartX = (float)event->x;
      drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
      drawing.zoomFocusFrac = 0.5;
      //printf("Drag started! dragstartll=%i, dragstartul=%i\n",guiglobals.dragstartll,guiglobals.dragstartul);
    }else{
      //continue drag
      //printf("Drag updated!\n");
      GdkRectangle dasize;  // GtkDrawingArea size
      GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
      // Determine GtkDrawingArea dimensions
      gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
      drawing.xChanFocus = (int)((guiglobals.dragstartul + guiglobals.dragstartll)/2. + ((guiglobals.dragStartX - event->x)/(dasize.width-XORIGIN))*(guiglobals.dragstartul - guiglobals.dragstartll));
      drawing.zoomFocusFrac = 0.5;
      //printf("startx = %f, x = %f, drawing.lowerLimit = %i, drawing.upperLimit = %i, width = %i, focus = %i\n",guiglobals.dragStartX,event->x,drawing.lowerLimit,drawing.upperLimit,dasize.width,drawing.xChanFocus);
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }else{
    //no button press
    if(guiglobals.draggingSp == 1){
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
    guiglobals.draggingSp = 0;
  }

  int32_t cursorChan = (int)getCursorChannel(event->x, event->y);
  int32_t cursorChanRounded = cursorChan - (cursorChan % drawing.contractFactor); //channel at the start of the bin (if rebinned)
  //printf("cursorChan: %i\n",cursorChan);

  if(cursorChan >= 0){

    signed char commentToHighlight = (signed char)getCommentAtCursor(event->x, event->y);
    if(commentToHighlight != drawing.highlightedComment){
      //highlight the comment
      drawing.highlightedComment = commentToHighlight;
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
    
    signed char peakToHighlight = -1;
    if(guiglobals.fittingSp == FITSTATE_FITCOMPLETE){
      //check if the cursor is over a peak
      uint8_t i;
      for(i=0;i<fitpar.numFitPeaks;i++){
        if(cursorChan > (fitpar.fitParVal[FITPAR_POS1+(3*i)] - 2.*fitpar.fitParVal[FITPAR_WIDTH1+(3*i)])){
          if(cursorChan < (fitpar.fitParVal[FITPAR_POS1+(3*i)] + 2.*fitpar.fitParVal[FITPAR_WIDTH1+(3*i)])){
            if(peakToHighlight == -1){
              peakToHighlight = (signed char)i;
            }else{
              //highlight whichever peak is closer to the cursor
              if(fabsl(cursorChan - fitpar.fitParVal[FITPAR_POS1+(3*i)]) < fabsl(cursorChan - fitpar.fitParVal[FITPAR_POS1+(3*drawing.highlightedPeak)])){
                peakToHighlight = (signed char)i;
              }
            }
          }
        }
      }
    }

    //set the peak to highlight in the drawing routine
    if(drawing.highlightedPeak != peakToHighlight){
      //highlight the peak
      drawing.highlightedPeak = peakToHighlight;
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }

    //print info on the status bar
    if(drawing.highlightedComment>=0){
      //show the comment in the status bar
      gtk_label_set_text(bottom_info_text,rawdata.chanComment[drawing.highlightedComment]);
    }else{
      //show the default status bar info
      char statusBarLabel[256];
      char *statusBarLabelp = statusBarLabel;
      char binValStr[50];
      //char *binValStrp = binValStr;
      float binVal, errVal;
      int32_t i;
      switch(drawing.highlightedPeak){
        case -1:
          //print cursor position on status bar
          switch(drawing.multiplotMode){
            case MULTIPLOT_STACKED:
            case MULTIPLOT_OVERLAY_INDEPENDENT:
            case MULTIPLOT_OVERLAY_COMMON:
              //multiple visible plots
              if(calpar.calMode == 1){
                int32_t cursorChanEnd = cursorChanRounded + drawing.contractFactor;
                float cal_lowerChanLimit = (float)(getCalVal(cursorChanRounded));
                float cal_upperChanLimit = (float)(getCalVal(cursorChanEnd));
                statusBarLabelp += snprintf(statusBarLabelp,50,"%s: %0.1f - %0.1f, Values:", calpar.calUnit, cal_lowerChanLimit, cal_upperChanLimit);
              }else{
                if(drawing.contractFactor <= 1){
                  statusBarLabelp += snprintf(statusBarLabel,50,"Channel: %i, Values:",cursorChanRounded);
                }else{
                  statusBarLabelp += snprintf(statusBarLabel,256,"Channels: %i - %i, Values:",cursorChanRounded, cursorChanRounded + drawing.contractFactor - 1);
                }
              }
              for(i=0;i<(drawing.numMultiplotSp-1);i++){
                binVal = getDispSpBinVal(i,cursorChanRounded-drawing.lowerLimit);
                errVal = getDispSpBinErr(i,cursorChanRounded-drawing.lowerLimit);
                getFormattedValAndUncertainty(binVal,errVal,binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
                statusBarLabelp += snprintf(statusBarLabelp,17," %s,", binValStr);
              }
              binVal = getDispSpBinVal(drawing.numMultiplotSp-1,cursorChanRounded-drawing.lowerLimit);
              errVal = getDispSpBinErr(drawing.numMultiplotSp-1,cursorChanRounded-drawing.lowerLimit);
              getFormattedValAndUncertainty(binVal,errVal,binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
              binValStr[15] = '\0'; //truncate string (staying safe with sprintf, working around compiler warning when using snprintf instead)
              statusBarLabelp += sprintf(statusBarLabelp," %s", binValStr);
              break;
            case MULTIPLOT_SUMMED:
            case MULTIPLOT_NONE:
            default:
              //single plot
              binVal = getDispSpBinVal(0,cursorChanRounded-drawing.lowerLimit);
              errVal = getDispSpBinErr(0,cursorChanRounded-drawing.lowerLimit);
              getFormattedValAndUncertainty(binVal,errVal,binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
              if(calpar.calMode == 1){
                int32_t cursorChanEnd = cursorChanRounded + drawing.contractFactor;
                float cal_lowerChanLimit = (float)(getCalVal(cursorChanRounded));
                float cal_upperChanLimit = (float)(getCalVal(cursorChanEnd));
                snprintf(statusBarLabel,256,"%s: %0.1f to %0.1f, Value: %s", calpar.calUnit,cal_lowerChanLimit,cal_upperChanLimit,binValStr);
              }else{
                if(drawing.contractFactor <= 1){
                  snprintf(statusBarLabel,256,"Channel: %i, Value: %s",cursorChanRounded,binValStr);
                }else{
                  snprintf(statusBarLabel,256,"Channels: %i to %i, Value: %s",cursorChanRounded,cursorChanRounded + drawing.contractFactor - 1,binValStr);
                }
              }
              break;
          }
          break;
        default:
          //print highlighted peak info
          if(calpar.calMode == 1){
            float calCentr = (float)(getCalVal((double)(fitpar.fitParVal[FITPAR_POS1+(3*drawing.highlightedPeak)])));
            float calWidth = (float)(getCalWidth((double)(fitpar.fitParVal[FITPAR_WIDTH1+(3*drawing.highlightedPeak)])));
            float calCentrErr = (float)(getCalWidth((double)(fitpar.fitParErr[FITPAR_POS1+(3*drawing.highlightedPeak)])));
            float calWidthErr = (float)(getCalWidth((double)(fitpar.fitParErr[FITPAR_WIDTH1+(3*drawing.highlightedPeak)])));
            char fitParStr[3][50];
            getFormattedValAndUncertainty((double)fitpar.areaVal[drawing.highlightedPeak],(double)fitpar.areaErr[drawing.highlightedPeak],fitParStr[0],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(calCentr,calCentrErr,fitParStr[1],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(2.35482*calWidth,2.35482*calWidthErr,fitParStr[2],50,1,guiglobals.roundErrors);
            snprintf(statusBarLabel,256,"Area: %s, Centroid: %s, FWHM: %s",fitParStr[0],fitParStr[1],fitParStr[2]);
          }else{
            char fitParStr[3][50];
            getFormattedValAndUncertainty((double)fitpar.areaVal[drawing.highlightedPeak],(double)fitpar.areaErr[drawing.highlightedPeak],fitParStr[0],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty((double)(fitpar.fitParVal[FITPAR_POS1+(3*drawing.highlightedPeak)]),(double)(fitpar.fitParErr[FITPAR_POS1+(3*drawing.highlightedPeak)]),fitParStr[1],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(2.35482*(double)(fitpar.fitParVal[FITPAR_WIDTH1+(3*drawing.highlightedPeak)]),2.35482*(double)(fitpar.fitParErr[FITPAR_WIDTH1+(3*drawing.highlightedPeak)]),fitParStr[2],50,1,guiglobals.roundErrors);
            snprintf(statusBarLabel,256,"Area: %s, Centroid: %s, FWHM: %s",fitParStr[0],fitParStr[1],fitParStr[2]);
          }

          break;
      }
      gtk_label_set_text(bottom_info_text,statusBarLabel);
    }

    //draw cursor on plot (expensive, requires redraw of plot itself)
    if((guiglobals.draggingSp == 0)&&(guiglobals.drawSpCursor != -1)){
      //don't redraw if the cursor hasn't moved, that would be st00pid
      if(fabs(guiglobals.cursorPosX - event->x) >= 1.0){
        guiglobals.cursorPosX = (float)(event->x);
        guiglobals.drawSpCursor = 1; //draw vertical cursor
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
      }
    }

  }else{
    gtk_label_set_text(bottom_info_text,"Drag spectrum to pan, mouse wheel to zoom.");
    if(guiglobals.drawSpCursor == 1){
      guiglobals.drawSpCursor = 0; //hide vertical cursor
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }

  return;
}

//get the bin position in the histogram plot
float getXPos(const int32_t bin, const float width){
  int32_t binc=bin;
  if(bin < 0)
    binc=0;
  else if(bin > (drawing.upperLimit - drawing.lowerLimit))
    binc=drawing.upperLimit - drawing.lowerLimit;
  //printf("bin: %i\n",bin);
  return (float)(XORIGIN + ((double)binc*(width-XORIGIN)/((double)(drawing.upperLimit-drawing.lowerLimit))));
}

//get the screen position of a channel (or fractional channel)
//returns -1 if offscreen
//if halfBinOffset=1, will offset by half a bin (for drawing fits)
float getXPosFromCh(const float chVal, const float width, const uint8_t halfBinOffset){
  if((chVal < drawing.lowerLimit)||(chVal > drawing.upperLimit)){
    return -1;
  }
  float bin = chVal - (float)(drawing.lowerLimit);
  if(halfBinOffset)
    bin += ((float)drawing.contractFactor/2.0f);
  return (float)(XORIGIN + ((double)bin*(width-XORIGIN)/((double)(drawing.upperLimit-drawing.lowerLimit))));
}

//get the y-coordinate for drawing a specific bin value
float getYPos(const float val, const int32_t multiplotSpNum, const float height){
  double pos, minVal;
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked
      minVal = YORIGIN + (height-YORIGIN)*(double)(multiplotSpNum/(drawing.numMultiplotSp*1.0));
      if(drawing.logScale){
        if((val > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            pos = YORIGIN + (height-YORIGIN)*(double)((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*(log10f(val - drawing.scaleLevelMin[multiplotSpNum])/log10f(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])));
          }else{
            pos = YORIGIN + (height-YORIGIN)*(double)((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*(log10f(val)/log10f(drawing.scaleLevelMax[multiplotSpNum])));
          }
        }else{
          pos = minVal;
        }
      }else{
        pos = YORIGIN + (height-YORIGIN)*(double)((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])));
      }
      //clip value to bottom edge of plot
      if((pos < minVal)||(pos!=pos))
        pos = minVal;
      break;
    case MULTIPLOT_OVERLAY_INDEPENDENT:
    case MULTIPLOT_OVERLAY_COMMON:
    case MULTIPLOT_SUMMED:
    case MULTIPLOT_NONE:
    default:
      //single plot
      if(drawing.logScale){
        if((val > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            pos = YORIGIN + (height-YORIGIN)*(log10(val - drawing.scaleLevelMin[multiplotSpNum])/log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]));
          }else{
            pos = YORIGIN + (height-YORIGIN)*(log10(val)/log10(drawing.scaleLevelMax[multiplotSpNum]));
          } 
        }else{
          return (float)YORIGIN;
        }
      }else{
        pos = YORIGIN + (height-YORIGIN)*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]));
      }
      //clip value to bottom edge of plot
      if((pos < YORIGIN)||(pos!=pos))
        return (float)YORIGIN;
      break;
  }
  //printf("pos: %f\n",pos);
  return (float)pos;
}

//axis tick drawing
float getAxisXPos(const double axisVal, const float width){
  double cal_lowerLimit = (double)drawing.lowerLimit;
  double cal_upperLimit = (double)drawing.upperLimit;
  if(calpar.calMode==1){
    //calibrate
    cal_lowerLimit = getCalVal(drawing.lowerLimit);
    cal_upperLimit = getCalVal(drawing.upperLimit);
  }
  if(((cal_upperLimit>cal_lowerLimit)&&((axisVal < cal_lowerLimit)||(axisVal >= cal_upperLimit))) || ((cal_lowerLimit>cal_upperLimit)&&((axisVal > cal_lowerLimit)||(axisVal <= cal_upperLimit))))
    return (float)SMALL_NUMBER; //value is off the visible axis
  
  return (float)(XORIGIN + (width-XORIGIN)*(double)((double)axisVal - cal_lowerLimit)/(double)(cal_upperLimit - cal_lowerLimit));
}
void drawXAxisTick(const double axisVal, cairo_t *cr, const float width, const float height, const double baseFontSize, const uint8_t drawGridLines){
  float axisPos = getAxisXPos(axisVal,width);
  //printf("axis Val: %i, axisPos: %f\n",axisVal,axisPos);
  if(axisPos != (float)SMALL_NUMBER){
    //axis position is valid
    setTextColor(cr);
    cairo_move_to(cr, axisPos, -YORIGIN);
    cairo_line_to(cr, axisPos, -YORIGIN*0.875);
    if(drawGridLines){
      //don't draw a gridline that overlaps the y-axis
      if(axisPos > XORIGIN){
        cairo_stroke(cr);
        setGridLineColor(cr);
        float yPos = -YORIGIN;
        while(yPos > -height){
          cairo_move_to(cr, axisPos, yPos);
          yPos = yPos - 5.0f;
          cairo_line_to(cr, axisPos, yPos);
          yPos = yPos - 5.0f;
        }
        cairo_stroke(cr);
        setTextColor(cr);
      }
    }
    char tickLabel[20];
    sprintf(tickLabel,"%.0f",axisVal); //set string for label
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, axisPos - extents.width/2., -YORIGIN*0.5);
    cairo_show_text(cr, tickLabel);
  }
}
float getAxisYPos(const float axisVal, const int32_t multiplotSpNum, const float height){
  double posVal;
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked
      if(drawing.logScale){
        if((axisVal > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            posVal = (1.0/drawing.numMultiplotSp)*(YORIGIN-height)*log10(axisVal - drawing.scaleLevelMin[multiplotSpNum])/log10f(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) + (YORIGIN-height)*(float)(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - YORIGIN;
          }else{
            posVal = (1.0/drawing.numMultiplotSp)*(YORIGIN-height)*log10(axisVal)/log10f(drawing.scaleLevelMax[multiplotSpNum]) + (YORIGIN-height)*(float)(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - YORIGIN;
          }
        }else{
          posVal = (YORIGIN-height)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - YORIGIN;
        }
      }else{
        posVal = (float)(1.0/drawing.numMultiplotSp)*(YORIGIN-height)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) + (YORIGIN-height)*(float)(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - YORIGIN;
      }
      break;
    case MULTIPLOT_OVERLAY_INDEPENDENT:
    case MULTIPLOT_OVERLAY_COMMON:
    case MULTIPLOT_SUMMED:
    case MULTIPLOT_NONE:
    default:
      if(drawing.logScale){
        if((axisVal > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            posVal = (YORIGIN-height)*log10f(axisVal - drawing.scaleLevelMin[multiplotSpNum])/log10f(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) - YORIGIN;
          }else{
            posVal = (YORIGIN-height)*log10f(axisVal)/log10f(drawing.scaleLevelMax[multiplotSpNum]) - YORIGIN;
          }    
        }else{
          posVal = -YORIGIN;
        }
      }else{
        posVal = (YORIGIN-height)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) - YORIGIN;
      }
      break;
  }

  //printf("height: %f, multiplotsp: %i, axisval: %f, posval: %f\n",height,multiplotSpNum,axisVal,posVal);
  return (float)posVal;
}
void drawYAxisTick(const double axisVal, const int32_t multiplotSpNum, cairo_t *cr, const float width, const float height, const double baseFontSize, const uint8_t drawGridLines){
  if((axisVal < drawing.scaleLevelMin[multiplotSpNum])||(axisVal >= drawing.scaleLevelMax[multiplotSpNum])){
    //printf("axisval:%f,scalemin:%f,scalemax:%f\n",axisVal,drawing.scaleLevelMin[multiplotSpNum],drawing.scaleLevelMax[multiplotSpNum]);
    return; //invalid axis value,
  }
  if(drawing.logScale == 0){
    if((axisVal!=0.0)&&(fabs(axisVal) < (drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/20.0)){
      //tick is too close to zero, don't draw
      return;
    }
    if((drawing.multiplotMode == MULTIPLOT_STACKED)&&(fabs(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < ((drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/8.0)) ){
      //tick is too close to the top of the spectrum in a stacked view, don't draw
      return;
    }
  }else{
    if(axisVal <= 0.){
      return; //invalid axis value in log scale
    }
    if(drawing.scaleLevelMin[multiplotSpNum] > 0.){
      if((log10(axisVal) < log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/20.0)){
        //tick is too close to zero, don't draw
        return;
      }
      if((drawing.multiplotMode == MULTIPLOT_STACKED)&&(log10(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < (log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/8.0)) ){
        //tick is too close to the top of the spectrum in a stacked view, don't draw
        return;
      }
    }else{
      if((log10(axisVal) < log10(drawing.scaleLevelMax[multiplotSpNum])/20.0)){
        //tick is too close to zero, don't draw
        return;
      }
      if((drawing.multiplotMode == MULTIPLOT_STACKED)&&(log10(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < (log10(drawing.scaleLevelMax[multiplotSpNum])/8.0)) ){
        //tick is too close to the top of the spectrum in a stacked view, don't draw
        return;
      }
    }
    
  }
  
  float axisPos = getAxisYPos((float)axisVal,multiplotSpNum,height);
  if((axisPos <= 0) && (axisPos > (height)*-0.98)) {
    //axis position is valid (ie. on the plot, and not too close to the top of the plot so that it won't be cut off)
    if(drawing.multiplotMode!=MULTIPLOT_OVERLAY_INDEPENDENT){
      //default colours used, unless in independent scaling mode
      setTextColor(cr);
    }
    cairo_move_to(cr, XORIGIN*1.06f, axisPos);
    cairo_line_to(cr, XORIGIN*0.94f, axisPos);
    if(drawGridLines){
      //don't draw a gridline which overlaps with the x-axis
      if(axisVal > drawing.scaleLevelMin[multiplotSpNum]){
        cairo_stroke(cr);
        if(drawing.multiplotMode!=MULTIPLOT_OVERLAY_INDEPENDENT){
          //default colours used, unless in independent scaling mode
          setGridLineColor(cr);
        }
        float xPos = (float)(XORIGIN*1.06 + 5.0);
        while(xPos < width){
          cairo_move_to(cr, xPos, axisPos);
          xPos = xPos + 5.0f;
          cairo_line_to(cr, xPos, axisPos);
          xPos = xPos + 5.0f;
        }
        cairo_stroke(cr);
        if(drawing.multiplotMode!=MULTIPLOT_OVERLAY_INDEPENDENT){
          //default colours used, unless in independent scaling mode
          setTextColor(cr);
        }
      }
    }
    char tickLabel[20];
    getFormattedYAxisVal(axisVal, drawing.scaleLevelMin[multiplotSpNum], drawing.scaleLevelMax[multiplotSpNum], tickLabel, 20);
    
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, XORIGIN*0.875 - extents.width, axisPos + extents.height/2.);
    cairo_show_text(cr, tickLabel);
  }
}

void drawPlotLabel(cairo_t *cr, const float width, const float height, const double baseFontSize){
  char plotLabel[256];
  cairo_text_extents_t extents; //get dimensions needed to justify text labels
  float labelYOffset;
  cairo_set_font_size(cr, baseFontSize);
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked spectra
      labelYOffset = height/(float)(3.*drawing.numMultiplotSp);
      if(labelYOffset > YORIGIN){
        labelYOffset = YORIGIN;
      }
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width, (height-YORIGIN)*((drawing.numMultiplotSp-i-1)/(drawing.numMultiplotSp*1.0)) + labelYOffset);
        cairo_show_text(cr, plotLabel);
      }
      break;
    case MULTIPLOT_OVERLAY_INDEPENDENT:
    case MULTIPLOT_OVERLAY_COMMON:
      //overlaid spectra
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width, YORIGIN*(1.0 + 0.45*i));
        cairo_show_text(cr, plotLabel);
      }
      break;
    case MULTIPLOT_SUMMED:
      //summed spectra
      setTextColor(cr);
      strcpy(plotLabel, "Sum of:");
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (width)*0.95 - extents.width, YORIGIN);
      cairo_show_text(cr, plotLabel);
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width,  YORIGIN*(1.0 + 0.45*(i+1)));
        cairo_show_text(cr, plotLabel);
      }
      break;
    case MULTIPLOT_NONE:
      //single plot mode
      setTextColor(cr);
      if(drawing.scaleFactor[drawing.multiPlots[0]] == 1.0){
        strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[0]]);
      }else{
        snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[0]],drawing.scaleFactor[drawing.multiPlots[0]]);
      }
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (width)*0.95 - extents.width, YORIGIN);
      cairo_show_text(cr, plotLabel);
      break;
    default:
      break;
  }
}

float getDistBetweenYAxisTicks(const float axisRange, const int32_t numTicks){

  if((axisRange < 1E-10)||(numTicks == 0)){
    return 1.0f;
  }

  float trialDist = 0.0f;
  float sfFactor = 0.0f;
  int32_t sigf = 0;
  if(axisRange < 1){
    //get number of sig figs
    float axisRangec = axisRange;
    while(axisRangec < 1){
      axisRangec *= 10.0f;
      sigf--;
    }
  }else{
    //get number of sig figs
    float axisRangec = 1;
    while(axisRangec < axisRange){
      axisRangec *= 10.0f;
      sigf++;
    }
  }

  trialDist = axisRange/(float)numTicks;
  if(trialDist < 0.0f){
    trialDist *= -1.0f;
  }

  sfFactor = powf(10.0f,(float)(sigf-1));
  if(trialDist > sfFactor){
    return sfFactor*(float)((int)(trialDist/sfFactor));
  }else if(trialDist > sfFactor/2.0f){
    return sfFactor/2.0f;
  }
  while(trialDist < sfFactor){
    sfFactor /= 10.0f;
  }
  return sfFactor*(float)((int)(trialDist/sfFactor));
  

}

//provides the distance (in x axis units) between ticks on the x axis
//axisRange: range of the x-axis being displayed (in x-axis units)
//width: width of the displayed plot (in pixels)
double getDistBetweenXAxisTicks(const double axisRange, const float width){

  double tickDist = axisRange*200.0/width; //ticks every 200px or so
  //printf("axisRange: %f, width: %f, tickDist: %f\n",axisRange,width,tickDist);

  if(tickDist > 5000.0){
    tickDist = tickDist - fmod(tickDist,5000.0);
  }else if(tickDist > 1000.0){
    tickDist = tickDist - fmod(tickDist,1000.0);
  }else if(tickDist > 100.0){
    tickDist = tickDist - fmod(tickDist,100.0);
  }else if(tickDist > 10.0){
    tickDist = tickDist - fmod(tickDist,10.0);
  }else{
    tickDist = tickDist - fmod(tickDist,1.0);
  }

  if(tickDist > 0.0){
    return tickDist;
  }else{
    return 1.0; //a non-zero return value is assumed
  }
  

}

//get the x range of the plot in terms of x axis units, 
//taking into account whether or not a calibration is in use
double getPlotRangeXUnits(){
  
  if(calpar.calMode==1){
    //calibrate
    double cal_lowerLimit = getCalVal(drawing.lowerLimit);
    double cal_upperLimit = getCalVal(drawing.upperLimit);
    //printf("cal upperlimit: %f, cal lowerlim: %f\n",cal_upperLimit,cal_lowerLimit);
    return cal_upperLimit - cal_lowerLimit;
  }
  return (double)(drawing.upperLimit - drawing.lowerLimit);
  
}


//draw a spectrum
//drawLabels: 0=don't draw, 1=draw
//showFit: 0=don't show, 1=show without highlighted peaks, 2=show with highlighted peaks
//drawComments: 0=don't draw, 1=draw
//drawFast: 0=don't interpolate, 1=interpolate (faster drawing, less accurate)
void drawSpectrum(cairo_t *cr, const float width, const float height, const float scaleFactor, const uint8_t drawLabels, const uint8_t drawGridLines, const uint8_t showFit, const uint8_t drawComments, const uint8_t drawFast){

  if(!rawdata.openedSp){
    return;
  }

  if(drawing.multiPlots[0] >= NSPECT){
    printf("Spectrum number too high (%i)!\n", drawing.multiPlots[0]);
    return;
  }

  //set the origin of the coordinate system in pixels
  
  // Draw the background colour
  //cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  //cairo_paint(cr);

  //printf("width: %f, height: %f\n",width,height);
  cairo_set_line_width(cr, 2.0*scaleFactor);

  double plotFontSize = 13.5*scaleFactor;

  // transform the coordinate system
  cairo_translate(cr, 0.0, height); //so that the origin is at the lower left

  setPlotLimits(); //setup the x range to plot over

  //get the maximum/minimum y values of the displayed region
  float maxVal[MAX_DISP_SP];
  float minVal[MAX_DISP_SP];
  float currentVal[MAX_DISP_SP];
  for(int32_t i=0;i<drawing.numMultiplotSp;i++){
    minVal[i] = (float)(BIG_NUMBER);
    maxVal[i] = (float)(SMALL_NUMBER);
  }
  for(int32_t i=0;i<(drawing.upperLimit-drawing.lowerLimit-1);i+=drawing.contractFactor){
    switch(drawing.multiplotMode){
      case MULTIPLOT_STACKED:
        //stacked
      case MULTIPLOT_OVERLAY_INDEPENDENT:
        //overlay (independent scaling)
        for(int32_t j=0;j<drawing.numMultiplotSp;j++){
          currentVal[j] = getDispSpBinVal(j, i);
          if(currentVal[j] > maxVal[j]){
            maxVal[j] = currentVal[j];
          }
          if(currentVal[j] < minVal[j]){
            minVal[j] = currentVal[j];
          }
        }
        break;
      case MULTIPLOT_OVERLAY_COMMON:
        //overlay (common scaling)
        for(int32_t j=0;j<drawing.numMultiplotSp;j++){
          currentVal[0] = getDispSpBinVal(j, i);
          if(currentVal[0] > maxVal[0]){
            maxVal[0] = currentVal[0];
          }
          if(currentVal[0] < minVal[0]){
            minVal[0] = currentVal[0];
          }
        }
        break;
      case MULTIPLOT_SUMMED:
        //summed
      case MULTIPLOT_NONE:
        currentVal[0] = getDispSpBinVal(0, i);
        if(currentVal[0] > maxVal[0]){
          maxVal[0] = currentVal[0];
        }
        if(currentVal[0] < minVal[0]){
          minVal[0] = currentVal[0];
        }
        break;
      default:
        break;
    }
  }
  //setup autoscaling
  if((drawing.autoScale)||(drawing.scaleLevelMax[0] <= drawing.scaleLevelMin[0])){
    switch(drawing.multiplotMode){
      case MULTIPLOT_STACKED:
      case MULTIPLOT_OVERLAY_INDEPENDENT:
        for(int32_t i=0;i<drawing.numMultiplotSp;i++){
          if(drawing.logScale){
            drawing.scaleToLevelMax[i] = maxVal[i]*2.0f;
          }else{
            drawing.scaleToLevelMax[i] = maxVal[i]*1.2f;
          }
          drawing.scaleToLevelMin[i] = minVal[i];
        }
        break;
      case MULTIPLOT_OVERLAY_COMMON:
        for(int32_t i=0;i<drawing.numMultiplotSp;i++){
          if(drawing.logScale){
            drawing.scaleToLevelMax[i] = maxVal[0]*2.0f;
          }else{
            drawing.scaleToLevelMax[i] = maxVal[0]*1.2f;
          }
          drawing.scaleToLevelMin[i] = minVal[0];
        }
        break;
      case MULTIPLOT_SUMMED:
      case MULTIPLOT_NONE:
        if(drawing.logScale){
          drawing.scaleToLevelMax[0] = maxVal[0]*2.0f;
        }else{
          drawing.scaleToLevelMax[0] = maxVal[0]*1.2f;
        }
        drawing.scaleToLevelMin[0] = minVal[0];
        break;
      default:
        break;
    }
    if(guiglobals.useZoomAnimations){
      if((drawing.zoomYLastFrameTime-drawing.zoomYStartFrameTime) > 3000000){
        //reset zoom
        for(int32_t i=0;i<drawing.numMultiplotSp;i++){
          drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
          drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
        }
      }
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        if(drawing.scaleLevelMax[i]==drawing.scaleLevelMin[i]){
          drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
          drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
        }
      }
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        if((drawing.scaleLevelMax[i] != drawing.scaleToLevelMax[i])||(drawing.scaleLevelMin[i] != drawing.scaleToLevelMin[i])){
          if(drawing.zoomingSpY == 0){
            //printf("Starting y zoom.\n");
            drawing.zoomingSpY = 1;
            drawing.zoomYStartFrameTime = gdk_frame_clock_get_frame_time(frameClock);
            drawing.zoomYLastFrameTime = drawing.zoomYStartFrameTime;
            gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_y_callback, NULL, NULL);
            break;
          }
        }
      }

    }else{
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }
  }
  /*printf("mode: %i   ",drawing.multiplotMode);
  for(i=0;i<drawing.numMultiplotSp;i++){
    printf("i = %i, maxVal = %f, minVal = %f  ",i,maxVal[i],minVal[i]);
    printf("scaleToMax = %f, scaleToMin = %f  ",drawing.scaleToLevelMax[i],drawing.scaleToLevelMin[i]);
    printf("scaleMax = %f, scaleMin = %f  ",drawing.scaleLevelMax[i],drawing.scaleLevelMin[i]);
  }
  printf("\n");*/

  //draw x axis ticks and gridlines
  double tickDist = getDistBetweenXAxisTicks(getPlotRangeXUnits(),width);
  double maxXVal = getCalVal(S32K*1.0);
  double di;
  for(di=0;fabs(di)<maxXVal;di+=tickDist){
    drawXAxisTick(di, cr, width, height, plotFontSize, drawGridLines);
  }
  for(di=0;fabs(di)<maxXVal;di-=tickDist){
    drawXAxisTick(di, cr, width, height, plotFontSize, drawGridLines);
  }
  cairo_stroke(cr);

  //draw y axis ticks and gridlines
  int32_t numTickPerSp;
  float yTickDist, yTick;
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked
      if(drawing.logScale){
        for(int32_t i=0;i<drawing.numMultiplotSp;i++){
          float rangeVal = drawing.scaleLevelMax[i] - drawing.scaleLevelMin[i];
          if(rangeVal > drawing.scaleLevelMax[i])
            rangeVal = drawing.scaleLevelMax[i];
          int32_t numTickUsed = 0;
          if(rangeVal >= 1000.){
            //logarithmic scale ticks in base-10
            float tickVal = powf(10.0f,(float)(getNSigf(drawing.scaleLevelMax[i],10.0)));
            while((tickVal > drawing.scaleLevelMin[i])&&(tickVal >= 1.0f)){
              drawYAxisTick(tickVal, i, cr, width, height, plotFontSize, drawGridLines);
              tickVal /= 10.0f;
              numTickUsed++;
            }
          }else{
            //logarithmic scale ticks in base-2
            float tickVal = powf(2.0f,(float)(getNSigf(drawing.scaleLevelMax[i],2.0)));
            while((tickVal > drawing.scaleLevelMin[i])&&(tickVal >= 1.0f)){
              drawYAxisTick(tickVal, i, cr, width, height, plotFontSize, drawGridLines);
              tickVal /= 2.0f;
              numTickUsed++;
            }
          }
        }
      }else{
        numTickPerSp = (int)((height)/(YORIGIN*(float)(drawing.numMultiplotSp)));
        if(numTickPerSp < 2)
          numTickPerSp = 2;
        for(int32_t i=0;i<drawing.numMultiplotSp;i++){
          yTickDist = getDistBetweenYAxisTicks(drawing.scaleLevelMax[i] - drawing.scaleLevelMin[i],numTickPerSp);
          cairo_set_source_rgb(cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
          for(yTick=0.;yTick<drawing.scaleLevelMax[i];yTick+=yTickDist){
            drawYAxisTick(yTick, i, cr, width, height, plotFontSize, drawGridLines);
          }
          for(yTick=0.;yTick>drawing.scaleLevelMin[i];yTick-=yTickDist){
            if(yTick != 0)
              drawYAxisTick(yTick, i, cr, width, height, plotFontSize, drawGridLines);
          }
          //drawYAxisTick(0.0, i, cr, width, height, plotFontSize, drawGridLines); //always draw the zero label on the y axis
          cairo_stroke(cr);
          //draw the zero line if applicable
          if((drawing.scaleLevelMin[i] < 0.0) && (drawing.scaleLevelMax[i] > 0.0)){
            cairo_set_line_width(cr, 1.0*scaleFactor);
            cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
            cairo_move_to(cr, XORIGIN, getAxisYPos(0.0,i,height));
            cairo_line_to(cr, width, getAxisYPos(0.0,i,height));
            cairo_stroke(cr);
          }
          cairo_set_line_width(cr, 2.0*scaleFactor);
        }
      }
      break;
    case MULTIPLOT_OVERLAY_INDEPENDENT:
      //overlay (independent scaling)
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        double labelOffset = 0.4*((double)i+1.0)/((double)(drawing.numMultiplotSp)*1.0);
        cairo_set_source_rgb(cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        drawYAxisTick((double)drawing.scaleLevelMax[i]*(0.3 + labelOffset), i, cr, width, height, plotFontSize, drawGridLines); //draw one axis tick near the middle of the axis, per spectrum
        drawYAxisTick(0.0, i, cr, width, height, plotFontSize, drawGridLines); //always draw the zero label on the y axis
      }
      break;
    case MULTIPLOT_OVERLAY_COMMON:
    case MULTIPLOT_SUMMED:
    case MULTIPLOT_NONE:
      //modes with a single scale
      setTextColor(cr);
      if(drawing.logScale){
        //numTickPerSp *= 2;
        int32_t nsigf10 = 0;
        if(drawing.scaleLevelMin[0] <= 0)
          nsigf10 = getNSigf(drawing.scaleLevelMax[0],10.0);
        else
          nsigf10 = getNSigf(drawing.scaleLevelMax[0],10.0) - getNSigf(drawing.scaleLevelMin[0],10.0);
        int32_t numTickUsed = 0;
        //printf("nsigf10: %i\n",nsigf10);
        if(nsigf10 >= 3){
          //logarithmic scale ticks in base-10
          float tickVal = powf(10.0f,(float)(getNSigf(drawing.scaleLevelMax[0],10.0)));
          while((tickVal > drawing.scaleLevelMin[0])&&(tickVal >= 1.0f)){
            drawYAxisTick(tickVal, 0, cr, width, height, plotFontSize, drawGridLines);
            tickVal /= 10.0f;
            numTickUsed++;
          }
        }else{
          //logarithmic scale ticks in base-2
          float tickVal = powf(2.0f,(float)(getNSigf(drawing.scaleLevelMax[0],2.0)));
          while((tickVal > drawing.scaleLevelMin[0])&&(tickVal >= 1.0f)){
            drawYAxisTick(tickVal, 0, cr, width, height, plotFontSize, drawGridLines);
            tickVal /= 2.0f;
            numTickUsed++;
          }
        }
      }else{
        numTickPerSp = (int)(height/(4.0f*YORIGIN)) + 1;
        yTickDist = getDistBetweenYAxisTicks(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0],numTickPerSp);
        for(yTick=0.;yTick<drawing.scaleLevelMax[0];yTick+=yTickDist){
          drawYAxisTick(yTick, 0, cr, width, height, plotFontSize, drawGridLines);
        }
        for(yTick=0.;yTick>drawing.scaleLevelMin[0];yTick-=yTickDist){
          if(yTick != 0)
            drawYAxisTick(yTick, 0, cr, width, height, plotFontSize, drawGridLines);
        }
        //printf("min: %f, max: %f, yTickDist: %f, numTickPerSp: %i\n",drawing.scaleLevelMin[0],drawing.scaleLevelMax[0],yTickDist,numTickPerSp);
        //drawYAxisTick(0.0, 0, cr, width, height, plotFontSize, drawGridLines); //always draw the zero label on the y axis
        cairo_stroke(cr);
        //draw the zero line if applicable
        if((drawing.scaleLevelMin[0] < 0.0) && (drawing.scaleLevelMax[0] > 0.0)){
          cairo_set_line_width(cr, 1.0*scaleFactor);
          cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
          cairo_move_to(cr, XORIGIN, getAxisYPos(0.0,0,height));
          cairo_line_to(cr, width, getAxisYPos(0.0,0,height));
          cairo_stroke(cr);
        }
        cairo_set_line_width(cr, 2.0*scaleFactor);
      }
      break;
    default:
      break;
  }

  //draw label(s) for the plot
  if(drawLabels){
    cairo_translate(cr, 0.0, -height);
    setTextColor(cr);
    drawPlotLabel(cr, width, height, plotFontSize); //draw plot label(s)
    cairo_stroke(cr);
    cairo_translate(cr, 0.0, height);
  }
  
  //interpolate (ie. limit the number of bins drawn in the next step), 
  //to help drawing performance
  float maxDrawBins;
  switch(drawFast){
    case 1:
      maxDrawBins = width*1.5f;
      break;
    case 0:
    default:
      maxDrawBins = S32K;
      break;
  }
   
  //printf("maximum bins to draw: %i\n",maxDrawBins);
  float binSkipFactorF = (float)(drawing.upperLimit-drawing.lowerLimit)/(maxDrawBins);
  if(drawing.zoomLevel > 1.05){
    gint64 zoomYTime = drawing.zoomYLastFrameTime-drawing.zoomYStartFrameTime;
    if((guiglobals.draggingSp)||(drawing.zoomingSpX)||(zoomYTime>0 && zoomYTime<400000)){
      //optimize when dragging/zooming
      binSkipFactorF *= 1.5f;
      if(drawing.multiplotMode>MULTIPLOT_SUMMED){
        binSkipFactorF *= (float)(drawing.numMultiplotSp);
      }
    }
  }
  int32_t binSkipFactor = (int)binSkipFactorF;
  if(binSkipFactor <= drawing.contractFactor){
    binSkipFactor = drawing.contractFactor;
  }
  if(binSkipFactor <= 0){
    binSkipFactor = 1;
  }
  //printf("default bins: %i, binskipfactor: %f, contractFactor: %i\n",drawing.upperLimit-drawing.lowerLimit,binSkipFactorF,drawing.contractFactor);
  //printf("dragging: %i, zooming x: %i, zooming y: %i\n",guiglobals.draggingSp,drawing.zoomingSpX,drawing.zoomingSpY);
  //for smooth scrolling of interpolated spectra, have the start bin always
  //be a multiple of the skip factor
  int32_t startBin = 0 - (drawing.lowerLimit % binSkipFactor);

  cairo_scale(cr, 1.0, -1.0); //invert y-axis so that positive y values go up

  //draw the actual histogram
  int32_t range = drawing.upperLimit-drawing.lowerLimit;
  float nextVal;
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked
    case MULTIPLOT_OVERLAY_INDEPENDENT:
      //overlay (independent scaling)
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        cairo_move_to(cr, getXPos(startBin,width), getYPos(getDispSpBinVal(i,startBin),i,height));
        for(int32_t j=startBin;j<range;j+=binSkipFactor){

          //draw high values even if they were going to be interpolated over
          if(binSkipFactor > drawing.contractFactor){
            for(int32_t k=0;k<binSkipFactor;k++){
              if(getDispSpBinVal(i, j+k) > drawing.scaleLevelMax[i]*0.8){
                currentVal[0] = getDispSpBinVal(i, j);
                nextVal = getDispSpBinVal(i, j+k);
                cairo_line_to(cr, getXPos(j+k,width), getYPos(currentVal[0],i,height));
                cairo_line_to(cr, getXPos(j+k,width), getYPos(nextVal,i,height));
                break;
              }
            }
          }

          currentVal[0] = getDispSpBinVal(i, j);
          nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_line_to(cr, getXPos(j+binSkipFactor,width), getYPos(currentVal[0],i,height));
          cairo_line_to(cr, getXPos(j+binSkipFactor,width), getYPos(nextVal,i,height));
        }
        //choose color
        cairo_set_source_rgb(cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case MULTIPLOT_OVERLAY_COMMON:
      //overlay (common scaling)
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        cairo_move_to(cr, getXPos(startBin,width), getYPos(getDispSpBinVal(i,startBin),0,height));
        for(int32_t j=startBin;j<range;j+=binSkipFactor){

          //draw high values even if they were going to be interpolated over
          if(binSkipFactor > drawing.contractFactor){
            for(int32_t k=0;k<binSkipFactor;k++){
              if(getDispSpBinVal(i, j+k) > drawing.scaleLevelMax[0]*0.8){
                currentVal[0] = getDispSpBinVal(i, j);
                nextVal = getDispSpBinVal(i, j+k);
                cairo_line_to(cr, getXPos(j+k,width), getYPos(currentVal[0],0,height));
                cairo_line_to(cr, getXPos(j+k,width), getYPos(nextVal,0,height));
                break;
              }
            }
          }

          currentVal[0] = getDispSpBinVal(i, j);
          nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_line_to(cr, getXPos(j+binSkipFactor,width), getYPos(currentVal[0],0,height));
          cairo_line_to(cr, getXPos(j+binSkipFactor,width), getYPos(nextVal,0,height));
        }
        //choose color
        cairo_set_source_rgb(cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case MULTIPLOT_SUMMED:
      //summed
    case MULTIPLOT_NONE:
      cairo_move_to(cr, getXPos(startBin,width), getYPos(getDispSpBinVal(0,startBin),0,height));
      for(int32_t i=startBin;i<range;i+=binSkipFactor){

        //draw high values even if they were going to be interpolated over
        if(binSkipFactor > drawing.contractFactor){
          for(int32_t k=0;k<binSkipFactor;k++){
            if(getDispSpBinVal(0, i+k) > drawing.scaleLevelMax[0]*0.8){
              currentVal[0] = getDispSpBinVal(0, i);
              nextVal = getDispSpBinVal(0, i+k);
              cairo_line_to(cr, getXPos(i+k,width), getYPos(currentVal[0],0,height));
              cairo_line_to(cr, getXPos(i+k,width), getYPos(nextVal,0,height));
              break;
            }
          }
        }

        currentVal[0] = getDispSpBinVal(0, i);
        nextVal = getDispSpBinVal(0, i+binSkipFactor);
        //printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,width), rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],0,width);
        cairo_line_to(cr, getXPos(i+binSkipFactor,width), getYPos(currentVal[0],0,height));
        cairo_line_to(cr, getXPos(i+binSkipFactor,width), getYPos(nextVal,0,height));
      }
      cairo_set_source_rgb(cr, drawing.spColors[0], drawing.spColors[1], drawing.spColors[2]);
      cairo_stroke(cr);
      break;
    default:
      break;
  }

  //draw fit
  if((guiglobals.fittingSp == FITSTATE_FITCOMPLETE)&&(showFit>0)){
    if(fitpar.fitType != FITTYPE_SUMREGION){
      
      if((drawing.lowerLimit < fitpar.fitEndCh)&&(drawing.upperLimit > fitpar.fitStartCh)){
        cairo_set_line_width(cr, 3.0*scaleFactor);
        //cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.8);
        float fitDrawX, nextFitDrawX, nextXpos;
        float fitSkipFactor = 0.5f*(float)(binSkipFactor);
        if(fitSkipFactor <= 0.0f){
          fitSkipFactor = 0.5f;
        }
        //draw each peak
        /*for(int32_t i=0;i<fitpar.numFitPeaks;i++){
          fitDrawX=(float)(fitpar.fitStartCh);
          fitDrawX = floorf((float)(fitpar.fitParVal[FITPAR_POS1+(3*i)] - 5.*fitpar.fitParVal[FITPAR_WIDTH1+(3*i)]));
          nextXpos = getXPosFromCh(fitDrawX,width,1);
          if(nextXpos > 0){
            cairo_move_to(cr, nextXpos, getYPos((float)(evalFitOnePeak(fitDrawX,i)),0,height));
          }else{
            cairo_move_to(cr, XORIGIN, getYPos((float)(evalFitOnePeak((float)drawing.lowerLimit,i)),0,height,YORIGIN));
          }
          for(; fitDrawX<=floorf((float)(fitpar.fitParVal[FITPAR_POS1+(3*i)] + 5.*fitpar.fitParVal[FITPAR_WIDTH1+(3*i)])); fitDrawX+= fitSkipFactor){
            nextFitDrawX = fitDrawX + fitSkipFactor;
            nextXpos = getXPosFromCh(nextFitDrawX,width,1);
            if(nextXpos > 0){
              cairo_line_to(cr, nextXpos, getYPos((float)(evalFitOnePeak(nextFitDrawX,i)),0,height));
            }
          }
        }*/
        //draw background
        cairo_set_line_width(cr, 1.0*scaleFactor);
        fitDrawX=(float)(fitpar.fitStartCh);
        nextXpos = getXPosFromCh(fitDrawX,width,1);
        if(nextXpos > 0){
          cairo_move_to(cr, nextXpos, getYPos((float)(evalFitBG(fitDrawX)),0,height));
        }else{
          cairo_move_to(cr, XORIGIN, getYPos((float)(evalFitBG((float)drawing.lowerLimit)),0,height));
        }
        for(; fitDrawX<=(float)(fitpar.fitEndCh); fitDrawX+= fitSkipFactor){
          nextFitDrawX = fitDrawX + fitSkipFactor;
          nextXpos = getXPosFromCh(nextFitDrawX,width,1);
          if(nextXpos > 0){
            cairo_line_to(cr, nextXpos, getYPos((float)(evalFitBG(nextFitDrawX)),0,height));
          }
        }
        cairo_stroke(cr);
        //draw sum of peaks
        cairo_set_line_width(cr, 3.0*scaleFactor);
        fitDrawX=(float)(fitpar.fitStartCh);
        nextXpos = getXPosFromCh(fitDrawX,width,1);
        if(nextXpos > 0){
          cairo_move_to(cr, nextXpos, getYPos((float)(evalFit(fitDrawX)),0,height));
        }else{
          cairo_move_to(cr, XORIGIN, getYPos((float)(evalFit((float)drawing.lowerLimit)),0,height));
        }
        for(; fitDrawX<=(float)(fitpar.fitEndCh); fitDrawX+= fitSkipFactor){
          nextFitDrawX = fitDrawX + fitSkipFactor;
          nextXpos = getXPosFromCh(nextFitDrawX,width,1);
          if(nextXpos > 0){
            cairo_line_to(cr, nextXpos, getYPos((float)(evalFit(nextFitDrawX)),0,height));
          }
        }
        cairo_stroke(cr);
        //draw highlighed peak
        if((drawing.highlightedPeak >= 0)&&(drawing.highlightedPeak <= fitpar.numFitPeaks)&&(showFit>1)){
          cairo_set_line_width(cr, 6.0*scaleFactor);
          fitDrawX = floorf((float)(fitpar.fitParVal[FITPAR_POS1+(3*drawing.highlightedPeak)] - 5.*fitpar.fitParVal[FITPAR_WIDTH1+(3*drawing.highlightedPeak)]));
          nextXpos = getXPosFromCh(fitDrawX,width,1);
          if(nextXpos > 0){
            cairo_move_to(cr, nextXpos, getYPos((float)(evalFitOnePeak(fitDrawX,drawing.highlightedPeak)),0,height));
          }else{
            cairo_move_to(cr, XORIGIN, getYPos((float)(evalFitOnePeak((float)drawing.lowerLimit,drawing.highlightedPeak)),0,height));
          }
          for(; fitDrawX<=floorf((float)(fitpar.fitParVal[FITPAR_POS1+(3*drawing.highlightedPeak)] + 5.*fitpar.fitParVal[FITPAR_WIDTH1+(3*drawing.highlightedPeak)])); fitDrawX+= fitSkipFactor){
            nextFitDrawX = fitDrawX + fitSkipFactor;
            nextXpos = getXPosFromCh(nextFitDrawX,width,1);
            if(nextXpos > 0){
              cairo_line_to(cr, nextXpos, getYPos((float)(evalFitOnePeak(nextFitDrawX,drawing.highlightedPeak)),0,height));
            }
          }
          cairo_stroke(cr);
        }
      }

    }
  }

  //draw axis lines
  cairo_set_line_width(cr, 1.0*scaleFactor);
  setTextColor(cr);
  cairo_move_to(cr, XORIGIN, YORIGIN);
  cairo_line_to(cr, XORIGIN, height);
  switch(drawing.multiplotMode){
    case MULTIPLOT_STACKED:
      //stacked
      for(int32_t i=0;i<drawing.numMultiplotSp;i++){
        cairo_move_to(cr, XORIGIN, YORIGIN + (height-YORIGIN)*(float)(i/(drawing.numMultiplotSp*1.0)));
        cairo_line_to(cr, width, YORIGIN + (height-YORIGIN)*(float)(i/(drawing.numMultiplotSp*1.0)));
      }
      break;
    case MULTIPLOT_OVERLAY_INDEPENDENT:
    case MULTIPLOT_OVERLAY_COMMON:
    case MULTIPLOT_SUMMED:
    case MULTIPLOT_NONE:
    default:
      //single plot
      cairo_move_to(cr, XORIGIN, YORIGIN);
      cairo_line_to(cr, width, YORIGIN);
      break;
  }
  cairo_stroke(cr);
  
  cairo_scale(cr, 1.0, -1.0); //remove axis inversion, so that text is the right way up

  //draw axis labels
  setTextColor(cr);
  char axisLabel[16],axisYLabel[32];
  cairo_text_extents_t extents; //for getting dimensions needed to center text labels
  //x axis
  if(calpar.calMode == 0){
    //set default strings for labels
    sprintf(axisLabel,"Channel #");
    sprintf(axisYLabel,"Value");
  }else{
    //set labels to calibrated units
    strcpy(axisLabel,calpar.calUnit);
    sprintf(axisYLabel,"%s",calpar.calYUnit);
  }
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, (width)*0.55 - (extents.width/2), -3.0);
  cairo_show_text(cr, axisLabel);
  //y axis
  cairo_text_extents(cr, axisYLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, 20.0*scaleFactor, (-height)*0.525 + (extents.width/2));
  cairo_save(cr); //store the context before the rotation
  cairo_rotate(cr, 1.5*3.14159);
  cairo_translate(cr, (width)*0.015, -1.0*((-height)*0.5)); //so that the origin is at the lower left
  cairo_show_text(cr, axisYLabel);
  cairo_stroke(cr);
  cairo_restore(cr); //recall the unrotated context

  //draw fit cursors and indicators
  if((guiglobals.fittingSp != FITSTATE_NOTFITTING)&&(showFit>0)){

    //draw cursors at fit limits if needed
    if(fitpar.fitStartCh >= 0){
      float cursorPos = getXPosFromCh((float)(fitpar.fitStartCh), width, 0);
      if(cursorPos>=0){
        cairo_set_line_width(cr, 2.0*scaleFactor);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, cursorPos, -YORIGIN);
        cairo_line_to(cr, cursorPos, -height);
        cairo_stroke(cr);
      }
    }
    if(fitpar.fitEndCh >= 0){
      float cursorPos = getXPosFromCh((float)(fitpar.fitEndCh), width, 0);
      if(cursorPos>=0){
        cairo_set_line_width(cr, 2.0*scaleFactor);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, cursorPos, -YORIGIN);
        cairo_line_to(cr, cursorPos, -height);
        cairo_stroke(cr);
      }
    }
    //draw peak position indicators
    if((guiglobals.fittingSp >= FITSTATE_SETTINGPEAKS)&&(guiglobals.fittingSp < FITSTATE_FITCOMPLETE)){
      //put markers at guessed positions
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.8);
      cairo_set_line_width(cr, 2.0*scaleFactor);
      for(int32_t i=0;i<fitpar.numFitPeaks;i++){
        if((fitpar.fitPeakInitGuess[i] > drawing.lowerLimit)&&(fitpar.fitPeakInitGuess[i] < drawing.upperLimit)){
          cairo_arc(cr,getXPosFromCh(fitpar.fitPeakInitGuess[i],width,1),(-0.002*(height)*30.0)-getYPos(getDispSpBinVal(0,(int)(fitpar.fitPeakInitGuess[i])-drawing.lowerLimit),0,height),5.,0.,2*G_PI);
        }
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
      }
    }else if(guiglobals.fittingSp == FITSTATE_FITCOMPLETE){
      //put markers at fitted positions
      cairo_set_source_rgb(cr, 0.0, 0.0, 0.8);
      cairo_set_line_width(cr, 2.0*scaleFactor);
      for(int32_t i=0;i<fitpar.numFitPeaks;i++){
        if((fitpar.fitParVal[FITPAR_POS1+(3*i)] > drawing.lowerLimit)&&(fitpar.fitParVal[FITPAR_POS1+(3*i)] < drawing.upperLimit)){
          cairo_arc(cr,getXPosFromCh((float)(fitpar.fitParVal[FITPAR_POS1+(3*i)]),width,1),(-0.002*(height)*30.0)-getYPos((float)(evalFit(fitpar.fitParVal[FITPAR_POS1+(3*i)])),0,height),5.,0.,2*G_PI);
        }
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
      }
    }
  }

  //draw comment indicators
  if(drawComments){
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    switch (drawing.multiplotMode){
      case MULTIPLOT_SUMMED:
        //sum view
        for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
          if(rawdata.chanCommentView[i]==1){
            if(rawdata.chanCommentSp[i]==drawing.displayedView){
              if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                  if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                    if(drawing.highlightedComment == i){
                      cairo_set_line_width(cr, 8.0*scaleFactor);
                    }else{
                      cairo_set_line_width(cr, 4.0*scaleFactor);
                    }
                    float chYVal = rawdata.chanCommentVal[i];
                    if(chYVal < drawing.scaleLevelMin[0]){
                      chYVal = drawing.scaleLevelMin[0];
                    }else if(chYVal > drawing.scaleLevelMax[0]){
                      chYVal = drawing.scaleLevelMax[0];
                    }
                    float xc = getXPosFromCh((float)(rawdata.chanCommentCh[i]),width,1);
                    float yc = -1.0f*getYPos(chYVal,0,height);
                    float radius = 14.0;
                    cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                    cairo_set_font_size(cr, plotFontSize*1.5);
                    cairo_text_extents(cr, "i", &extents);
                    cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                    cairo_show_text(cr, "i");
                    cairo_stroke(cr);
                    //cairo_fill(cr);
                  }
                }
              }
            }
          }
        }
        break;
      case MULTIPLOT_NONE:
        //single non-summed spectrum
        if(drawing.displayedView == -1){
          for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i]==0){
              if(rawdata.chanCommentSp[i]==drawing.multiPlots[0]){
                if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                  if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                    if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                      if(drawing.highlightedComment == i){
                        cairo_set_line_width(cr, 8.0*scaleFactor);
                      }else{
                        cairo_set_line_width(cr, 4.0*scaleFactor);
                      }
                      float chYVal = rawdata.chanCommentVal[i];
                      if(chYVal < drawing.scaleLevelMin[0]){
                        chYVal = drawing.scaleLevelMin[0];
                      }else if(chYVal > drawing.scaleLevelMax[0]){
                        chYVal = drawing.scaleLevelMax[0];
                      }
                      float xc = getXPosFromCh((float)(rawdata.chanCommentCh[i]),width,1);
                      float yc = -1.0f*getYPos(chYVal,0,height);
                      float radius = 14.0;
                      cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                      cairo_set_font_size(cr, plotFontSize*1.5);
                      cairo_text_extents(cr, "i", &extents);
                      cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                      cairo_show_text(cr, "i");
                      cairo_stroke(cr);
                      //cairo_fill(cr);
                    }
                  }
                }
              }
            }
          }
        }else{
          for(int32_t i=0;i<(int32_t)rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i]==1){
              if(rawdata.chanCommentSp[i]==drawing.displayedView){
                if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                  if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                    if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                      if(drawing.highlightedComment == i){
                        cairo_set_line_width(cr, 8.0*scaleFactor);
                      }else{
                        cairo_set_line_width(cr, 4.0*scaleFactor);
                      }
                      float chYVal = rawdata.chanCommentVal[i];
                      if(chYVal < drawing.scaleLevelMin[0]){
                        chYVal = drawing.scaleLevelMin[0];
                      }else if(chYVal > drawing.scaleLevelMax[0]){
                        chYVal = drawing.scaleLevelMax[0];
                      }
                      float xc = getXPosFromCh((float)(rawdata.chanCommentCh[i]),width,1);
                      float yc = -1.0f*getYPos(chYVal,0,height);
                      float radius = 14.0;
                      cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                      cairo_set_font_size(cr, plotFontSize*1.5);
                      cairo_text_extents(cr, "i", &extents);
                      cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                      cairo_show_text(cr, "i");
                      cairo_stroke(cr);
                      //cairo_fill(cr);
                    }
                  }
                }
              }
            }
          }
        }
        break;
      default:
        //comments not implemented
        break;
    }
  }

  //draw cursor at mouse position
  if(guiglobals.drawSpCursor == 1){
    //printf("Drawing cursor!\n");
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1.0);
    setTextColor(cr);
    cairo_move_to(cr, guiglobals.cursorPosX, -YORIGIN);
    cairo_line_to(cr, guiglobals.cursorPosX, -height);
    cairo_stroke(cr);
  }

  return;
}

//update the spectrum drawing area
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr){

  if(!rawdata.openedSp){
    return;
  }

  //printf("Drawing spectrum!\n");

  cairo_set_antialias(cr,CAIRO_ANTIALIAS_FAST); //antialias setting, doesn't apply to text
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *wwindow = gtk_widget_get_window(widget);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);

  drawSpectrum(cr, (float)dasize.width, (float)dasize.height, 1.0, guiglobals.drawSpLabels, guiglobals.drawGridLines, 2, guiglobals.drawSpComments, 1);
}