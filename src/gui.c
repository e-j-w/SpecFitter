/* J. Williams, 2020 */

//File contains routines and callbacks for dealing with GTK and
//the various UI elements used in the program.  Initialization
//routines at the bottom of the file.

void showPreferences(int page){
  gtk_notebook_set_current_page(preferences_notebook,page);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(discard_empty_checkbutton),rawdata.dropEmptySpectra);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bin_errors_checkbutton),guiglobals.showBinErrors);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(round_errors_checkbutton),guiglobals.roundErrors);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dark_theme_checkbutton),guiglobals.preferDarkTheme);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(animation_checkbutton),guiglobals.useZoomAnimations);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autozoom_checkbutton),guiglobals.autoZoom);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spectrum_label_checkbutton),guiglobals.drawSpLabels);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(spectrum_comment_checkbutton),guiglobals.drawSpComments);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(relative_widths_checkbutton),fitpar.fixRelativeWidths);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(popup_results_checkbutton),guiglobals.popupFitResults);
  gtk_combo_box_set_active(GTK_COMBO_BOX(weight_mode_combobox),fitpar.weightMode);
  gtk_window_present(preferences_window); //show the window
}

void setupUITheme(){
  //set whether dark theme is preferred
  g_object_set(gtk_settings_get_default(),"gtk-application-prefer-dark-theme", guiglobals.preferDarkTheme, NULL);
  if(guiglobals.preferDarkTheme){
    gtk_image_set_from_pixbuf(display_button_icon, spIconPixbufDark);
    gtk_image_set_from_pixbuf(display_button_icon1, spIconPixbufDark);
    gtk_image_set_from_pixbuf(display_button_icon2, spIconPixbufDark);
    gtk_image_set_from_pixbuf(no_sp_image, spIconPixbufDark);
  }else{
    gtk_image_set_from_pixbuf(display_button_icon, spIconPixbuf);
    gtk_image_set_from_pixbuf(display_button_icon1, spIconPixbuf);
    gtk_image_set_from_pixbuf(display_button_icon2, spIconPixbuf);
    gtk_image_set_from_pixbuf(no_sp_image, spIconPixbuf);
  }
}

//function for opening a single file without UI (ie. from the command line)
//if append=1, append this file to already opened files
void openSingleFile(char *filename, int append){
  int i;
  int openErr = 0;
  if(append!=1){
    rawdata.numSpOpened=0;
    rawdata.numChComments=0;
  }
  int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
  if(numSp > 0){ //see read_data.c
    rawdata.openedSp = 1;
    //set comments and scaling for spectra just opened
    for (i = rawdata.numSpOpened; i < (rawdata.numSpOpened+numSp); i++){
      snprintf(rawdata.histComment[i],256,"Spectrum %i of %s",i-rawdata.numSpOpened+1,basename((char*)filename));
      //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
      drawing.scaleFactor[i] = 1.00;
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
      gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
      gtk_widget_set_sensitive(GTK_WIDGET(to_save_menu_button),TRUE);
      gtk_label_set_text(bottom_info_text,"");
      gtk_widget_hide(GTK_WIDGET(no_sp_box));
      if(rawdata.numSpOpened > 1){
        gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
      }
      //set the range of selectable spectra values
      gtk_adjustment_set_lower(spectrum_selector_adjustment, 1);
      gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened);
      gtk_spin_button_set_value(spectrum_selector, sel+1);
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

  //autozoom if needed
  if(guiglobals.autoZoom){
    autoZoom();
    gtk_range_set_value(GTK_RANGE(zoom_scale),log2(drawing.zoomLevel));
  }

}

void on_open_button_clicked(GtkButton *b)
{
  int i,j;
  
  file_open_dialog = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Open Spectrum File(s)", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL));
  gtk_file_chooser_set_select_multiple(file_open_dialog, TRUE);
  file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"Spectrum Data (.jf3, .txt, .mca, .fmca, .spe, .C)");
  gtk_file_filter_add_pattern(file_filter,"*.txt");
  gtk_file_filter_add_pattern(file_filter,"*.mca");
  gtk_file_filter_add_pattern(file_filter,"*.fmca");
  gtk_file_filter_add_pattern(file_filter,"*.spe");
  gtk_file_filter_add_pattern(file_filter,"*.C");
  gtk_file_filter_add_pattern(file_filter,"*.jf3");
  gtk_file_chooser_add_filter(file_open_dialog,file_filter);

  int openErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_open_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    rawdata.numSpOpened = 0; //reset the open spectra
    rawdata.numChComments = 0; //reset the number of comments
    char *filename = NULL;
    GSList *file_list = gtk_file_chooser_get_filenames(file_open_dialog);
    for(i=0;i<g_slist_length(file_list);i++){
      filename = g_slist_nth_data(file_list,i);
      int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
      if(numSp > 0){ //see read_data.c
        rawdata.openedSp = 1;
        //set comments for spectra just opened
        for (j = rawdata.numSpOpened; j < (rawdata.numSpOpened+numSp); j++){
          snprintf(rawdata.histComment[j],256,"Spectrum %i of %s",j-rawdata.numSpOpened+1,basename((char*)filename));
          //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
          drawing.scaleFactor[j] = 1.00;
        }
        rawdata.numSpOpened += numSp;
        //select the first non-empty spectrum by default
        int sel = getFirstNonemptySpectrum(rawdata.numSpOpened);
        if(sel >=0){
          drawing.multiPlots[0] = sel;
          drawing.multiplotMode = 0; //files just opened, disable multiplot
          guiglobals.fittingSp = 0; //files just opened, reset fit state
          gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(display_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(zoom_scale),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(to_save_menu_button),TRUE);
          gtk_label_set_text(bottom_info_text,"");
          gtk_widget_hide(GTK_WIDGET(no_sp_box));
          if(rawdata.numSpOpened > 1){
            gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
          }
          //set the range of selectable spectra values
          gtk_adjustment_set_lower(spectrum_selector_adjustment, 1);
          gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened);
          gtk_spin_button_set_value(spectrum_selector, sel+1);
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

    gtk_widget_destroy(GTK_WIDGET(file_open_dialog));

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
    gtk_widget_destroy(GTK_WIDGET(file_open_dialog));
  }
  
  //autozoom if needed
  if(guiglobals.autoZoom){
    autoZoom();
    gtk_range_set_value(GTK_RANGE(zoom_scale),log2(drawing.zoomLevel));
  }
  
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  
}


void on_append_button_clicked(GtkButton *b)
{

  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }

  int i,j;
  
  file_open_dialog = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new ("Add More Spectrum File(s)", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL));
  gtk_file_chooser_set_select_multiple(file_open_dialog, TRUE);
  file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"Spectrum Data (.jf3, .txt, .mca, .fmca, .spe, .C)");
  gtk_file_filter_add_pattern(file_filter,"*.txt");
  gtk_file_filter_add_pattern(file_filter,"*.mca");
  gtk_file_filter_add_pattern(file_filter,"*.fmca");
  gtk_file_filter_add_pattern(file_filter,"*.spe");
  gtk_file_filter_add_pattern(file_filter,"*.C");
  gtk_file_filter_add_pattern(file_filter,"*.jf3");
  gtk_file_chooser_add_filter(file_open_dialog,file_filter);

  int openErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_open_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename = NULL;
    GSList *file_list = gtk_file_chooser_get_filenames(file_open_dialog);
    for(i=0;i<g_slist_length(file_list);i++){
      filename = g_slist_nth_data(file_list,i);
      int numSp = readSpectrumDataFile(filename,rawdata.hist,rawdata.numSpOpened);
      if(numSp > 0){ //see read_data.c
        rawdata.openedSp = 1;
        //set comments for spectra just opened
        for (j = rawdata.numSpOpened; j < (rawdata.numSpOpened+numSp); j++){
          snprintf(rawdata.histComment[j],256,"Spectrum %i of %s",j-rawdata.numSpOpened+1,basename((char*)filename));
          //printf("Comment %i: %s\n",j,rawdata.histComment[j]);
          drawing.scaleFactor[j] = 1.00;
        }
        rawdata.numSpOpened += numSp;
        if(rawdata.numSpOpened > 1){
          gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
        }
        gtk_adjustment_set_upper(spectrum_selector_adjustment, rawdata.numSpOpened);
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

    gtk_widget_destroy(GTK_WIDGET(file_open_dialog));

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
    gtk_widget_destroy(GTK_WIDGET(file_open_dialog));
  }
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  
}

