#include "jf3.h"

#include "jf3-resources.c"
#include "utils.c"
#include "read_data.c"
#include "read_config.c"
#include "fit_data.c"
#include "spectrum_drawing.c"

void updateConfigFile(){
  char dirPath[256];
  strcpy(dirPath,"");
	strcat(dirPath,getenv("HOME"));
	strcat(dirPath,"/.config/jf3/jf3.conf");
  FILE *configFile = fopen(dirPath, "w");
  if(configFile != NULL){
    writeConfigFile(configFile); //write the default configuration values
    fclose(configFile);
  }else{
    printf("WARNING: Unable to write preferences to configuration file.\n");
  }
}

//function for opening a single file without UI (ie. from the command line)
//if append=1, append this file to already opened files
void openSingleFile(char *filename, int append){
  int i;
  int openErr = 0;
  if(append!=1)
    rawdata.numSpOpened=0;
  int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
  if(numSp > 0){ //see read_data.c
      rawdata.openedSp = 1;
      //set comments for spectra just opened
      for (i = rawdata.numSpOpened; i < (rawdata.numSpOpened+numSp); i++){
        snprintf(rawdata.histComment[i],256,"Spectrum %i of %s",i-rawdata.numSpOpened,basename((char*)filename));
        //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
      }
      rawdata.numSpOpened += numSp;
      //select the first non-empty spectrum by default
      int sel = getFirstNonemptySpectrum(rawdata.numSpOpened);
      if(sel >=0){
        drawing.multiPlots[0] = sel;
        drawing.multiplotMode = 0; //file just opened, disable multiplot
        gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(display_button),TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(zoom_scale),TRUE);
        gtk_label_set_text(bottom_info_text,"");
        if(numSp > 1){
          gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
        }
        //set the range of selectable spectra values
        gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
        gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened - 1);
        gtk_spin_button_set_value(spectrum_selector, sel);
      }else{
        //no spectra with any data in the selected file
        openErr = 2;
      }
    }else if (numSp == -1){
      //too many files opened
      openErr = 3;
    }else{
      //improper file format
      openErr = 1;
    }
  
  if(openErr>0){
    char errMsg[256];
    switch (openErr)
    {
      case 2:
        snprintf(errMsg,256,"All spectrum data in file %s is empty.",filename);
        break;
      case 3:
        snprintf(errMsg,256,"You are trying to open too many files at once.  Maximum number of individual spectra which may be imported is %i.",NSPECT);
        break;
      default:
        snprintf(errMsg,256,"Data in file %s is incorrectly formatted.",filename);
        break;
    }
    printf("ERROR: %s\n",errMsg);
    exit(-1);
  }

}

