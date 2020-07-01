/* J. Williams, 2020 */

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
#include <sys/types.h>
#include <sys/stat.h>

#include "lin_eq_solver.h"

#define BIG_NUMBER    1E30
#define SMALL_NUMBER -1E30
#define MAX_DISP_SP   12 //maximum number of spectra which may be displayed at once
#define MAX_FIT_PK    10 //maximum number of peaks which may be fit at once

//spectrum data file specs (be careful if changing these, can break compatibility)
#define S32K      32768 //maximum number of channels per spectrum in .mca and .fmca (changing breaks file compatibility)
#define NSPECT    100   //maximum number of spectra which may be opened at once (for compatibility should be 255 or less)
#define MAXNVIEWS 100   //maximum number of views which can be saved by the user
#define NCHCOM    1000  //maximum number of comments that can be placed by the user on individual channels

/* GUI globals */
GtkWindow *window;
GtkHeaderBar *header_bar;
GtkButton *open_button, *append_button;
GtkButton *fit_button, *display_button;
GtkPopover *display_popover;
GtkButton *calibrate_button;
GtkButton *manage_spectra_button;
GtkButton *to_save_menu_button, *save_button, *save_button_radware, *save_button_text;
GtkButton *help_button;
GtkLabel *bottom_info_text;
GtkFileChooser *file_open_dialog, *file_save_dialog;
GtkFileFilter *file_filter;
GtkAccelGroup *main_window_accelgroup;
GtkBox *no_sp_box;
GtkImage *display_button_icon;
GtkImage *no_sp_image;
//display menu
GtkSpinButton *spectrum_selector;
GtkCheckButton *autoscale_button, *logscale_button, *cursor_draw_button;
GtkScale *contract_scale, *zoom_scale; //*pan_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkButton *sum_all_button;
//spectrum drawing
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
//fit overlay/revealer
GtkRevealer *revealer_info_panel;
GtkLabel *revealer_info_label;
GtkButton *fit_cancel_button, *fit_fit_button, *fit_preferences_button;
GtkSpinner *fit_spinner;
//Calibration dialog
GtkWidget *calibrate_ok_button, *remove_calibration_button;
GtkWindow *calibrate_window;
GtkEntry *cal_entry_unit, *cal_entry_const, *cal_entry_lin, *cal_entry_quad, *cal_entry_y_axis;
//comment dialog
GtkWindow *comment_window;
GtkEntry *comment_entry;
GtkButton *comment_ok_button, *remove_comment_button;
GtkAccelGroup *comment_window_accelgroup;
//'About' dialog
GtkAboutDialog *about_dialog;
GtkModelButton *about_button;
//MultiPlot and manage dialog
GtkWindow *multiplot_manage_window;
GtkStack *multiplot_manage_stack, *multiplot_manage_button_stack;
GtkStackSwitcher *multiplot_manage_stack_switcher;
//multiplot
GtkWidget *multiplot_box;
GtkButton *multiplot_button, *multiplot_ok_button, *multiplot_make_view_button;
GtkListStore *multiplot_liststore;
GtkTreeView *multiplot_tree_view;
GtkTreeViewColumn *multiplot_column1, *multiplot_column2;
GtkCellRenderer *multiplot_cr1, *multiplot_cr2, *multiplot_cr3;
GtkComboBoxText *multiplot_mode_combobox;
//view
GtkListStore *view_liststore;
GtkTreeView *view_tree_view;
GtkTreeViewColumn *view_column1;
GtkCellRenderer *view_cr1;
//manage
GtkWidget *manage_box;
GtkButton *manage_delete_button;
GtkListStore *manage_liststore;
GtkTreeView *manage_tree_view;
GtkTreeViewColumn *manage_column1, *manage_column2;
GtkCellRenderer *manage_cr1, *manage_cr2;
//export data dialog
GtkWindow *export_options_window;
GtkLabel *export_description_label, *export_note_label;
GtkComboBoxText *export_mode_combobox;
GtkCheckButton *export_rebin_checkbutton;
GtkButton *export_options_save_button;
//preferences dialog
GtkModelButton *preferences_button;
GtkWindow *preferences_window;
GtkNotebook *preferences_notebook;
GtkCheckButton *discard_empty_checkbutton, *bin_errors_checkbutton, *round_errors_checkbutton, *dark_theme_checkbutton, *spectrum_label_checkbutton, *spectrum_comment_checkbutton, *autozoom_checkbutton;
GtkCheckButton *relative_widths_checkbutton;
GtkButton *preferences_apply_button;
GtkComboBoxText *weight_mode_combobox;
GtkCheckButton *popup_results_checkbutton;
GtkCheckButton *animation_checkbutton;
//shortcuts window
GtkModelButton *shortcuts_button;
GtkShortcutsWindow *shortcuts_window;
//help window
GtkWindow *help_window;
GtkImage *display_button_icon1, *display_button_icon2;
//builder
GtkBuilder *builder;

//custom icons
GdkPixbuf *appIcon, *spIconPixbuf, *spIconPixbufDark;