void on_save_button_clicked(GtkButton *b)
{
  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }

  file_save_dialog = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new ("Save Spectrum Data", window, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL));
  gtk_file_chooser_set_select_multiple(file_save_dialog, FALSE);

  int saveErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_save_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *fn = NULL;
    char *tok, fileName[256];
    fn = gtk_file_chooser_get_filename(file_save_dialog);
    tok = strtok (fn,".");
    strncpy(fileName,tok,255);
    //save as a .jf3 file by default
    strncat(fileName,".jf3",255);
    //write file
    saveErr = writeJF3(fileName, rawdata.hist);

    gtk_widget_destroy(GTK_WIDGET(file_save_dialog));

    if(saveErr>0){
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error saving spectrum data!");
      char errMsg[256];
      switch (saveErr)
      {
        case 1:
          snprintf(errMsg,256,"Error writing to file %s.",fileName);
          break;
        default:
          snprintf(errMsg,256,"Unknown error saving spectrum data.");
          break;
      }
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errMsg);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
    }else{
      //update the status bar
      char saveMsg[256];
      snprintf(saveMsg,256,"Saved data to file %s.",fileName);
      gtk_label_set_text(bottom_info_text,saveMsg);
    }
    g_free(fn);
  }else{
    gtk_widget_destroy(GTK_WIDGET(file_save_dialog));
  }
  
}

void on_save_text_button_clicked(GtkButton *b)
{
  int i;
  guiglobals.exportFileType = 0; //exporting to text format
  gtk_label_set_text(export_description_label,"Export to .txt format:");
  gtk_widget_hide(GTK_WIDGET(export_note_label));
  gtk_combo_box_text_remove_all(export_mode_combobox);
  gtk_combo_box_text_insert(export_mode_combobox,0,NULL,"All spectra (multiple columns)");
  for(i=0;i<rawdata.numSpOpened;i++){
    gtk_combo_box_text_insert(export_mode_combobox,i+1,NULL,rawdata.histComment[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(export_mode_combobox),0); //set the default entry
  gtk_window_present(export_options_window); //show the window
}

void on_save_radware_button_clicked(GtkButton *b)
{
  int i;
  guiglobals.exportFileType = 1; //exporting to radware format
  gtk_label_set_text(export_description_label,"Export to .spe format:");
  if(rawdata.numChComments > 0)
    gtk_label_set_text(export_note_label,"NOTE: Exported data will be truncated to the first 4096 bins.\nComments will not be exported (incomatible with .spe format).");
  else
    gtk_label_set_text(export_note_label,"NOTE: Exported data will be truncated to the first 4096 bins.");
  gtk_widget_show(GTK_WIDGET(export_note_label));
  gtk_combo_box_text_remove_all(export_mode_combobox);
  gtk_combo_box_text_insert(export_mode_combobox,0,NULL,"All spectra (separate files)");
  for(i=0;i<rawdata.numSpOpened;i++){
    gtk_combo_box_text_insert(export_mode_combobox,i+1,NULL,rawdata.histComment[i]);
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(export_mode_combobox),0); //set the default entry
  gtk_window_present(export_options_window); //show the window
}

void on_export_save_button_clicked(GtkButton *b)
{
  //get export settings
  int exportMode = gtk_combo_box_get_active(GTK_COMBO_BOX(export_mode_combobox));
  int rebin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(export_rebin_checkbutton));

  file_save_dialog = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new ("Export Spectrum Data", window, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Export", GTK_RESPONSE_ACCEPT, NULL));
  gtk_file_chooser_set_select_multiple(file_save_dialog, FALSE);

  int saveErr = 0; //to track if there are any errors when opening spectra
  if (gtk_dialog_run(GTK_DIALOG(file_save_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    
    char *fn = NULL;
    char *tok, fileName[256];
    fn = gtk_file_chooser_get_filename(file_save_dialog);
    tok = strtok (fn,".");
    strncpy(fileName,tok,255);

    //write file
    switch (guiglobals.exportFileType)
    {
      case 1:
        //radware
        saveErr = exportSPE(fileName, exportMode, rebin);
        break;
      case 0:
      default:
        //text
        saveErr = exportTXT(fileName, exportMode, rebin);
        break;
    }

    gtk_widget_destroy(GTK_WIDGET(file_save_dialog));

    if(saveErr>0){
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error exporting spectrum data!");
      char errMsg[256];
      switch (saveErr)
      {
        case 2:
          snprintf(errMsg,256,"Error processing spectrum data.");
          break;
        case 1:
          snprintf(errMsg,256,"Error writing to file.");
          break;
        default:
          snprintf(errMsg,256,"Unknown error exporting spectrum data.");
          break;
      }
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errMsg);
      gtk_dialog_run (GTK_DIALOG (message_dialog));
      gtk_widget_destroy (message_dialog);
    }else{
      //update the status bar
      char saveMsg[256];
      snprintf(saveMsg,256,"Successfully exported data.");
      gtk_label_set_text(bottom_info_text,saveMsg);
    }
    g_free(fn);
  }else{
    gtk_widget_destroy(GTK_WIDGET(file_save_dialog));
  }

  gtk_widget_hide(GTK_WIDGET(export_options_window)); //hide the window
}


void on_display_button_clicked(GtkButton *b)
{
  //gtk_range_set_value(GTK_RANGE(pan_scale),(drawing.xChanFocus*100.0/S32K));
  if(drawing.logScale){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logscale_button),TRUE);
  }else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(logscale_button),FALSE);
  }
  gtk_range_set_value(GTK_RANGE(contract_scale),drawing.contractFactor);
  gtk_popover_popup(display_popover); //show the popover menu
}

