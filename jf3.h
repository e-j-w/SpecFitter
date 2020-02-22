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

#define BIG_NUMBER    1E10
#define SMALL_NUMBER -1E10

//spectrum data file specs
#define S32K   32768
#define NSPECT 100

//GUI globals
GtkWindow *window;
GtkWidget *box1;
GtkWidget *open_button;
GtkWidget *reload_button;
GtkSpinButton *spectrum_selector;
GtkCheckButton *autoscale_button;
GtkScale *contract_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkLabel *status_label;
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
GtkWidget *file_open_dialog;
GtkFileFilter *file_filter;
GtkBuilder *builder;

//spectrum drawing globals
double hist[NSPECT][S32K]; //spectrum histogram data
int openedSp; //0=not opened, 1=opened
int dispSp; //# of the spectrum to display
int lowerLimit, upperLimit; //lower and upper limits to plot spectrum
int xChanFocus; //x channel to focus on when zooming
float zoomLevel; //1.0 = zoomed out fully (on x axis)
int autoScale; //0=don't autoscale y axis, 1=autoscale y axis
float scaleLevelMax, scaleLevelMin; //the y scale values, ie. the maximum and minimum values to show on the y axis

int dragstartul, dragstartll; //click and drag position storage parameters

#endif