void on_open_button_clicked(GtkButton *b)
{
  int i,j;
  
  file_open_dialog = gtk_file_chooser_dialog_new ("Open Spectrum File(s)", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_open_dialog), TRUE);
  file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"Spectrum Data (.txt, .mca, .fmca, .spe)");
  gtk_file_filter_add_pattern(file_filter,"*.txt");
  gtk_file_filter_add_pattern(file_filter,"*.mca");
  gtk_file_filter_add_pattern(file_filter,"*.fmca");
  gtk_file_filter_add_pattern(file_filter,"*.spe");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_open_dialog),file_filter);

  int openErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_open_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    rawdata.numSpOpened = 0; //reset the open spectra
    char *filename;
    GSList *file_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_open_dialog));
    for(i=0;i<g_slist_length(file_list);i++){
      filename = g_slist_nth_data(file_list,i);
      int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
      if(numSp > 0){ //see read_data.c
        rawdata.openedSp = 1;
        //set comments for spectra just opened
        for (j = rawdata.numSpOpened; j < (rawdata.numSpOpened+numSp); j++){
          snprintf(rawdata.histComment[j],256,"Spectrum %i of %s",j-rawdata.numSpOpened,basename((char*)filename));
          //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
        }
        rawdata.numSpOpened += numSp;
        //select the first non-empty spectrum by default
        int sel = getFirstNonemptySpectrum(rawdata.numSpOpened);
        if(sel >=0){
          drawing.multiPlots[0] = sel;
          drawing.multiplotMode = 0; //files just opened, disable multiplot
          gui.fittingSp = 0; //files just opened, reset fit state
          gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(display_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(zoom_scale),TRUE);
          gtk_label_set_text(bottom_info_text,"");
          if(numSp > 1){
            gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
          }
          //set the range of selectable spectra values
          gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
          gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened - 1);
          gtk_spin_button_set_value(spectrum_selector, sel);
        }else{
          //no spectra with any data in the selected file
          openErr = 2;
          break;
        }
      }else if (numSp == -1){
        //too many files opened
        openErr = 3;
        break;
      }else{
        //improper file format
        openErr = 1;
        break;
      }
    }

    gtk_widget_destroy(file_open_dialog);

    if(openErr>0){
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error opening spectrum data!");
      char errMsg[256];
      switch (openErr)
      {
        case 2:
          snprintf(errMsg,256,"All spectrum data in file %s is empty.",filename);
          break;
        case 3:
          snprintf(errMsg,256,"You are trying to open too many files at once.  Maximum number of individual spectra which may be imported is %i.",NSPECT);
          break;
        default:
          snprintf(errMsg,256,"Data in file %s is incorrectly formatted.",filename);
          break;
      }
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errMsg);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
    }else{
      rawdata.numFilesOpened = g_slist_length(file_list);
      //set the title of the opened spectrum in the header bar
      if(rawdata.numFilesOpened > 1){
        char headerBarSub[256];
        snprintf(headerBarSub,256,"%i files loaded",rawdata.numFilesOpened);
        gtk_header_bar_set_subtitle(header_bar,headerBarSub);
      }else{
        gtk_header_bar_set_subtitle(header_bar,filename);
      }
    }
    g_slist_free(file_list);
    g_free(filename);
  }else{
    gtk_widget_destroy(file_open_dialog);
  }
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  
  
}


void on_append_button_clicked(GtkButton *b)
{
  int i,j;
  
  file_open_dialog = gtk_file_chooser_dialog_new ("Add More Spectrum File(s)", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_open_dialog), TRUE);
  file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"Spectrum Data (.mca, .fmca, .spe)");
  gtk_file_filter_add_pattern(file_filter,"*.mca");
  gtk_file_filter_add_pattern(file_filter,"*.fmca");
  gtk_file_filter_add_pattern(file_filter,"*.spe");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_open_dialog),file_filter);

  int openErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_open_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    GSList *file_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_open_dialog));
    for(i=0;i<g_slist_length(file_list);i++){
      filename = g_slist_nth_data(file_list,i);
      int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
      if(numSp > 0){ //see read_data.c
        rawdata.openedSp = 1;
        //set comments for spectra just opened
        for (j = rawdata.numSpOpened; j < (rawdata.numSpOpened+numSp); j++){
          snprintf(rawdata.histComment[j],256,"Spectrum %i of %s",j-rawdata.numSpOpened,basename((char*)filename));
          //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
        }
        rawdata.numSpOpened += numSp;
        gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened - 1);
      }else if (numSp == -1){
        //too many files opened
        openErr = 3;
        break;
      }else{
        //improper file format
        openErr = 1;
        break;
      }
    }

    gtk_widget_destroy(file_open_dialog);

    if(openErr>0){
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error opening spectrum data!");
      char errMsg[256];
      switch (openErr)
      {
        case 2:
          snprintf(errMsg,256,"All spectrum data in file %s is empty.",filename);
          break;
        case 3:
          snprintf(errMsg,256,"You are trying to open too many files at once.  Maximum number of individual spectra which may be imported is %i.",NSPECT);
          break;
        default:
          snprintf(errMsg,256,"Data in file %s is incorrectly formatted.",filename);
          break;
      }
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errMsg);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
    }else{
      rawdata.numFilesOpened += g_slist_length(file_list);
      //set the title of the opened spectrum in the header bar
      if(rawdata.numFilesOpened > 1){
        char headerBarSub[256];
        snprintf(headerBarSub,256,"%i files loaded",rawdata.numFilesOpened);
        gtk_header_bar_set_subtitle(header_bar,headerBarSub);
      }else{
        gtk_header_bar_set_subtitle(header_bar,filename);
      }
    }
    g_slist_free(file_list);
    g_free(filename);
  }else{
    gtk_widget_destroy(file_open_dialog);
  }
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  
  
}