void on_zoom_scale_changed(GtkRange *range, gpointer user_data){
  drawing.zoomLevel = pow(2,gtk_range_get_value(range)); //modify the zoom level
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}
void on_contract_scale_changed(GtkRange *range, gpointer user_data){
  int oldContractFactor = drawing.contractFactor;
  drawing.contractFactor = (int)gtk_range_get_value(range); //modify the contraction factor
  if(guiglobals.fittingSp == 5){
    int i;
    //rescale fit (optimization - don't refit)
    for(i=0;i<fitpar.numFitPeaks;i++){
      fitpar.fitParVal[6+(3*i)] *= 1.0*drawing.contractFactor/oldContractFactor;
    }
    for(i=0;i<3;i++){
      fitpar.fitParVal[i] *= 1.0*drawing.contractFactor/oldContractFactor;
    }
  }
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
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
    gtk_entry_set_text(cal_entry_y_axis,calpar.calYUnit);
    gtk_widget_set_sensitive(GTK_WIDGET(remove_calibration_button),TRUE);
  }else{
    gtk_widget_set_sensitive(GTK_WIDGET(remove_calibration_button),FALSE);
  }
  gtk_window_present(calibrate_window); //show the window
}

void on_cal_par_activate (GtkEntry *entry, gpointer user_data){
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
  strncpy(calpar.calUnit,gtk_entry_get_text(cal_entry_unit),12);
  if(strcmp(calpar.calUnit,"")==0){
    strncpy(calpar.calUnit,"Cal. Units",12);
  }
  strncpy(calpar.calYUnit,gtk_entry_get_text(cal_entry_y_axis),28);
  if(strcmp(calpar.calYUnit,"")==0){
    strncpy(calpar.calYUnit,"Value",28);
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

void on_remove_calibration_button_clicked(GtkButton *b)
{
  calpar.calMode=0;
  //printf("Calibration parameters: %f %f %f, drawing.calMode: %i, calpar.calUnit: %s\n",calpar.calpar0,calpar.calpar1,calpar.calpar2,drawing.calMode,drawing.calUnit);
  updateConfigFile();
  gtk_widget_hide(GTK_WIDGET(calibrate_window)); //close the calibration window
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  
}


void on_comment_entry_changed(GtkEntry *entry, gpointer user_data)
{
  int len = gtk_entry_get_text_length(entry);
  if(len==0){
    gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
  }else{
    gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),TRUE);
  }
}
void on_comment_ok_button_clicked(GtkButton *b)
{
  if(guiglobals.commentEditInd == -1){
    //making a new comment
    if(strcmp(gtk_entry_get_text(comment_entry),"")!=0){
      strncpy(rawdata.chanComment[(int)rawdata.numChComments],gtk_entry_get_text(comment_entry),256);
      rawdata.numChComments++;
    }
  }else{
    //editing an existing comment
    if(guiglobals.commentEditInd < NCHCOM){
      strncpy(rawdata.chanComment[guiglobals.commentEditInd],gtk_entry_get_text(comment_entry),256);
    }
  }

  gtk_widget_hide(GTK_WIDGET(comment_window)); //close the comment window
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}
void on_remove_comment_button_clicked(GtkButton *b)
{
  if(guiglobals.commentEditInd == -1){
    printf("WARNING: attempting to delete a comment that doesn't exist!\n");
  }else if(guiglobals.commentEditInd < NCHCOM){
    //shorten the comment array
    int i;
    for(i=guiglobals.commentEditInd+1;i<rawdata.numChComments;i++){
      rawdata.chanCommentCh[i-1]=rawdata.chanCommentCh[i];
      rawdata.chanCommentSp[i-1]=rawdata.chanCommentSp[i];
      rawdata.chanCommentVal[i-1]=rawdata.chanCommentVal[i];
      strncpy(rawdata.chanComment[i-1],rawdata.chanComment[i],256);
    }
    rawdata.numChComments--;
  }
  gtk_widget_hide(GTK_WIDGET(comment_window)); //close the comment window
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
}


