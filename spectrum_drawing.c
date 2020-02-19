
#define ZOOM_X  100.0
#define ZOOM_Y  100.0

//get the bin position in the histogram plot
float getXPos(int bin, float clip_x1, float clip_x2){
	return clip_x1 + (bin*(clip_x2-clip_x1)/(upperLimit-lowerLimit));
}

//draw a spectrum
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{

	if(openedSp){
		gtk_label_set_text(GTK_LABEL(no_sp_label), (const gchar *)"");
	}else{
		gtk_label_set_text(GTK_LABEL(no_sp_label), (const gchar *)"No spectrum loaded");
		return;
	}

	if (dispSp >= NSPECT)
	{
		printf("Spectrum number too high (%i)!\n", dispSp);
		return;
	}

	GdkRectangle da;            // GtkDrawingArea size
    gdouble clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
    GdkWindow *window = gtk_widget_get_window(widget);

    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry (window, &da.x, &da.y, &da.width, &da.height);

    // Draw the background colour
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint (cr);

    // transform the coordinate system
    cairo_translate (cr, 0.0, da.height); //so that the origin is at the lower left
    cairo_scale (cr, 1.0, -1.0); //so that positive y values go up

    /* Determine the data points to calculate (ie. those in the clipping zone */
    //cairo_device_to_user_distance (cr, &dx, &dy);
    cairo_clip_extents (cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    cairo_set_line_width (cr, 3.0);

    

	//draw histogram
	int i;
	for(i=0;i<(upperLimit-lowerLimit-1);i++){
		//printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,clip_x1,clip_x2), yScaling*hist[dispSp][lowerLimit+i],hist[dispSp][lowerLimit+i],clip_x1,clip_x2);
		cairo_move_to (cr, getXPos(i,clip_x1,clip_x2), yScaling*hist[dispSp][lowerLimit+i]);
		cairo_line_to (cr, getXPos(i+1,clip_x1,clip_x2), yScaling*hist[dispSp][lowerLimit+i]);
		cairo_line_to (cr, getXPos(i+1,clip_x1,clip_x2), yScaling*hist[dispSp][lowerLimit+i+1]);
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
