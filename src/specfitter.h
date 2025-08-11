/* © J. Williams, 2020-2025 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <cairo.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <libgen.h>
#include <glib/gstdio.h>
#include <sys/stat.h>

#include "lin_eq_solver.h"
#include "utils.h"

#define BIG_NUMBER    1E30
#define SMALL_NUMBER -1E30

#define MAX_DISP_SP   12 //maximum number of spectra which may be displayed at once
#define MAX_FIT_PK    10 //maximum number of peaks which may be fit at once (when changing, also change MAX_DIM in lin_eq_solver.h)

/* Data file specs (be careful if changing these, can break compatibility) */
#define S32K          32768 //maximum number of channels per spectrum in .mca, .fmca, and .dmca (changing breaks file compatibility)
#define NSPECT        128   //maximum number of spectra which may be opened at once (for compatibility should be 255 or less)
#define MAXNVIEWS     128   //maximum number of views which can be saved by the user
#define MAXNSAVEDFITS 16    //maximum number of fits which can be saved by the user
#define NCHCOM        1000  //maximum number of comments that can be placed by the user on individual channels

/* GUI globals */
GtkWindow *window;
GtkHeaderBar *header_bar;
GtkButton *open_button, *append_button;
GtkButton *fit_button, *display_button;
GtkPopover *display_popover;
GtkButton *calibrate_button;
GtkButton *manage_spectra_button;
GtkButton *to_save_menu_button, *save_button, *save_button_radware;
GtkButton *save_button_fmca, *save_button_dmca, *save_button_text, *save_button_png;
GtkButton *help_button;
GtkLabel *bottom_info_text;
GtkFileChooser *file_open_dialog, *file_save_dialog;
GtkFileFilter *file_filter;
GtkAccelGroup *main_window_accelgroup;
GtkBox *no_sp_box;
GtkImage *no_sp_image;
//display menu
GtkSpinButton *spectrum_selector;
GtkComboBoxText *value_mode_combobox;
GtkCheckButton *autoscale_button, *logscale_button, *cursor_draw_button;
GtkLabel *display_spectrumname_label;
GtkScale *contract_scale, *zoom_scale; //*pan_scale;
GtkAdjustment *spectrum_selector_adjustment, *contract_adjustment;
GtkButton *sum_all_button;
//spectrum drawing
GtkWidget *spectrum_drawing_area;
GtkGesture *spectrum_drag_gesture;
//fit overlay/revealer
GtkRevealer *revealer_info_panel;
GtkBox *fit_button_box, *fit_display_button_box;
GtkLabel *fit_panel_label, *fit_info_label;
GtkButton *fit_cancel_button, *fit_fit_button, *fit_refit_button, *fit_preferences_button, *fit_save_button, *fit_dismiss_button;
GtkSpinner *fit_spinner;
//Calibration dialog
GtkWidget *calibrate_ok_button, *remove_calibration_button;
GtkWindow *calibrate_window;
GtkEntry *cal_entry_unit, *cal_entry_const, *cal_entry_lin, *cal_entry_quad, *cal_entry_y_axis;
GtkAccelGroup *calibration_window_accelgroup;
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
GtkWidget *view_box, *view_list_box, *no_view_box;
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
GtkRevealer *export_options_revealer;
GtkCheckButton *export_rebin_checkbutton;
GtkButton *export_options_save_button;
//export image dialog
GtkWindow *export_image_window;
GtkSpinButton *export_h_res_spinbutton, *export_v_res_spinbutton;
GtkCheckButton *export_image_label_checkbutton, *export_image_fit_checkbutton, *export_image_gridline_checkbutton;
GtkComboBoxText *export_axissize_combobox;
GtkButton *export_image_save_button;
//preferences dialog
GtkModelButton *preferences_button;
GtkWindow *preferences_window;
GtkNotebook *preferences_notebook;
GtkCheckButton *discard_empty_checkbutton, *bin_errors_checkbutton, *round_errors_checkbutton, *dark_theme_checkbutton;
GtkCheckButton *spectrum_label_checkbutton, *spectrum_comment_checkbutton, *spectrum_gridline_checkbutton, *autozoom_checkbutton;
GtkRevealer *manual_width_revealer, *skew_parameters_revealer, *peak_parameters_revealer, *background_parameters_revealer;
GtkSpinButton *manual_width_spinbutton, *manual_width_offset_spinbutton;
GtkCheckButton *limit_centroid_checkbutton;
GtkSpinButton *limit_centroid_spinbutton;
GtkCheckButton *fix_skew_amplitude_checkbutton, *fix_beta_checkbutton;
GtkSpinButton *skew_amplitude_spinbutton, *beta_spinbutton;
GtkCheckButton *relative_widths_checkbutton, *step_function_checkbutton, *positive_peak_checkbutton, *inflate_errors_checkbutton;
GtkButton *preferences_apply_button;
GtkComboBoxText *background_type_combobox, *peak_shape_combobox, *peak_width_combobox, *weight_mode_combobox;
GtkCheckButton *animation_checkbutton;
//shortcuts window
GtkModelButton *shortcuts_button;
GtkShortcutsWindow *shortcuts_window;
//help window
GtkWindow *help_window;
//builder
GtkBuilder *builder;
//custom icons
GdkPixbuf *appIcon, *spIconPixbuf, *spIconPixbufDark;
//timing
GdkFrameClock *frameClock;
//other
gchar *currentFolderSelection; //folder selection for file choosers