void on_display_button_clicked(GtkButton *b)
{
  //gtk_range_set_value(GTK_RANGE(pan_scale),(drawing.xChanFocus*100.0/S32K));
  gtk_range_set_value(GTK_RANGE(contract_scale),drawing.contractFactor);
  gtk_popover_popup(display_popover); //show the popover menu
}

void on_zoom_scale_changed(GtkRange *range, gpointer user_data){
  drawing.zoomLevel = pow(2,gtk_range_get_value(range)); //modify the zoom level
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}
void on_pan_scale_changed(GtkRange *range, gpointer user_data){
  drawing.xChanFocus = (S32K/100.0)*gtk_range_get_value(range); //modify the pan level
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}
void on_contract_scale_changed(GtkRange *range, gpointer user_data){
  int oldContractFactor = drawing.contractFactor;
  drawing.contractFactor = (int)gtk_range_get_value(range); //modify the contraction factor
  if(gui.fittingSp == 3){
    int i;
    //rescale fit (optimization - don't refit until scale is released)
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[6+(3*i)] *= 1.0*drawing.contractFactor/oldContractFactor;
    }
    for(i=0;i<3;i++){
      fitpar.fitParVal[i] *= 1.0*drawing.contractFactor/oldContractFactor;
    }
    gui.deferFit = 1;
  }
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}

void on_contract_scale_button_release(GtkWidget *widget, GdkEvent *event, gpointer user_data){
  if((gui.deferFit)&&(gui.fittingSp == 3)){
    gui.deferFit = 0; //unset the flag
    performGausFit(); //refit
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
  }
}

void on_calibrate_button_clicked(GtkButton *b)
{
  if(calpar.calMode == 1){
    char str[256];
    sprintf(str,"%.3f",calpar.calpar0);
    gtk_entry_set_text(cal_entry_const,str);
    sprintf(str,"%.3f",calpar.calpar1);
    gtk_entry_set_text(cal_entry_lin,str);
    sprintf(str,"%.3f",calpar.calpar2);
    gtk_entry_set_text(cal_entry_quad,str);
    gtk_entry_set_text(cal_entry_unit,calpar.calUnit);
  }
  gtk_window_present(calibrate_window); //show the window
}

void on_cal_par_activate (GtkEntry *entry, gpointer  user_data){
  const gchar *entryText;
  entryText = gtk_entry_get_text(cal_entry_const);
  if(strtod(entryText,NULL)==0.){
    gtk_entry_set_text (cal_entry_const, "");
  }
  entryText = gtk_entry_get_text(cal_entry_lin);
  if(strtod(entryText,NULL)==0.){
    gtk_entry_set_text (cal_entry_lin, "");
  }
  entryText = gtk_entry_get_text(cal_entry_quad);
  if(strtod(entryText,NULL)==0.){
    gtk_entry_set_text (cal_entry_quad, "");
  }
  g_free((gchar*)entryText);
}

