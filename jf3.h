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
#include <libgen.h>

#define BIG_NUMBER    1E10
#define SMALL_NUMBER -1E10

//spectrum data file specs
#define S32K   32768
#define NSPECT 100

/* GUI globals */
GtkWindow *window;
GtkHeaderBar *header_bar;
GtkWidget *box1;
GtkWidget *open_button;
GtkWidget *fit_button, *display_button;
GtkPopover *display_popover;
GtkWidget *calibrate_ok_button;
GtkSpinButton *spectrum_selector;
GtkCheckButton *autoscale_button;
GtkScale *contract_scale, *zoom_scale, *pan_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
GtkWidget *file_open_dialog;
GtkFileFilter *file_filter;
//Calibration dialog
GtkWidget *calibrate_button;
GtkWindow *calibrate_window;
GtkEntry *cal_entry_unit, *cal_entry_const, *cal_entry_lin, *cal_entry_quad;
//'About' dialog
GtkAboutDialog *about_dialog;
GtkModelButton *about_button;
//MultiPlot dialog
GtkWidget *multiplot_button;
GtkWindow *multiplot_window;
GtkListStore *multiplot_liststore;
GtkTreeView *multiplot_tree_view;
GtkTreeViewColumn *multiplot_column1, *multiplot_column2;
GtkTreeSelection *multiplot_tree_selection;
GtkCellRenderer *multiplot_cr1, *multiplot_cr2;
//builder
GtkBuilder *builder;

//non-GTK GUI globals
int dragstartul, dragstartll; //click and drag position storage parameters

//spectrum drawing globals
double hist[NSPECT][S32K]; //spectrum histogram data
char histComment[NSPECT][256]; //spectrum description/comment
int openedSp; //0=not opened, 1=opened
int glob_numSpOpened; //number of spectra in the opened file(s)
//int dispSp; //# of the spectrum to display
int lowerLimit, upperLimit; //lower and upper limits to plot spectrum (in uncalibrated units ie. channels)
int xChanFocus; //x channel to focus on when zooming
float zoomLevel; //1.0 = zoomed out fully (on x axis)
int autoScale; //0=don't autoscale y axis, 1=autoscale y axis
float scaleLevelMax, scaleLevelMin; //the y scale values, ie. the maximum and minimum values to show on the y axis
int contractFactor; //the number of channels per bin (default=1)
int glob_multiplotMode; //0=no multiplot, 1=summed spectra, 2=overlay spectra (common scaling), 3=overlay spectra (independent scaling), 4=stacked view
int glob_numMultiplotSp; //number of spectra to show in multiplot mode
int glob_multiPlots[NSPECT]; //indices of all the spectra to show in multiplot mode

//calibration globals
int calMode; //0=no calibration, 1=calibration enabled
float calpar0,calpar1,calpar2; //0th, 1st, and 2nd order calibration parameters
char calUnit[16]; //name of the unit used for calibration

#endif