//fitting globals
typedef struct {
  int32_t fitStartCh, fitEndCh, fitMidCh; //upper and lower channel bounds for fitting, and middle channel
  int32_t ndf; //DOF for fit
  float fitPeakInitGuess[MAX_FIT_PK]; //initial guess of peak positions, in channels
  double widthFGH[3]; //F,G,H parameters used to evaluate widths
  //fit parameters (indices defined in fit_par_enum): 
  //0, 1, 2       : quadratic background
  //3             : R (ratio of symmetric and skewed Gaussians, range 0 to 1)
  //4             : beta (skewness)
  //5             : step function
  //6, 9, 12 ...  : Peak position(s)
  //7, 10, 13 ... : Peak width(s)
  //8, 11, 14 ... : Peak amplitude(s)
  long double fitParVal[6+(3*MAX_FIT_PK)]; //parameter values found by the fitter
  long double fitParErr[6+(3*MAX_FIT_PK)]; //errors in parameter values
  long double areaVal[MAX_FIT_PK], areaErr[MAX_FIT_PK]; //areas of peaks and errors
  long double centroidVal[MAX_FIT_PK]; //centroids of peaks (taking skewed shape into account)
  long double chisq; //fit chisq
  uint8_t fitParFree[6+(3*MAX_FIT_PK)]; //whether individual parameters are fixed or free
  uint8_t numFreePar; //number of fit parameters which have been freed
  uint8_t bgType; //0=constant, 1=linear, 2=quadratic
  uint8_t fitType; //fit type (values from fit_type_enum)
  uint8_t numFitPeaks; //number of peaks to fit
  float manualWidthVal, manualWidthOffset; //manually fixed peak width value and offset/error
  uint8_t limitCentroid; //whether the centroid range is limited
  uint8_t fixSkewAmplitide; //whether the R parameter is fixed
  uint8_t fixBeta; //whether the skewness parameter is fixed
  float limitCentroidVal; //number of channels to limit the centroid by
  float fixedRVal; //fixed value of the skewed component amplitude R, if fixSkewAmplitide==1
  float fixedBetaVal; //fixed value of the skewness beta, if fixBeta==1
  uint8_t peakWidthMethod; //values from peak_width_mode_enum
  uint8_t stepFunction; //0=no step function, 1=step function
  uint8_t forcePositivePeaks; //0=peak amplitude is free, 1=peak amplitude must be +ve
  uint8_t inflateErrors; //0=don't inflate errors based on chisq, 1=inflate errors
  uint8_t weightMode; //uses values from fit_weight_mode_enum: 0=weight using data (properly weighting for background subtraction), 1=weight using fit, 2=no weights
  uint8_t prevFitNumPeaks; //number of peaks in the previous fit
  int32_t prevFitStartCh, prevFitEndCh;
  float prevFitPeakInitGuess[MAX_FIT_PK]; //previous fit guess of peak positions, in channels
  long double prevFitWidths[MAX_FIT_PK];
  uint8_t fittingSp; //uses values from fit_state_enum: 0=not fitting, 1=selecting limits, 2=selecting peaks, 3=fitting, 4,5=refining fit, 6=fitted (display fit)
} fitpar;