void on_calibrate_ok_button_clicked(GtkButton *b)
{
  //apply settings here!
  strcpy(calpar.calUnit,gtk_entry_get_text(cal_entry_unit));
  if(strcmp(calpar.calUnit,"")==0){
    strcpy(calpar.calUnit,"Cal. Units");
  }
  calpar.calpar0 = (float)strtod(gtk_entry_get_text(cal_entry_const),NULL);
  calpar.calpar1 = (float)strtod(gtk_entry_get_text(cal_entry_lin),NULL);
  calpar.calpar2 = (float)strtod(gtk_entry_get_text(cal_entry_quad),NULL);
  if(!((calpar.calpar0 == 0.0)&&(calpar.calpar1==0.0)&&(calpar.calpar2==0.0))){
    //not all calibration parameters are zero, calibration is valid
    calpar.calMode=1;
    //printf("Calibration parameters: %f %f %f, drawing.calMode: %i, calpar.calUnit: %s\n",calpar.calpar0,calpar.calpar1,calpar.calpar2,drawing.calMode,drawing.calUnit);
    updateConfigFile();
    gtk_widget_hide(GTK_WIDGET(calibrate_window)); //close the calibration window
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }else{
    calpar.calMode=0;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *message_dialog = gtk_message_dialog_new(calibrate_window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid calibration!");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"At least one of the calibration parameters must be non-zero.");
    gtk_dialog_run (GTK_DIALOG (message_dialog));
    gtk_widget_destroy (message_dialog);
    //gtk_window_set_transient_for(message_dialog, calibrate_window); //center massage dialog on calibrate window
  }
  
}

void on_multiplot_cell_toggled(GtkCellRendererToggle *c, gchar *path_string){
  GtkTreeIter iter;
  gboolean val = FALSE;
  GtkTreeModel *model = gtk_tree_view_get_model(multiplot_tree_view);
  gtk_tree_model_get_iter_from_string(model, &iter, path_string);
  gtk_tree_model_get(model,&iter,1,&val,-1); //get the boolean value
  if(val==FALSE){
    val=TRUE;
  }else{
    val=FALSE;
  }
  gtk_list_store_set(multiplot_liststore,&iter,1,val,-1); //set the boolean value (change checkbox value)
  //printf("toggled %i\n",val);
  //gtk_widget_queue_draw(GTK_WIDGET(multiplot_window));
}

void on_multiplot_button_clicked(GtkButton *b)
{
  GtkTreeIter iter;
  int i;
  gtk_list_store_clear(multiplot_liststore);
  for(i=0;i<rawdata.numSpOpened;i++){
    gtk_list_store_append(multiplot_liststore,&iter);
    gtk_list_store_set(multiplot_liststore, &iter, 0, rawdata.histComment[i], -1);
    gtk_list_store_set(multiplot_liststore, &iter, 1, FALSE, -1);
    gtk_list_store_set(multiplot_liststore, &iter, 2, i, -1);
  }
  
  gtk_window_present(multiplot_window); //show the window
}

void on_multiplot_ok_button_clicked(GtkButton *b)
{
  GtkTreeIter iter;
  gboolean readingTreeModel;
  gboolean val = FALSE;
  int spInd = 0;
  int selectedSpCount = 0;
  GtkTreeModel *model = gtk_tree_view_get_model(multiplot_tree_view);
  readingTreeModel = gtk_tree_model_get_iter_first (model, &iter);
  while (readingTreeModel)
  {
    gtk_tree_model_get(model,&iter,1,&val,2,&spInd,-1); //get whether the spectrum is selected and the spectrum index
    if((spInd < NSPECT)&&(selectedSpCount<NSPECT)){
      if(val==TRUE){
        drawing.multiPlots[selectedSpCount]=spInd;
        selectedSpCount++;
      }
    }
    readingTreeModel = gtk_tree_model_iter_next (model, &iter);
  }
  drawing.numMultiplotSp = selectedSpCount;
  drawing.multiplotMode = gtk_combo_box_get_active(GTK_COMBO_BOX(multiplot_mode_combobox));
  if((drawing.numMultiplotSp > MAX_DISP_SP)||(drawing.numMultiplotSp < 2)){

    //reset default values, in case the multiplot window is closed
    drawing.multiplotMode = 0;
    drawing.multiPlots[0] = 0; 
    drawing.numMultiplotSp = 1;
    gtk_spin_button_set_value(spectrum_selector, 0);

    //show an error dialog
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *message_dialog = gtk_message_dialog_new(multiplot_window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid selection!");
    if(drawing.multiplotMode < 0)
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Please select a plotting mode.");
    if(drawing.numMultiplotSp < 2)
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Please select at least two spectra to plot together.");
    if(drawing.numMultiplotSp > MAX_DISP_SP){
      char errStr[256];
      snprintf(errStr,256,"The maximum number of spectra that may be plotted at once is %i.",MAX_DISP_SP);
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errStr);
    }
    gtk_dialog_run (GTK_DIALOG (message_dialog));
    gtk_widget_destroy (message_dialog);
  }else{

    //set drawing mode
    drawing.multiplotMode++; //value of 0 means no multiplot
    
    //handle fitting
    if(drawing.multiplotMode > 1){
      gui.fittingSp = 0; //clear any fits being displayed
    }else if(gui.fittingSp == 3){
      performGausFit(); //refit
    }
    
    printf("Number of spectra selected for plotting: %i.  Selected spectra: ", drawing.numMultiplotSp);
    int i;
    for(i=0;i<drawing.numMultiplotSp;i++){
      printf("%i ",drawing.multiPlots[i]);
    }
    printf(", multiplot mode: %i\n",drawing.multiplotMode);
    if(drawing.multiplotMode >= 2){
      //multiple spectra displayed simultaneously, cannot fit
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
    }else{
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
    }
    gtk_widget_hide(GTK_WIDGET(multiplot_window)); //close the multiplot window
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
  }
  
}

void on_spectrum_selector_changed(GtkSpinButton *spin_button, gpointer user_data)
{
  drawing.multiPlots[0] = gtk_spin_button_get_value_as_int(spin_button);
  drawing.multiplotMode = 0;//unset multiplot, if it is being used
  
  //handle fitting
  if(gui.fittingSp == 3){
    performGausFit(); //refit
  }

  gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE); //no multiplot, therefore can fit
  //printf("Set selected spectrum to %i\n",dispSp);
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
}