void on_multiplot_cell_toggled(GtkCellRendererToggle *c, gchar *path_string){
  int i;
  GtkTreeIter iter;
  gboolean toggleVal = FALSE;
  gboolean val = FALSE;
  GtkTreeModel *model = gtk_tree_view_get_model(multiplot_tree_view);

  //get the value of the cell which was toggled
  gtk_tree_model_get_iter_from_string(model, &iter, path_string);
  gtk_tree_model_get(model,&iter,1,&val,-1); //get the boolean value
  if(val==FALSE){
    toggleVal=TRUE;
  }else{
    toggleVal=FALSE;
  }

  //GList *list;
  GList * rowList = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(multiplot_tree_view),&model);
  //printf("List length: %i, toggleVal: %i\n",g_list_length(rowList),toggleVal);
  int llength = g_list_length(rowList);
  if(llength > 1){
    for(i = 0; i < g_list_length(rowList); i++){
      gtk_tree_model_get_iter(model,&iter,g_list_nth_data(rowList, i));
      gtk_tree_model_get(model,&iter,1,&val,-1); //get the boolean value
      if(toggleVal==TRUE){
        val=TRUE;
      }else{
        val=FALSE;
      }
      gtk_list_store_set(multiplot_liststore,&iter,1,val,-1); //set the boolean value (change checkbox value)
    }
    guiglobals.deferToggleRow = 1;
  }else if(llength == 1){
    gtk_list_store_set(multiplot_liststore,&iter,1,!val,-1); //set the boolean value (change checkbox value)
  }
  g_list_free_full(rowList,(GDestroyNotify)gtk_tree_path_free);
  
  int selectedSpCount = 0;
  int spInd = 0;
  gboolean readingTreeModel = gtk_tree_model_get_iter_first (model, &iter);
  while (readingTreeModel)
  {
    gtk_tree_model_get(model,&iter,1,&val,3,&spInd,-1); //get whether the spectrum is selected and the spectrum index
    if((spInd < NSPECT)&&(selectedSpCount<NSPECT)){
      if(val==TRUE){
        selectedSpCount++;
      }
    }
    readingTreeModel = gtk_tree_model_iter_next (model, &iter);
  }
  if(selectedSpCount > 1){
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_mode_combobox),TRUE);
  }else{
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_mode_combobox),FALSE);
  }
  if(selectedSpCount < 1){
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_ok_button),FALSE);
  }else{
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_ok_button),TRUE);
  }
  //printf("toggled %i\n",val);
  //gtk_widget_queue_draw(GTK_WIDGET(multiplot_window));
}

void on_multiplot_scaling_edited(GtkCellRendererText *c, gchar *path_string, gchar *new_text, gpointer user_data){
  //int i;
  GtkTreeIter iter;
  gdouble val = 1.0;
  gint spInd = -1;
  GtkTreeModel *model = gtk_tree_view_get_model(multiplot_tree_view);

  //get the value of the cell which was toggled
  gtk_tree_model_get_iter_from_string(model, &iter, path_string);
  val = atof(new_text);
  gtk_tree_model_get(model,&iter,3,&spInd,-1); //get the spectrum index
  if((spInd >= 0)&&(spInd < NSPECT)){
    drawing.scaleFactor[spInd] = val;
    printf("Set scaling for spectrum %i to %.2f\n",spInd,drawing.scaleFactor[spInd]);
  }else{
    printf("WARNING: scale factor belongs to invalid spectrum number (%i).\n",spInd);
    return;
  }

  char scaleFacStr[16];
  snprintf(scaleFacStr,16,"%.2f",drawing.scaleFactor[spInd]);
  gtk_list_store_set(multiplot_liststore,&iter,2,scaleFacStr,-1); //set the string

  //gtk_widget_queue_draw(GTK_WIDGET(multiplot_window));
}

void on_multiplot_button_clicked(GtkButton *b)
{

  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }

  GtkTreeIter iter;
  gboolean val = FALSE;
  GtkTreeModel *model = gtk_tree_view_get_model(multiplot_tree_view);
  int i;
  char scaleFacStr[16];
  gtk_list_store_clear(multiplot_liststore);
  for(i=0;i<rawdata.numSpOpened;i++){
    snprintf(scaleFacStr,16,"%.2f",drawing.scaleFactor[i]);
    gtk_list_store_append(multiplot_liststore,&iter);
    gtk_list_store_set(multiplot_liststore, &iter, 0, rawdata.histComment[i], -1);
    gtk_list_store_set(multiplot_liststore, &iter, 1, isSpSelected(i), -1);
    gtk_list_store_set(multiplot_liststore, &iter, 2, scaleFacStr, -1);
    gtk_list_store_set(multiplot_liststore, &iter, 3, i, -1);
  }

  int selectedSpCount = 0;
  int spInd = 0;
  gboolean readingTreeModel = gtk_tree_model_get_iter_first (model, &iter);
 
  while (readingTreeModel)
  {
    gtk_tree_model_get(model,&iter,1,&val,3,&spInd,-1); //get whether the spectrum is selected and the spectrum index
    if((spInd < NSPECT)&&(selectedSpCount<NSPECT)){
      if(val==TRUE){
        selectedSpCount++;
      }
    }
    readingTreeModel = gtk_tree_model_iter_next (model, &iter);
  }
  
  if(selectedSpCount > 1){
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_mode_combobox),TRUE);
  }else{
    gtk_widget_set_sensitive(GTK_WIDGET(multiplot_mode_combobox),FALSE);
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
    gtk_tree_model_get(model,&iter,1,&val,3,&spInd,-1); //get whether the spectrum is selected and the spectrum index
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

  if((drawing.numMultiplotSp > MAX_DISP_SP)||((drawing.multiplotMode < 0))){

    //show an error dialog
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *message_dialog = gtk_message_dialog_new(multiplot_window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid selection!");
    if(drawing.multiplotMode < 0)
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Please select a plotting mode.");
    if(drawing.numMultiplotSp > MAX_DISP_SP){
      char errStr[256];
      snprintf(errStr,256,"The maximum number of spectra\nthat may be plotted at once is %i.",MAX_DISP_SP);
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),errStr);
    }

    //reset default values, in case the multiplot window is closed
    drawing.multiplotMode = 0;
    drawing.multiPlots[0] = 0; 
    drawing.numMultiplotSp = 1;
    gtk_spin_button_set_value(spectrum_selector, drawing.multiPlots[0]+1);

    gtk_dialog_run (GTK_DIALOG (message_dialog));
    gtk_widget_destroy (message_dialog);
  }else{

    //set drawing mode
    if(drawing.numMultiplotSp == 1){
      drawing.multiplotMode = 0;
    }else{
      drawing.multiplotMode++; //value of 0 means no multiplot
    }

    guiglobals.deferSpSelChange = 1;
    gtk_spin_button_set_value(spectrum_selector, drawing.multiPlots[0]+1);
    
    printf("Number of spectra selected for plotting: %i.  Selected spectra: ", drawing.numMultiplotSp);
    int i;
    for(i=0;i<drawing.numMultiplotSp;i++){
      printf("%i ",drawing.multiPlots[i]);
    }
    printf(", multiplot mode: %i\n",drawing.multiplotMode);
    if(drawing.multiplotMode > 1){
      //multiple spectra displayed simultaneously, cannot fit
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
    }else{
      gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE);
    }
    
    //handle fitting
    if(drawing.multiplotMode > 1){
      guiglobals.fittingSp = 0; //clear any fits being displayed
    }else if(guiglobals.fittingSp == 5){
      startGausFit(); //refit
    }
    
    gtk_widget_hide(GTK_WIDGET(multiplot_window)); //close the multiplot window
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
  }
  
}

