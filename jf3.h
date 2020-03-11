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

#include "lin_eq_solver.h"

#define BIG_NUMBER    1E10
#define SMALL_NUMBER -1E10
#define MAX_DISP_SP   12 //maximum number of spectra which may be displayed at once
#define MAX_FIT_PK    10 //maximum number of peaks which may be fit at once


//spectrum data file specs
#define S32K   32768
#define NSPECT 100

/* GUI globals */
GtkWindow *window;
GtkHeaderBar *header_bar;
GtkWidget *open_button, *append_button;
GtkWidget *fit_button, *display_button;
GtkPopover *display_popover;
GtkWidget *calibrate_ok_button;
GtkSpinButton *spectrum_selector;
GtkCheckButton *autoscale_button, *cursor_draw_button;
GtkScale *contract_scale, *zoom_scale; //*pan_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkLabel *bottom_info_text;
GtkWidget *file_open_dialog;
GtkFileFilter *file_filter;
//spectrum drawing
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
//spectrum overlay
GtkInfoBar *overlay_info_bar;
GtkLabel *overlay_info_label;
GtkButton *fit_cancel_button, *fit_fit_button;
//Calibration dialog
GtkWidget *calibrate_button;
GtkWindow *calibrate_window;
GtkEntry *cal_entry_unit, *cal_entry_const, *cal_entry_lin, *cal_entry_quad;
//'About' dialog
GtkAboutDialog *about_dialog;
GtkModelButton *about_button;
//MultiPlot dialog
GtkWidget *multiplot_button, *multiplot_ok_button;
GtkWindow *multiplot_window;
GtkListStore *multiplot_liststore;
GtkTreeView *multiplot_tree_view;
GtkTreeViewColumn *multiplot_column1, *multiplot_column2;
GtkTreeSelection *multiplot_tree_selection;
GtkCellRenderer *multiplot_cr1, *multiplot_cr2;
GtkComboBoxText *multiplot_mode_combobox;
//builder
GtkBuilder *builder;

//non-GTK GUI globals
struct {
  int draggingSp; //0 if no drag motion, 1 if dragging
  int dragstartul, dragstartll; //click and drag position storage parameters
  float dragStartX; //start cursor position when dragging
  float cursorPosX, cursorPosY; //cursor position
  int drawSpCursor; //0 = don't draw vertical cursor on spectrum, 1=draw, -1=drawing disabled
  int fittingSp; //0=not fitting, 1=selecting limits, 2=selecting peaks, 3=fitted (display fit)
  int deferFit; //0=no action, 1=fit is being deferred until a later time
} gui;

//imported data globals
struct {
  double hist[NSPECT][S32K]; //spectrum histogram data
  char histComment[NSPECT][256]; //spectrum description/comment
  int openedSp; //0=not opened, 1=opened
  int numSpOpened; //number of spectra in the opened file(s)
  int numFilesOpened; //number of files containing spectra opened
} rawdata;

//spectrum drawing globals
struct {
  int lowerLimit, upperLimit; //lower and upper limits to plot spectrum (in uncalibrated units ie. channels)
  int xChanFocus; //x channel to focus on when zooming
  float zoomLevel; //1.0 = zoomed out fully (on x axis)
  int autoScale; //0=don't autoscale y axis, 1=autoscale y axis
  float scaleLevelMax[MAX_DISP_SP], scaleLevelMin[MAX_DISP_SP]; //the y scale values, ie. the maximum and minimum values to show on the y axis
  int contractFactor; //the number of channels per bin (default=1)
  int multiplotMode; //0=no multiplot, 1=summed spectra, 2=overlay spectra (common scaling), 3=overlay spectra (independent scaling), 4=stacked view
  int numMultiplotSp; //number of spectra to show in multiplot mode
  int multiPlots[NSPECT]; //indices of all the spectra to show in multiplot mode
  float spColors[MAX_DISP_SP*3];
} drawing;

//calibration globals
struct {
  int calMode; //0=no calibration, 1=calibration enabled
  float calpar0,calpar1,calpar2; //0th, 1st, and 2nd order calibration parameters
  char calUnit[16]; //name of the unit used for calibration
} calpar;

//fitting globals
struct {
  int fitStartCh, fitEndCh; //upper and lower channel bounds for fitting
  float fitPeakInitGuess[MAX_FIT_PK]; //initial guess of peak positions, in channels
  double widthFGH[3]; //F,G,H parameters used to evaluate widths
  double fitParVal[6+(3*MAX_FIT_PK)]; //parameter values found by the fitter
  int numFitPeaks; //number of peaks to fit
} fitpar;

#endif