void on_fit_button_clicked(GtkButton *b)
{
  gui.fittingSp = 1;
  memset(fitpar.fitParVal,0,sizeof(fitpar.fitParVal));
  //set default values
  fitpar.fitStartCh = -1;
  fitpar.fitEndCh = -1;
  fitpar.numFitPeaks = 0;
  //update widgets
  gtk_widget_set_sensitive(GTK_WIDGET(open_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
  gtk_label_set_text(overlay_info_label,"Right-click to set fit region lower and upper bounds.");
  gtk_widget_show(GTK_WIDGET(overlay_info_bar));
  gtk_info_bar_set_revealed(overlay_info_bar, TRUE);
}

void on_fit_fit_button_clicked(GtkButton *b)
{
  performGausFit(); //perform the fit
  gui.fittingSp = 3;
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  //update widgets
  gtk_widget_set_sensitive(GTK_WIDGET(open_button),TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
  gtk_info_bar_set_revealed(overlay_info_bar, FALSE);
  gtk_widget_hide(GTK_WIDGET(overlay_info_bar));
}

void on_fit_cancel_button_clicked(GtkButton *b)
{
  gui.fittingSp = 0;
  //update widgets
  gtk_widget_set_sensitive(GTK_WIDGET(open_button),TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
  gtk_info_bar_set_revealed(overlay_info_bar, FALSE);
  gtk_widget_hide(GTK_WIDGET(overlay_info_bar));
}

void on_toggle_discard_empty(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    rawdata.dropEmptySpectra=1;
  else
    rawdata.dropEmptySpectra=0;
}

void on_preferences_button_clicked(GtkButton *b)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(discard_empty_checkbutton),rawdata.dropEmptySpectra);
  gtk_window_present(preferences_window); //show the window
}

void on_preferences_apply_button_clicked(GtkButton *b)
{
  updateConfigFile();
  gtk_widget_hide(GTK_WIDGET(preferences_window)); //close the multiplot window
}

void on_about_button_clicked(GtkButton *b)
{
  gtk_window_present(GTK_WINDOW(about_dialog)); //show the window
}

int main(int argc, char *argv[])
{
  
  gtk_init(&argc, &argv); //initialize Gtk

  builder = gtk_builder_new_from_resource("/resources/jf3.glade"); //get UI layout from glade XML file

  //windows
  window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); //quit the program when closing the window
  calibrate_window = GTK_WINDOW(gtk_builder_get_object(builder, "calibration_window"));
  gtk_window_set_transient_for(calibrate_window, window); //center calibrate window on main window
  multiplot_window = GTK_WINDOW(gtk_builder_get_object(builder, "multiplot_window"));
  gtk_window_set_transient_for(multiplot_window, window); //center multiplot window on main window
  preferences_window = GTK_WINDOW(gtk_builder_get_object(builder, "preferences_window"));
  gtk_window_set_transient_for(preferences_window, window); //center preferences window on main window
  about_dialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "about_dialog"));
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), window); //center about dialog on main window
  
  //header bar
  header_bar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));

  //menus
  display_popover = GTK_POPOVER(gtk_builder_get_object(builder, "display_popover"));

  //main window UI elements
  open_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_button"));
  append_button = GTK_WIDGET(gtk_builder_get_object(builder, "append_button"));
  calibrate_button = GTK_WIDGET(gtk_builder_get_object(builder, "calibrate_button"));
  fit_button = GTK_WIDGET(gtk_builder_get_object(builder, "fit_button"));
  display_button = GTK_WIDGET(gtk_builder_get_object(builder, "display_button"));
  calibrate_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "options_ok_button"));
  spectrum_selector = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spectrumselector"));
  spectrum_selector_adjustment = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "spectrum_selector_adjustment"));
  autoscale_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autoscalebutton"));
  cursor_draw_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "cursordrawbutton"));
  spectrum_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "spectrumdrawingarea"));
  spectrum_drag_gesture = gtk_gesture_drag_new(spectrum_drawing_area); //without this, cannot click away from menus onto the drawing area, needs further investigation
  zoom_scale = GTK_SCALE(gtk_builder_get_object(builder, "zoom_scale"));
  about_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "about_button"));
  bottom_info_text = GTK_LABEL(gtk_builder_get_object(builder, "bottom_info_text"));
  overlay_info_bar = GTK_INFO_BAR(gtk_builder_get_object(builder, "overlay_info_bar"));
  overlay_info_label = GTK_LABEL(gtk_builder_get_object(builder, "overlay_info_label"));
  fit_cancel_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_cancel_button"));
  fit_fit_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_fit_button"));

  //calibration window UI elements
  cal_entry_unit = GTK_ENTRY(gtk_builder_get_object(builder, "calibration_unit_entry"));
  cal_entry_const = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_const"));
  cal_entry_lin = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_lin"));
  cal_entry_quad = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_quad"));

  //multiplot window UI elements
  multiplot_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "multiplot_ok_button"));
  multiplot_liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "multiplot_liststore"));
  multiplot_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "multiplot_tree_view"));
  multiplot_column1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column1"));
  multiplot_column2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column2"));
  multiplot_cr1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr1"));
  multiplot_cr2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr2"));
  multiplot_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "multiplot_mode_combobox"));

  //preferences window UI elements
  preferences_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "preferences_button"));
  discard_empty_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "discard_empty_checkbutton"));
  preferences_apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "preferences_apply_button"));

  //display menu UI elements
  multiplot_button = GTK_WIDGET(gtk_builder_get_object(builder, "multiplot_button"));
  contract_scale = GTK_SCALE(gtk_builder_get_object(builder, "contract_scale"));


  //connect signals
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "draw", G_CALLBACK (drawSpectrumArea), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "scroll-event", G_CALLBACK (on_spectrum_scroll), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "motion-notify-event", G_CALLBACK (on_spectrum_cursor_motion), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "button-press-event", G_CALLBACK (on_spectrum_click), NULL);
  g_signal_connect (G_OBJECT (open_button), "clicked", G_CALLBACK (on_open_button_clicked), NULL);
  g_signal_connect (G_OBJECT (append_button), "clicked", G_CALLBACK (on_append_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_button), "clicked", G_CALLBACK (on_calibrate_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_button), "clicked", G_CALLBACK (on_multiplot_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_button), "clicked", G_CALLBACK (on_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_fit_button), "clicked", G_CALLBACK (on_fit_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_cancel_button), "clicked", G_CALLBACK (on_fit_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (display_button), "clicked", G_CALLBACK (on_display_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_ok_button), "clicked", G_CALLBACK (on_calibrate_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (spectrum_selector), "value-changed", G_CALLBACK (on_spectrum_selector_changed), NULL);
  g_signal_connect (G_OBJECT (autoscale_button), "toggled", G_CALLBACK (on_toggle_autoscale), NULL);
  g_signal_connect (G_OBJECT (cursor_draw_button), "toggled", G_CALLBACK (on_toggle_cursor), NULL);
  g_signal_connect (G_OBJECT (discard_empty_checkbutton), "toggled", G_CALLBACK (on_toggle_discard_empty), NULL);
  g_signal_connect (G_OBJECT (preferences_apply_button), "clicked", G_CALLBACK (on_preferences_apply_button_clicked), NULL);
  g_signal_connect (G_OBJECT (preferences_button), "clicked", G_CALLBACK (on_preferences_button_clicked), NULL);
  gtk_widget_set_events(spectrum_drawing_area, gtk_widget_get_events (spectrum_drawing_area) | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK); //allow mouse scrolling over the drawing area
  g_signal_connect (G_OBJECT (cal_entry_const), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_lin), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_quad), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (zoom_scale), "value-changed", G_CALLBACK (on_zoom_scale_changed), NULL);
  g_signal_connect (G_OBJECT (contract_scale), "value-changed", G_CALLBACK (on_contract_scale_changed), NULL);
  g_signal_connect (G_OBJECT (contract_scale), "button-release-event", G_CALLBACK (on_contract_scale_button_release), NULL);
  gtk_widget_set_events(GTK_WIDGET(contract_scale), gtk_widget_get_events (GTK_WIDGET(contract_scale)) | GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (G_OBJECT (about_button), "clicked", G_CALLBACK (on_about_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_ok_button), "clicked", G_CALLBACK (on_multiplot_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_cr2), "toggled", G_CALLBACK (on_multiplot_cell_toggled), NULL);
  g_signal_connect (G_OBJECT (calibrate_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (multiplot_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (preferences_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (about_dialog), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button

  //set attributes
  gtk_tree_view_column_add_attribute(multiplot_column2,multiplot_cr2, "active",1);

  //set default values
  rawdata.openedSp = 0;
  rawdata.numFilesOpened = 0;
  drawing.lowerLimit = 0;
  drawing.upperLimit = S32K - 1;
  drawing.scaleLevelMax[0] = 0.0;
  drawing.scaleLevelMin[0] = 0.0;
  drawing.xChanFocus = 0;
  drawing.zoomLevel = 1.0;
  drawing.contractFactor = 1;
  drawing.autoScale = 1;
  calpar.calMode = 0;
  rawdata.dropEmptySpectra = 1;
  rawdata.numSpOpened = 0;
  drawing.multiplotMode = 0;
  drawing.numMultiplotSp = 1;
  drawing.spColors[0] = 0.8; drawing.spColors[1] = 0.0; drawing.spColors[2] = 0.0;    //RGB values for color 1
  drawing.spColors[3] = 0.0; drawing.spColors[4] = 0.0; drawing.spColors[5] = 0.8;    //RGB values for color 2
  drawing.spColors[6] = 0.0; drawing.spColors[7] = 0.8; drawing.spColors[8] = 0.0;    //RGB values for color 3
  drawing.spColors[9] = 0.0; drawing.spColors[10] = 0.8; drawing.spColors[11] = 0.8;  //RGB values for color 4
  drawing.spColors[12] = 0.7; drawing.spColors[13] = 0.7; drawing.spColors[14] = 0.0; //RGB values for color 5
  drawing.spColors[15] = 0.8; drawing.spColors[16] = 0.0; drawing.spColors[17] = 0.8; //RGB values for color 6
  drawing.spColors[18] = 0.2; drawing.spColors[19] = 0.0; drawing.spColors[20] = 0.0; //RGB values for color 7
  drawing.spColors[21] = 0.0; drawing.spColors[22] = 0.0; drawing.spColors[23] = 0.2; //RGB values for color 8
  drawing.spColors[24] = 0.0; drawing.spColors[25] = 0.2; drawing.spColors[26] = 0.0; //RGB values for color 9
  drawing.spColors[27] = 0.0; drawing.spColors[28] = 0.2; drawing.spColors[29] = 0.2; //RGB values for color 10
  drawing.spColors[30] = 0.2; drawing.spColors[31] = 0.2; drawing.spColors[32] = 0.0; //RGB values for color 11
  drawing.spColors[33] = 0.2; drawing.spColors[34] = 0.0; drawing.spColors[35] = 0.8; //RGB values for color 12
  gui.fittingSp = 0;
  gui.deferFit = 0;
  gui.draggingSp = 0;
  gui.drawSpCursor = -1; //disabled by default
  fitpar.fitStartCh = -1;
  fitpar.fitEndCh = -1;
  fitpar.numFitPeaks = 0;

  gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
  gtk_adjustment_set_upper(spectrum_selector_adjustment, 0);
  gtk_label_set_text(bottom_info_text,"No spectrum loaded.");
  
  //'gray out' widgets that can't be used yet
  gtk_widget_set_sensitive(GTK_WIDGET(append_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(display_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(zoom_scale),FALSE);

  //hide widgets that can't be seen yet
  gtk_info_bar_set_revealed(overlay_info_bar,FALSE);
  gtk_widget_hide(GTK_WIDGET(overlay_info_bar));

  //setup UI element appearance at startup
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoscale_button), drawing.autoScale);

  //open a file if requested from the command line
  if(argc > 1){
    int i;
    openSingleFile(argv[1],0);
    for(i=2;i<argc;i++){
      openSingleFile(argv[i],1);
    }
    //set headerbar info for opened files
    if(argc>2){
      char headerBarSub[256];
      rawdata.numFilesOpened = argc-1;
      snprintf(headerBarSub,256,"%i files loaded",rawdata.numFilesOpened);
      gtk_header_bar_set_subtitle(header_bar,headerBarSub);
    }else{
      gtk_header_bar_set_subtitle(header_bar,argv[1]);
    }
    gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
  }

  //setup config file
  char dirPath[256];
  strcpy(dirPath,"");
	strcat(dirPath,getenv("HOME"));
	strcat(dirPath,"/.config/jf3");
  struct stat st = {0};
  if (stat(dirPath, &st) == -1) {
    //config directory doesn't exist, make it
    mkdir(dirPath, 0700);
    printf("Setup configuration file directory: %s\n",dirPath);
  }
  FILE *configFile;
  strcat(dirPath,"/jf3.conf");
  if ((configFile = fopen(dirPath, "r")) == NULL){ //open the config file
		printf("Creating configuration file at: %s\n", dirPath);
    configFile = fopen(dirPath, "w");
    if(configFile != NULL){
      writeConfigFile(configFile); //write the default configuration values
      fclose(configFile);
    }else{
      printf("WARNING: Unable to create configuration file, falling back to default values.\n");
    }
	}else{
    readConfigFile(configFile);
    fclose(configFile);
  }

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main();              //Gtk main loop

  return 0;
}

