#ifndef SV_H
#define SV_H

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <cairo.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

//spectrum data file specs
#define S32K 32768
#define NSPECT 100

//GUI globals
GtkWindow *window;
GtkWidget *fixedgrid1;
GtkWidget *open_button;
GtkWidget *reload_button;
GtkSpinButton *spectrum_selector;
GtkScale *contract_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkWidget *no_sp_label;
GtkWidget *spectrum_drawing_area;
GtkWidget *file_open_dialog;
GtkFileFilter *file_filter;
GtkBuilder *builder;

//spectrum drawing globals
double hist[NSPECT][S32K]; //spectrum histogram data
int openedSp; //0=not opened, 1=opened
int dispSp; //# of the spectrum to display
int lowerLimit, upperLimit; //lower and upper limits to plot spectrum
float yScaling; //y axis scaling
int xChanFocus; //x channel to focus on when zooming
float zoomLevel; //1.0 = zoomed out fully

#endif