//non-GTK GUI globals
struct {
  uint8_t scrollDir;
  double accSmoothScrollDelta; //storage variable for smooth scrolling
  uint8_t draggingSp; //0 if no drag motion, 1 if dragging
  int32_t dragstartul, dragstartll; //click and drag position storage parameters
  float dragStartX; //start cursor position when dragging
  float cursorPosX, cursorPosY; //cursor position
  int8_t drawSpCursor; //0 = don't draw vertical cursor on spectrum, 1=draw, -1=drawing disabled
  uint8_t drawSpLabels; //0 = don't draw labels, 1 = draw labels
  uint8_t drawSpComments; //0 = don't draw comments, 1 = draw comments
  uint8_t drawGridLines; //0 = don't draw grid lines on plot, 1 = draw grid lines
  int32_t deferSpSelChange;
  int32_t deferToggleRow;
  uint8_t showBinErrors; //0=don't show, 1=show
  uint8_t roundErrors; //0=don't round, 1=round
  uint8_t autoZoom; //0=don't autozoom, 1=autozoom
  uint8_t commentEditMode; //0=editing comment, 1=editing view title
  int32_t commentEditInd;
  uint8_t preferDarkTheme; //0=prefer light, 1=prefer dark
  uint8_t usingDarkTheme;
  uint8_t useZoomAnimations; //0=don't use, 1=use
  uint8_t exportFileType; //0=text, 1=radware, 2=fmca, 3=dmca
} guiglobals;

//raw histogram data globals
struct {
  double hist[NSPECT][S32K]; //spectrum histogram data
  double histErr[NSPECT][S32K]; //errors per bin of the histogram
  char histComment[NSPECT][256]; //spectrum description/comment
  uint8_t hasCustomErr[NSPECT];
  uint8_t openedSp; //0=not opened, 1=opened
  uint8_t numSpOpened; //number of spectra in the opened file(s)
  uint8_t numFilesOpened; //number of files containing spectra opened
  char viewComment[MAXNVIEWS][256]; //view description/comment
  uint8_t viewMode[MAXNVIEWS]; //multiplot mode for each saved view, uses values from view_mode_enum
  uint8_t viewNumMultiplotSp[MAXNVIEWS]; //number of spectra to show for each saved view
  double viewScaleFactor[MAXNVIEWS][NSPECT]; //scaling factors for each spectrum in each saved view
  uint8_t viewMultiPlots[MAXNVIEWS][NSPECT]; //indices of all the spectra to show for each saved view
  uint8_t numViews; //number of views that have been saved
  uint8_t numSavedFits; //number of fits that have been saved
  fitpar dispFitPar;
  fitpar savedFitPar[MAXNSAVEDFITS];
  char chanComment[NCHCOM][256]; //channel comment text
  uint8_t chanCommentView[NCHCOM]; //0=comment is on spectrum, 1=comment is on view
  uint8_t chanCommentSp[NCHCOM]; //spectrum/view number at which channel comments are displayed
  int32_t chanCommentCh[NCHCOM]; //channels at which channel comments are displayed
  float chanCommentVal[NCHCOM]; //y-values at which channel comments are displayed
  uint32_t numChComments; //number of comments which have been placed
  uint8_t dropEmptySpectra; //0=don't discard empty spectra on import, 1=discard
} rawdata;