//non-GTK GUI globals
struct {
  char draggingSp; //0 if no drag motion, 1 if dragging
  int dragstartul, dragstartll; //click and drag position storage parameters
  float dragStartX; //start cursor position when dragging
  float cursorPosX, cursorPosY; //cursor position
  char drawSpCursor; //0 = don't draw vertical cursor on spectrum, 1=draw, -1=drawing disabled
  char drawSpLabels; //0 = don't draw labels, 1 = draw labels
  char drawSpComments; //0 = don't draw comments, 1 = draw comments
  char fittingSp; //0=not fitting, 1=selecting limits, 2=selecting peaks, 3=fitting, 4=refining fit, 5=fitted (display fit)
  int deferSpSelChange;
  int deferToggleRow;
  char showBinErrors; //0=don't show, 1=show
  char roundErrors; //0=don't round, 1=round
  char autoZoom; //0=don't autozoom, 1=autozoom
  int commentEditInd;
  char preferDarkTheme; //0=prefer light, 1=prefer dark
  char popupFitResults; //0=don't popup results after fit, 1=popup results
  char useZoomAnimations; //0=don't use, 1=use
  int framesSinceZoom;
  char exportFileType; //0=text, 1=radware
} guiglobals;

//raw histogram data globals
struct {
  double hist[NSPECT][S32K]; //spectrum histogram data
  char histComment[NSPECT][256]; //spectrum description/comment
  char openedSp; //0=not opened, 1=opened
  unsigned char numSpOpened; //number of spectra in the opened file(s)
  unsigned char numFilesOpened; //number of files containing spectra opened
  char viewComment[MAXNVIEWS][256]; //view description/comment
  char viewMultiplotMode[MAXNVIEWS]; //multiplot mode for each saved view
  int viewNumMultiplotSp[MAXNVIEWS]; //number of spectra to show for each saved view
  double viewScaleFactor[MAXNVIEWS][NSPECT]; //scaling factors for each spectrum in each saved view
  int viewMultiPlots[MAXNVIEWS][NSPECT]; //indices of all the spectra to show for each saved view
  unsigned char numViews; //number of views that have been saved
  char chanComment[NCHCOM][256]; //channel comment text
  char chanCommentSp[NCHCOM]; //spectra at which channel comments are displayed
  int chanCommentCh[NCHCOM]; //channels at which channel comments are displayed
  float chanCommentVal[NCHCOM]; //y-values at which channel comments are displayed
  unsigned int numChComments; //number of comments which have been placed
  char dropEmptySpectra; //0=don't discard empty spectra on import, 1=discard
} rawdata;

//spectrum drawing globals
struct {
  int lowerLimit, upperLimit; //lower and upper limits to plot spectrum (in uncalibrated units ie. channels)
  int xChanFocus, xChanFocusChangePerFrame, xChanToFocus; //x channel to focus on when zooming
  float zoomLevel, zoomToLevel; //1.0 = zoomed out fully (on x axis)
  int autoScale; //0=don't autoscale y axis, 1=autoscale y axis
  int logScale; //0=draw in linear scale, 1=draw in log scale (y-axis)
  float scaleLevelMax[MAX_DISP_SP], scaleLevelMin[MAX_DISP_SP]; //the y scale values, ie. the maximum and minimum values to show on the y axis
  int contractFactor; //the number of channels per bin (default=1)
  char multiplotMode; //0=no multiplot, 1=summed spectra, 2=overlay spectra (common scaling), 3=overlay spectra (independent scaling), 4=stacked view
  int numMultiplotSp; //number of spectra to show in multiplot mode
  double scaleFactor[NSPECT]; //scaling factors for each spectrum
  int multiPlots[NSPECT]; //indices of all the spectra to show in multiplot mode
  float spColors[MAX_DISP_SP*3];
  signed char highlightedPeak; //the peak to highlight when drawing spectra, -1=don't highlight
  signed char highlightedComment; //the comment to highlight when drawing spectra, -1=don't highlight
} drawing;

//calibration globals
struct {
  char calMode; //0=no calibration, 1=calibration enabled
  float calpar0,calpar1,calpar2; //0th, 1st, and 2nd order calibration parameters
  char calUnit[16]; //name of the unit used for calibration
  char calYUnit[32]; //name of the y-axis units
} calpar;

//fitting globals
struct {
  int fitStartCh, fitEndCh; //upper and lower channel bounds for fitting
  int ndf; //DOF for fit
  float fitPeakInitGuess[MAX_FIT_PK]; //initial guess of peak positions, in channels
  double widthFGH[3]; //F,G,H parameters used to evaluate widths
  double fitParVal[6+(3*MAX_FIT_PK)]; //parameter values found by the fitter
  double fitParErr[6+(3*MAX_FIT_PK)]; //errors in parameter values
  int numFitPeaks; //number of peaks to fit
  char fixRelativeWidths; //0=don't fix width, 1=fix widths
  char weightMode; //0=weight using data (properly weighting for background subtraction), 1=weight using fit, 2=no weights
  double relWidths[MAX_FIT_PK]; //relative width factors
  char errFound; //whether or not paramter errors have been found
} fitpar;

#endif