void on_sum_all_button_clicked(GtkButton *b)
{

  //set all opened spectra to be drawn, in sum mode 

  int i;
  drawing.multiplotMode = 1;//sum spectra
  drawing.numMultiplotSp = rawdata.numSpOpened;
  if(drawing.numMultiplotSp > NSPECT)
    drawing.numMultiplotSp = NSPECT;
  
  for(i=0;i<drawing.numMultiplotSp;i++){
    drawing.multiPlots[i] = i;
  }

  //clear fit if necessary
  if(guiglobals.fittingSp == 5){
    guiglobals.fittingSp = 0;
    //update widgets
    update_gui_fit_state();
  }

  gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE); //sum mode, therefore can fit
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));

}

void on_spectrum_selector_changed(GtkSpinButton *spin_button, gpointer user_data)
{
  if(!guiglobals.deferSpSelChange){
    drawing.multiPlots[0] = gtk_spin_button_get_value_as_int(spin_button) - 1;
    drawing.multiplotMode = 0;//unset multiplot, if it is being used
    drawing.numMultiplotSp = 1;//unset multiplot
    
    //clear fit if necessary
    if(guiglobals.fittingSp == 5){
      guiglobals.fittingSp = 0;
      //update widgets
      update_gui_fit_state();
    }

    gtk_widget_set_sensitive(GTK_WIDGET(fit_button),TRUE); //no multiplot, therefore can fit
    //printf("Set selected spectrum to %i\n",dispSp);
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }else{
    guiglobals.deferSpSelChange = 0;
  }
  
}

void on_fit_button_clicked(GtkButton *b)
{
  //do some checks, since this can be activated from a keyboard shortcut
  //spectrum must be open to fit
  if(rawdata.openedSp){
    //cannot be already fitting
    if((guiglobals.fittingSp == 0)||(guiglobals.fittingSp == 5)){
      //must be displaying only a single spectrum
      if(drawing.multiplotMode < 2){

        //safe to fit
      
        guiglobals.fittingSp = 1;
        memset(fitpar.fitParVal,0,sizeof(fitpar.fitParVal));
        //set default values
        fitpar.fitStartCh = -1;
        fitpar.fitEndCh = -1;
        fitpar.numFitPeaks = 0;
        //update widgets
        update_gui_fit_state();
      }
    }
  }
  
}

void on_fit_fit_button_clicked(GtkButton *b)
{
  startGausFit(); //perform the fit
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  //update widgets
  update_gui_fit_state();
}

void on_fit_cancel_button_clicked(GtkButton *b)
{
  guiglobals.fittingSp = 0;
  //update widgets
  update_gui_fit_state();
}

void on_fit_preferences_button_clicked(GtkButton *b)
{
  showPreferences(1);
}

void on_toggle_discard_empty(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    rawdata.dropEmptySpectra=1;
  else
    rawdata.dropEmptySpectra=0;
}
void on_toggle_bin_errors(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.showBinErrors=1;
  else
    guiglobals.showBinErrors=0;
}
void on_toggle_round_errors(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.roundErrors=1;
  else
    guiglobals.roundErrors=0;
}
void on_toggle_dark_theme(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.preferDarkTheme=1;
  else
    guiglobals.preferDarkTheme=0;
  
  setupUITheme();
}
void on_toggle_animation(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.useZoomAnimations=1;
  else
    guiglobals.useZoomAnimations=0;
}
void on_toggle_autozoom(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.autoZoom=1;
  else
    guiglobals.autoZoom=0;
}

void on_toggle_spectrum_label(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.drawSpLabels=1;
  else
    guiglobals.drawSpLabels=0;
}

void on_toggle_spectrum_comment(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.drawSpComments=1;
  else
    guiglobals.drawSpComments=0;
}

void on_toggle_relative_widths(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    fitpar.fixRelativeWidths=1;
  else
    fitpar.fixRelativeWidths=0;
}

void on_toggle_popup_results(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    guiglobals.popupFitResults=1;
  else
    guiglobals.popupFitResults=0;
}

void on_preferences_button_clicked(GtkButton *b)
{
  showPreferences(0);
}

void on_preferences_apply_button_clicked(GtkButton *b)
{
  fitpar.weightMode = gtk_combo_box_get_active(GTK_COMBO_BOX(weight_mode_combobox));
  updateConfigFile();
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area)); //redraw the spectrum
  gtk_widget_hide(GTK_WIDGET(preferences_window)); //close the multiplot window
}

void on_preferences_cancel_button_clicked(GtkButton *b)
{
  updatePrefsFromConfigFile(); //rather than updating the config file, read from it to revert settings
  setupUITheme();
  //hide the dialog
  gtk_widget_hide(GTK_WIDGET(preferences_window)); //close the multiplot window
}

void on_shortcuts_button_clicked(GtkButton *b)
{
  gtk_window_present(GTK_WINDOW(shortcuts_window)); //show the window
}

void on_help_button_clicked(GtkButton *b)
{
  gtk_window_present(help_window); //show the window
}

void on_about_button_clicked(GtkButton *b)
{
  gtk_window_present(GTK_WINDOW(about_dialog)); //show the window
}