//spectrum drawing globals
struct {
  int32_t lowerLimit, upperLimit; //lower and upper limits to plot spectrum (in uncalibrated units ie. channels)
  int32_t xChanFocus; //x channel to focus on when zooming
  float zoomFocusFrac; //fraction of the visible area to have before the zoom focus point
  float zoomLevel, zoomToLevel; //1.0 = zoomed out fully (on x axis)
  uint8_t autoScale; //0=don't autoscale y axis, 1=autoscale y axis
  uint8_t logScale; //0=draw in linear scale, 1=draw in log scale (y-axis)
  uint8_t valueDrawMode; //values from value_drawmode_enum
  char zoomingSpX, zoomingSpY; //0 if no zoom motion, 1 if zooming
  gint64 zoomXStartFrameTime, zoomYStartFrameTime; //the frames at which the x and y-scale zooms were started
  gint64 zoomXLastFrameTime, zoomYLastFrameTime; //the most recent frames during the x and y-scale zooms
  double scaleToLevelMax[MAX_DISP_SP], scaleToLevelMin[MAX_DISP_SP]; //the y scale values to zoom to
  double scaleLevelMax[MAX_DISP_SP], scaleLevelMin[MAX_DISP_SP]; //the y scale values, ie. the maximum and minimum values to show on the y axis
  uint8_t contractFactor; //the number of channels per bin (default=1)
  uint8_t multiplotMode; //uses values from view_mode_enum: 0=no multiplot, 1=summed spectra, 2=overlay spectra (common scaling), 3=overlay spectra (independent scaling), 4=stacked view
  uint8_t numMultiplotSp; //number of spectra to show in multiplot mode
  double scaleFactor[NSPECT]; //scaling factors for each spectrum
  uint8_t multiPlots[NSPECT]; //indices of all the spectra to show in multiplot mode
  int32_t displayedView; //-1 if no view is being displayed, -2 if temporary view displayed, otherwise the index of the displayed view
  int32_t displayedSavedFit; //-1 if no saved fit is being displayed, otherwise the index of the view corresponding to the saved fit
  float spColors[MAX_DISP_SP*3];
  int8_t highlightedPeak; //the peak to highlight when drawing spectra, -1=don't highlight
  int8_t highlightedComment; //the comment to highlight when drawing spectra, -1=don't highlight
} drawing;

//calibration globals
struct {
  float calpar[3]; //0th, 1st, and 2nd order calibration parameters
  uint8_t calMode; //0=no calibration, 1=calibration enabled
  char calUnit[16]; //name of the unit used for calibration
  char calYUnit[32]; //name of the y-axis units
} calpar;

enum fit_type_enum{FITTYPE_SYMMETRIC, FITTYPE_SKEWED, FITTYPE_BGONLY, FITTYPE_SUMREGION, FITTYPE_ENUM_LENGTH};
enum peak_width_mode_enum{PEAKWIDTHMODE_FREE, PEAKWIDTHMODE_RELATIVE, PEAKWIDTHMODE_PREVIOUS, PEAKWIDTHMODE_MANUAL, PEAKWIDTHMODE_ENUM_LENGTH};
enum fit_weight_mode_enum{FITWEIGHT_DATA, FITWEIGHT_FIT, FITWEIGHT_NONE, FITWEIGHT_ENUM_LENGTH};
enum fit_par_enum{FITPAR_BGCONST,FITPAR_BGLIN,FITPAR_BGQUAD,FITPAR_R,FITPAR_BETA,FITPAR_STEP,FITPAR_POS1,FITPAR_WIDTH1,FITPAR_AMP1,FITPAR_ENUM_LENGTH};
enum fit_state_enum{FITSTATE_NOTFITTING, FITSTATE_SETTINGLIMITS, FITSTATE_SETTINGPEAKS, FITSTATE_FITTING, FITSTATE_FITCOMPLETE, FITSTATE_FITCOMPLETEDUBIOUS, FITSTATE_ENUM_LENGTH};
enum view_mode_enum{VIEWTYPE_NONE, VIEWTYPE_SUMMED, VIEWTYPE_OVERLAY_COMMON, VIEWTYPE_OVERLAY_INDEPENDENT, VIEWTYPE_STACKED, VIEWTYPE_SAVEDFIT_OFSP, VIEWTYPE_SAVEDFIT_OFVIEW, VIEWTYPE_ENUM_LENGTH};
enum value_drawmode_enum{VALUE_DATA, VALUE_PLUSERR, VALUE_MINUSERR, VALUE_ENUM_LENGTH};