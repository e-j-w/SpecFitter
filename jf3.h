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
GtkWindow *window, *calibrate_window;
GtkWidget *box1;
GtkWidget *open_button;
GtkWidget *calibrate_button, *fit_button, *contract_button, *multiplot_button;
GtkPopover *contract_popover;
GtkWidget *calibrate_ok_button, *calibrate_cancel_button;
GtkSpinButton *spectrum_selector;
GtkCheckButton *autoscale_button;
GtkScale *contract_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkLabel *status_label;
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
GtkWidget *file_open_dialog;
GtkFileFilter *file_filter;
GtkEntry *cal_entry_unit, *cal_entry_const, *cal_entry_lin, *cal_entry_quad;
GtkBuilder *builder;

//non-GTK GUI globals
int dragstartul, dragstartll; //click and drag position storage parameters

//spectrum drawing globals
double hist[NSPECT][S32K]; //spectrum histogram data
int openedSp; //0=not opened, 1=opened
int dispSp; //# of the spectrum to display
int lowerLimit, upperLimit; //lower and upper limits to plot spectrum (in uncalibrated units ie. channels)
int xChanFocus; //x channel to focus on when zooming
float zoomLevel; //1.0 = zoomed out fully (on x axis)
int autoScale; //0=don't autoscale y axis, 1=autoscale y axis
float scaleLevelMax, scaleLevelMin; //the y scale values, ie. the maximum and minimum values to show on the y axis

//calibration globals
int calMode; //0=no calibration, 1=calibration enabled
float calpar0,calpar1,calpar2; //0th, 1st, and 2nd order calibration parameters
char calUnit[16]; //name of the unit used for calibration

#endif