void iniitalizeUIElements(){
  //import UI layout and graphics data
  builder = gtk_builder_new_from_resource("/resources/jf3.glade"); //get UI layout from glade XML file
  gtk_builder_add_from_resource (builder, "/resources/shortcuts_window.ui", NULL);
  appIcon = gdk_pixbuf_new_from_resource("/resources/jf3-application-icon.svg", NULL);
  spIconPixbuf = gdk_pixbuf_new_from_resource("/resources/icon-spectrum-symbolic", NULL);
  spIconPixbufDark = gdk_pixbuf_new_from_resource("/resources/icon-spectrum-symbolic-dark", NULL);

  //windows
  window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); //quit the program when closing the window
  calibrate_window = GTK_WINDOW(gtk_builder_get_object(builder, "calibration_window"));
  gtk_window_set_transient_for(calibrate_window, window); //center calibrate window on main window
  comment_window = GTK_WINDOW(gtk_builder_get_object(builder, "comment_window"));
  gtk_window_set_transient_for(comment_window, window); //center comment window on main window
  multiplot_window = GTK_WINDOW(gtk_builder_get_object(builder, "multiplot_window"));
  gtk_window_set_transient_for(multiplot_window, window); //center multiplot window on main window
  export_options_window = GTK_WINDOW(gtk_builder_get_object(builder, "export_options_window"));
  gtk_window_set_transient_for(export_options_window, window); //center export options window on main window
  preferences_window = GTK_WINDOW(gtk_builder_get_object(builder, "preferences_window"));
  gtk_window_set_transient_for(preferences_window, window); //center preferences window on main window
  shortcuts_window = GTK_SHORTCUTS_WINDOW(gtk_builder_get_object(builder, "shortcuts_window"));
  gtk_window_set_transient_for(GTK_WINDOW(shortcuts_window), window); //center shortcuts window on main window
  help_window = GTK_WINDOW(gtk_builder_get_object(builder, "help_window"));
  gtk_window_set_transient_for(GTK_WINDOW(help_window), window); //center help window on main window
  about_dialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "about_dialog"));
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), window); //center about dialog on main window
  main_window_accelgroup = GTK_ACCEL_GROUP(gtk_builder_get_object(builder, "main_window_accelgroup"));
  gtk_window_add_accel_group (window, main_window_accelgroup);
  
  //header bar
  header_bar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));

  //menus
  display_popover = GTK_POPOVER(gtk_builder_get_object(builder, "display_popover"));

  //main window UI elements
  open_button = GTK_BUTTON(gtk_builder_get_object(builder, "open_button"));
  append_button = GTK_BUTTON(gtk_builder_get_object(builder, "append_button"));
  calibrate_button = GTK_BUTTON(gtk_builder_get_object(builder, "calibrate_button"));
  fit_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_button"));
  to_save_menu_button = GTK_BUTTON(gtk_builder_get_object(builder, "to_save_menu_button"));
  save_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
  save_button_radware = GTK_BUTTON(gtk_builder_get_object(builder, "save_button_radware"));
  save_button_text = GTK_BUTTON(gtk_builder_get_object(builder, "save_button_text"));
  help_button = GTK_BUTTON(gtk_builder_get_object(builder, "help_button"));
  display_button = GTK_BUTTON(gtk_builder_get_object(builder, "display_button"));
  display_button_icon = GTK_IMAGE(gtk_builder_get_object(builder, "display_button_icon"));
  no_sp_image= GTK_IMAGE(gtk_builder_get_object(builder, "no_sp_image"));
  spectrum_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "spectrumdrawingarea"));
  spectrum_drag_gesture = gtk_gesture_drag_new(spectrum_drawing_area); //without this, cannot click away from menus onto the drawing area, needs further investigation
  zoom_scale = GTK_SCALE(gtk_builder_get_object(builder, "zoom_scale"));
  shortcuts_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "shortcuts_button"));
  about_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "about_button"));
  bottom_info_text = GTK_LABEL(gtk_builder_get_object(builder, "bottom_info_text"));
  no_sp_box = GTK_BOX(gtk_builder_get_object(builder, "no_sp_box"));

  //fit interface UI elements
  revealer_info_panel = GTK_REVEALER(gtk_builder_get_object(builder, "revealer_info_panel"));
  revealer_info_label = GTK_LABEL(gtk_builder_get_object(builder, "revealer_info_label"));
  fit_cancel_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_cancel_button"));
  fit_fit_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_fit_button"));
  fit_preferences_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_preferences_button"));
  fit_spinner = GTK_SPINNER(gtk_builder_get_object(builder, "fit_spinner"));

  //calibration window UI elements
  calibrate_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "options_ok_button"));
  remove_calibration_button = GTK_WIDGET(gtk_builder_get_object(builder, "remove_calibration_button"));
  cal_entry_unit = GTK_ENTRY(gtk_builder_get_object(builder, "calibration_unit_entry"));
  cal_entry_y_axis = GTK_ENTRY(gtk_builder_get_object(builder, "y_axis_entry"));
  cal_entry_const = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_const"));
  cal_entry_lin = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_lin"));
  cal_entry_quad = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_quad"));

  //comment window UI elements
  comment_ok_button = GTK_BUTTON(gtk_builder_get_object(builder, "comment_ok_button"));
  remove_comment_button = GTK_BUTTON(gtk_builder_get_object(builder, "remove_comment_button"));
  comment_entry = GTK_ENTRY(gtk_builder_get_object(builder, "comment_entry"));

  //multiplot window UI elements
  multiplot_ok_button = GTK_BUTTON(gtk_builder_get_object(builder, "multiplot_ok_button"));
  multiplot_liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "multiplot_liststore"));
  multiplot_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "multiplot_tree_view"));
  multiplot_column1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column1"));
  multiplot_column2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column2"));
  multiplot_cr1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr1"));
  multiplot_cr2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr2"));
  multiplot_cr3 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr3"));
  multiplot_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "multiplot_mode_combobox"));

  //export options window UI elements
  export_description_label = GTK_LABEL(gtk_builder_get_object(builder, "export_description_label"));
  export_note_label = GTK_LABEL(gtk_builder_get_object(builder, "export_note_label"));
  export_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "export_mode_combobox"));
  export_rebin_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "export_rebin_checkbutton"));
  export_options_save_button = GTK_BUTTON(gtk_builder_get_object(builder, "export_options_save_button"));

  //preferences window UI elements
  preferences_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "preferences_button"));
  preferences_notebook = GTK_NOTEBOOK(gtk_builder_get_object(builder, "preferences_notebook"));
  discard_empty_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "discard_empty_checkbutton"));
  bin_errors_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "bin_errors_checkbutton"));
  round_errors_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "round_errors_checkbutton"));
  autozoom_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autozoom_checkbutton"));
  dark_theme_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dark_theme_checkbutton"));
  spectrum_label_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "spectrum_label_checkbutton"));
  spectrum_comment_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "spectrum_comment_checkbutton"));
  relative_widths_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "relative_widths_checkbutton"));
  weight_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "weight_mode_combobox"));
  popup_results_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "popup_results_checkbutton"));
  animation_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "animation_checkbutton"));
  preferences_apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "preferences_apply_button"));

  //help window UI elements
  display_button_icon1 = GTK_IMAGE(gtk_builder_get_object(builder, "display_button_icon1"));
  display_button_icon2 = GTK_IMAGE(gtk_builder_get_object(builder, "display_button_icon2"));

  //display menu UI elements
  multiplot_button = GTK_BUTTON(gtk_builder_get_object(builder, "multiplot_button"));
  sum_all_button = GTK_BUTTON(gtk_builder_get_object(builder, "sum_all_button"));
  spectrum_selector = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spectrumselector"));
  spectrum_selector_adjustment = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "spectrum_selector_adjustment"));
  autoscale_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autoscalebutton"));
  logscale_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "logscalebutton"));
  cursor_draw_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "cursordrawbutton"));
  contract_scale = GTK_SCALE(gtk_builder_get_object(builder, "contract_scale"));


  //connect signals
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "draw", G_CALLBACK (drawSpectrumArea), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "scroll-event", G_CALLBACK (on_spectrum_scroll), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "motion-notify-event", G_CALLBACK (on_spectrum_cursor_motion), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "button-press-event", G_CALLBACK (on_spectrum_click), NULL);
  g_signal_connect (G_OBJECT (open_button), "clicked", G_CALLBACK (on_open_button_clicked), NULL);
  g_signal_connect (G_OBJECT (append_button), "clicked", G_CALLBACK (on_append_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_button), "clicked", G_CALLBACK (on_calibrate_button_clicked), NULL);
  g_signal_connect (G_OBJECT (save_button), "clicked", G_CALLBACK (on_save_button_clicked), NULL);
  g_signal_connect (G_OBJECT (save_button_radware), "clicked", G_CALLBACK (on_save_radware_button_clicked), NULL);
  g_signal_connect (G_OBJECT (save_button_text), "clicked", G_CALLBACK (on_save_text_button_clicked), NULL);
  g_signal_connect (G_OBJECT (help_button), "clicked", G_CALLBACK (on_help_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_button), "clicked", G_CALLBACK (on_multiplot_button_clicked), NULL);
  g_signal_connect (G_OBJECT (sum_all_button), "clicked", G_CALLBACK (on_sum_all_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_button), "clicked", G_CALLBACK (on_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_fit_button), "clicked", G_CALLBACK (on_fit_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_cancel_button), "clicked", G_CALLBACK (on_fit_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_preferences_button), "clicked", G_CALLBACK (on_fit_preferences_button_clicked), NULL);
  g_signal_connect (G_OBJECT (display_button), "clicked", G_CALLBACK (on_display_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_ok_button), "clicked", G_CALLBACK (on_calibrate_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (comment_ok_button), "clicked", G_CALLBACK (on_comment_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (remove_comment_button), "clicked", G_CALLBACK (on_remove_comment_button_clicked), NULL);
  g_signal_connect (G_OBJECT (comment_entry), "changed", G_CALLBACK (on_comment_entry_changed), NULL);
  g_signal_connect (G_OBJECT (remove_calibration_button), "clicked", G_CALLBACK (on_remove_calibration_button_clicked), NULL);
  g_signal_connect (G_OBJECT (spectrum_selector), "value-changed", G_CALLBACK (on_spectrum_selector_changed), NULL);
  g_signal_connect (G_OBJECT (autoscale_button), "toggled", G_CALLBACK (on_toggle_autoscale), NULL);
  g_signal_connect (G_OBJECT (logscale_button), "toggled", G_CALLBACK (on_toggle_logscale), NULL);
  g_signal_connect (G_OBJECT (cursor_draw_button), "toggled", G_CALLBACK (on_toggle_cursor), NULL);
  g_signal_connect (G_OBJECT (discard_empty_checkbutton), "toggled", G_CALLBACK (on_toggle_discard_empty), NULL);
  g_signal_connect (G_OBJECT (export_options_save_button), "clicked", G_CALLBACK (on_export_save_button_clicked), NULL);
  g_signal_connect (G_OBJECT (bin_errors_checkbutton), "toggled", G_CALLBACK (on_toggle_bin_errors), NULL);
  g_signal_connect (G_OBJECT (round_errors_checkbutton), "toggled", G_CALLBACK (on_toggle_round_errors), NULL);
  g_signal_connect (G_OBJECT (dark_theme_checkbutton), "toggled", G_CALLBACK (on_toggle_dark_theme), NULL);
  g_signal_connect (G_OBJECT (spectrum_label_checkbutton), "toggled", G_CALLBACK (on_toggle_spectrum_label), NULL);
  g_signal_connect (G_OBJECT (spectrum_comment_checkbutton), "toggled", G_CALLBACK (on_toggle_spectrum_comment), NULL);
  g_signal_connect (G_OBJECT (relative_widths_checkbutton), "toggled", G_CALLBACK (on_toggle_relative_widths), NULL);
  g_signal_connect (G_OBJECT (popup_results_checkbutton), "toggled", G_CALLBACK (on_toggle_popup_results), NULL);
  g_signal_connect (G_OBJECT (animation_checkbutton), "toggled", G_CALLBACK (on_toggle_animation), NULL);
  g_signal_connect (G_OBJECT (autozoom_checkbutton), "toggled", G_CALLBACK (on_toggle_autozoom), NULL);
  g_signal_connect (G_OBJECT (preferences_apply_button), "clicked", G_CALLBACK (on_preferences_apply_button_clicked), NULL);
  g_signal_connect (G_OBJECT (preferences_button), "clicked", G_CALLBACK (on_preferences_button_clicked), NULL);
  gtk_widget_set_events(spectrum_drawing_area, gtk_widget_get_events (spectrum_drawing_area) | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK); //allow mouse scrolling over the drawing area
  g_signal_connect (G_OBJECT (cal_entry_const), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_lin), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_quad), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (zoom_scale), "value-changed", G_CALLBACK (on_zoom_scale_changed), NULL);
  g_signal_connect (G_OBJECT (contract_scale), "value-changed", G_CALLBACK (on_contract_scale_changed), NULL);
  g_signal_connect (G_OBJECT (shortcuts_button), "clicked", G_CALLBACK (on_shortcuts_button_clicked), NULL);
  g_signal_connect (G_OBJECT (about_button), "clicked", G_CALLBACK (on_about_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_ok_button), "clicked", G_CALLBACK (on_multiplot_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_cr2), "toggled", G_CALLBACK (on_multiplot_cell_toggled), NULL);
  g_signal_connect (G_OBJECT (multiplot_cr3), "edited", G_CALLBACK (on_multiplot_scaling_edited), NULL);
  g_signal_connect (G_OBJECT (calibrate_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (comment_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (multiplot_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (export_options_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (preferences_window), "delete-event", G_CALLBACK (on_preferences_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (preferences_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (shortcuts_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (help_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (about_dialog), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button

  //setup keyboard shortcuts
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_f, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_fit_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_c, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_calibrate_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_p, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_multiplot_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_l, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(toggle_logscale), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_z, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(toggle_cursor), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_o, (GdkModifierType)4, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_open_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_a, (GdkModifierType)4, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_append_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_s, (GdkModifierType)4, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_save_button_clicked), NULL, 0));

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
  drawing.zoomToLevel = 1.0;
  drawing.contractFactor = 1;
  drawing.autoScale = 1;
  drawing.logScale = 0;
  calpar.calMode = 0;
  rawdata.dropEmptySpectra = 1;
  rawdata.numSpOpened = 0;
  rawdata.numChComments = 0;
  drawing.multiplotMode = 0;
  drawing.numMultiplotSp = 1;
  drawing.highlightedPeak = -1;
  drawing.highlightedComment = -1;
  drawing.spColors[0] = 220/255.; drawing.spColors[1] = 50/255.; drawing.spColors[2] = 47/255.;      //RGB values for color 1 (solarized red)
  drawing.spColors[3] = 38/255.; drawing.spColors[4] = 139/255.; drawing.spColors[5] = 210/255.;     //RGB values for color 2 (solarized blue)
  drawing.spColors[6] = 0.0; drawing.spColors[7] = 0.8; drawing.spColors[8] = 0.0;                   //RGB values for color 3
  drawing.spColors[9] = 0.8; drawing.spColors[10] = 0.0; drawing.spColors[11] = 0.8;                 //RGB values for color 4
  drawing.spColors[12] = 0.7; drawing.spColors[13] = 0.4; drawing.spColors[14] = 0.0;                //RGB values for color 5
  drawing.spColors[15] = 42/255.; drawing.spColors[16] = 161/255.; drawing.spColors[17] = 152/255.;  //RGB values for color 6 (solarized cyan)
  drawing.spColors[18] = 203/255.; drawing.spColors[19] = 75/255.; drawing.spColors[20] = 22/255.;   //RGB values for color 7 (solarized orange)
  drawing.spColors[21] = 133/255.; drawing.spColors[22] = 153/255.; drawing.spColors[23] = 0.0;      //RGB values for color 8 (solarized green)
  drawing.spColors[24] = 211/255.; drawing.spColors[25] = 54/255.; drawing.spColors[26] = 130/255.;  //RGB values for color 9 (solarized magenta)
  drawing.spColors[27] = 181/255.; drawing.spColors[28] = 137/255.; drawing.spColors[29] = 0.0;      //RGB values for color 10 (solarized yellow)
  drawing.spColors[30] = 0.5; drawing.spColors[31] = 0.5; drawing.spColors[32] = 0.5;                //RGB values for color 11
  drawing.spColors[33] = 0.7; drawing.spColors[34] = 0.0; drawing.spColors[35] = 0.3;                //RGB values for color 12
  guiglobals.fittingSp = 0;
  guiglobals.deferSpSelChange = 0;
  guiglobals.deferToggleRow = 0;
  guiglobals.draggingSp = 0;
  guiglobals.drawSpCursor = -1; //disabled by default
  guiglobals.drawSpLabels = 1; //enabled by default
  guiglobals.drawSpComments = 1; //enabled by default
  guiglobals.showBinErrors = 1;
  guiglobals.roundErrors = 0;
  guiglobals.autoZoom = 1;
  guiglobals.preferDarkTheme = 0;
  guiglobals.popupFitResults = 1;
  guiglobals.useZoomAnimations = 1;
  guiglobals.framesSinceZoom = -1;
  guiglobals.exportFileType = 0;
  fitpar.fixRelativeWidths = 1;
  fitpar.fitStartCh = -1;
  fitpar.fitEndCh = -1;
  fitpar.numFitPeaks = 0;

  gtk_adjustment_set_lower(spectrum_selector_adjustment, 1);
  gtk_adjustment_set_upper(spectrum_selector_adjustment, 1);
  //gtk_label_set_text(bottom_info_text,"No spectrum loaded.");
  gtk_label_set_text(bottom_info_text,"");
  
  //'gray out' widgets that can't be used yet
  gtk_widget_set_sensitive(GTK_WIDGET(append_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(to_save_menu_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(display_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(zoom_scale),FALSE);
  gtk_widget_hide(GTK_WIDGET(fit_spinner));

  //hide widgets that can't be seen yet
  gtk_revealer_set_reveal_child(revealer_info_panel, FALSE);

  //setup UI element appearance at startup
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoscale_button), drawing.autoScale);

  //setup app icon
  gtk_window_set_default_icon(appIcon);
  gtk_window_set_icon(window,appIcon);
  gtk_about_dialog_set_logo(about_dialog, appIcon);
}