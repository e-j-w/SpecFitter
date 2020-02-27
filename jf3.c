#include "jf3.h"

#include "jf3-resources.c"
#include "read_data.c"
#include "spectrum_drawing.c"

void on_open_button_clicked(GtkButton *b)
{
  int i,j;
  
  file_open_dialog = gtk_file_chooser_dialog_new ("Open Spectrum File(s)", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
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
    glob_numSpOpened = 0; //reset the open spectra
    char *filename;
    GSList *file_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(file_open_dialog));
    for(i=0;i<g_slist_length(file_list);i++){
      filename = g_slist_nth_data(file_list,i);
      int numSp = readSpectrumDataFile(filename,hist,glob_numSpOpened);
      if(numSp > 0){ //see read_data.c
        openedSp = 1;
        //set comments for spectra just opened
        for (j = glob_numSpOpened; j < (glob_numSpOpened+numSp); j++){
          snprintf(histComment[j],256,"%s, spectrum %i",basename((char*)filename),j-glob_numSpOpened);
          //printf("Comment %i: %s\n",j,histComment[j]);
        }
        glob_numSpOpened += numSp;
        //select the first non-empty spectrum by default
        int sel = getFirstNonemptySpectrum(glob_numSpOpened);
        if(sel >=0){
          gtk_spin_button_set_value(spectrum_selector, sel);
          glob_multiPlots[0] = sel;
          gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(display_button),TRUE);
          if(numSp > 1){
            gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),TRUE);
          }
          //set the range of selectable spectra values
          gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
          gtk_adjustment_set_upper(spectrum_selector_adjustment, glob_numSpOpened - 1);
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
    }
    //set the title of the opened spectrum in the header bar
    if(g_slist_length(file_list) > 1){
      char headerBarSub[256];
      snprintf(headerBarSub,256,"%i files loaded",g_slist_length(file_list));
      gtk_header_bar_set_subtitle(header_bar,headerBarSub);
    }else{
      gtk_header_bar_set_subtitle(header_bar,filename);
    }
    g_slist_free(file_list);
    g_free(filename);
  }else{
    gtk_widget_destroy(file_open_dialog);
  }
  gtk_widget_queue_draw(GTK_WIDGET(window));
  
  
}

void on_display_button_clicked(GtkButton *b)
{
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(zoomLevel)/log(2.));//base 2 log of zoom
  gtk_range_set_value(GTK_RANGE(pan_scale),(xChanFocus*100.0/S32K));
  gtk_range_set_value(GTK_RANGE(contract_scale),contractFactor);
  gtk_popover_popup(display_popover); //show the popover menu
}

void on_zoom_scale_changed(GtkRange *range, gpointer user_data){
  zoomLevel = pow(2,gtk_range_get_value(range)); //modify the zoom level
  gtk_widget_queue_draw(GTK_WIDGET(window)); //redraw the spectrum
}
void on_pan_scale_changed(GtkRange *range, gpointer user_data){
  xChanFocus = (S32K/100.0)*gtk_range_get_value(range); //modify the pan level
  gtk_widget_queue_draw(GTK_WIDGET(window)); //redraw the spectrum
}
void on_contract_scale_changed(GtkRange *range, gpointer user_data){
  contractFactor = (int)gtk_range_get_value(range); //modify the contraction factor
  gtk_widget_queue_draw(GTK_WIDGET(window)); //redraw the spectrum
}

void on_calibrate_button_clicked(GtkButton *b)
{
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
  strcpy(calUnit,gtk_entry_get_text(cal_entry_unit));
  if(strcmp(calUnit,"")==0){
    strcpy(calUnit,"Cal. Units");
  }
  calpar0 = (float)strtod(gtk_entry_get_text(cal_entry_const),NULL);
  calpar1 = (float)strtod(gtk_entry_get_text(cal_entry_lin),NULL);
  calpar2 = (float)strtod(gtk_entry_get_text(cal_entry_quad),NULL);
  if(!((calpar0 == 0.0)&&(calpar1==0.0)&&(calpar2==0.0))){
    //not all calibration parameters are zero, calibration is valid
    calMode=1;
    //printf("Calibration parameters: %f %f %f, calMode: %i, calUnit: %s\n",calpar0,calpar1,calpar2,calMode,calUnit);
    gtk_widget_hide(GTK_WIDGET(calibrate_window)); //close the calibration window
    gtk_widget_queue_draw(GTK_WIDGET(window));
  }else{
    calMode=0;
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
  for(i=0;i<glob_numSpOpened;i++){
    gtk_list_store_append(multiplot_liststore,&iter);
    gtk_list_store_set(multiplot_liststore, &iter, 0, histComment[i], -1);
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
        glob_multiPlots[selectedSpCount]=spInd;
        selectedSpCount++;
      }
    }
    readingTreeModel = gtk_tree_model_iter_next (model, &iter);
  }
  glob_numMultiplotSp = selectedSpCount;
  glob_multiplotMode = gtk_combo_box_get_active(GTK_COMBO_BOX(multiplot_mode_combobox));
  if((glob_multiplotMode < 0)||(glob_numMultiplotSp < 2)){
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *message_dialog = gtk_message_dialog_new(multiplot_window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Invalid selection!");
    if(glob_multiplotMode < 0)
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Please select a plotting mode.");
    if(glob_numMultiplotSp < 2)
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Please select at least two spectra to plot together.");
    glob_multiplotMode = 0; //reset the value
    gtk_dialog_run (GTK_DIALOG (message_dialog));
    gtk_widget_destroy (message_dialog);
  }else{
    glob_multiplotMode++; //value of 0 means no multiplot
    printf("Number of spectra selected for plotting: %i.  Selected spectra: ", glob_numMultiplotSp);
    int i;
    for(i=0;i<glob_numMultiplotSp;i++){
      printf("%i ",glob_multiPlots[i]);
    }
    printf(", multiplot mode: %i\n",glob_multiplotMode);
    gtk_widget_hide(GTK_WIDGET(multiplot_window)); //close the multiplot window
    gtk_widget_queue_draw(GTK_WIDGET(window)); //redraw the spectrum
  }
  
}

void on_spectrum_selector_changed(GtkSpinButton *spin_button, gpointer user_data)
{
  glob_multiPlots[0] = gtk_spin_button_get_value_as_int(spin_button);
  glob_multiplotMode = 0;//unset multiplot, if it is being used
  //printf("Set selected spectrum to %i\n",dispSp);
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

void on_about_button_clicked(GtkButton *b)
{
  gtk_window_present(GTK_WINDOW(about_dialog)); //show the window
}

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv); //initialize Gtk

  builder = gtk_builder_new_from_resource("/resources/jf3.glade"); //get UI layout from glade XML file

  window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); //quit the program when closing the window
  calibrate_window = GTK_WINDOW(gtk_builder_get_object(builder, "calibration_window"));
  gtk_window_set_transient_for(calibrate_window, window); //center calibrate window on main window
  multiplot_window = GTK_WINDOW(gtk_builder_get_object(builder, "multiplot_window"));
  gtk_window_set_transient_for(multiplot_window, window); //center multiplot window on main window
  about_dialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "about_dialog"));
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), window); //center about dialog on main window
  
  header_bar = GTK_HEADER_BAR(gtk_builder_get_object(builder, "header_bar"));

  box1 = GTK_WIDGET(gtk_builder_get_object(builder, "box1"));
  open_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_button"));
  calibrate_button = GTK_WIDGET(gtk_builder_get_object(builder, "calibrate_button"));
  fit_button = GTK_WIDGET(gtk_builder_get_object(builder, "fit_button"));
  display_button = GTK_WIDGET(gtk_builder_get_object(builder, "display_button"));
  multiplot_button = GTK_WIDGET(gtk_builder_get_object(builder, "multiplot_button"));
  display_popover = GTK_POPOVER(gtk_builder_get_object(builder, "display_popover"));
  calibrate_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "options_ok_button"));
  spectrum_selector = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spectrumselector"));
  spectrum_selector_adjustment = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "spectrum_selector_adjustment"));
  autoscale_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autoscalebutton"));
  spectrum_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "spectrumdrawingarea"));
  spectrum_drag_gesture = gtk_gesture_drag_new(spectrum_drawing_area);
  cal_entry_unit = GTK_ENTRY(gtk_builder_get_object(builder, "calibration_unit_entry"));
  cal_entry_const = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_const"));
  cal_entry_lin = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_lin"));
  cal_entry_quad = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_quad"));
  zoom_scale = GTK_SCALE(gtk_builder_get_object(builder, "zoom_scale"));
  pan_scale = GTK_SCALE(gtk_builder_get_object(builder, "pan_scale"));
  contract_scale = GTK_SCALE(gtk_builder_get_object(builder, "contract_scale"));
  about_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "about_button"));
  multiplot_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "multiplot_ok_button"));
  multiplot_liststore = GTK_LIST_STORE(gtk_builder_get_object(builder, "multiplot_liststore"));
  multiplot_tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "multiplot_tree_view"));
  multiplot_column1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column1"));
  multiplot_column2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "multiplot_column2"));
  multiplot_cr1 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr1"));
  multiplot_cr2 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr2"));
  multiplot_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "multiplot_mode_combobox"));

  //connect signals
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "draw", G_CALLBACK (drawSpectrumArea), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "scroll-event", G_CALLBACK (on_spectrum_scroll), NULL);
  g_signal_connect (G_OBJECT (open_button), "clicked", G_CALLBACK (on_open_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_button), "clicked", G_CALLBACK (on_calibrate_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_button), "clicked", G_CALLBACK (on_multiplot_button_clicked), NULL);
  g_signal_connect (G_OBJECT (display_button), "clicked", G_CALLBACK (on_display_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_ok_button), "clicked", G_CALLBACK (on_calibrate_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (spectrum_selector), "value-changed", G_CALLBACK (on_spectrum_selector_changed), NULL);
  g_signal_connect (G_OBJECT (autoscale_button), "toggled", G_CALLBACK (on_toggle_autoscale), NULL);
  gtk_widget_set_events(spectrum_drawing_area, GDK_SCROLL_MASK); //allow mouse scrolling over the drawing area
  g_signal_connect (G_OBJECT (spectrum_drag_gesture), "drag-begin", G_CALLBACK (on_spectrum_drag_begin), NULL);
  g_signal_connect (G_OBJECT (spectrum_drag_gesture), "drag-update", G_CALLBACK (on_spectrum_drag_update), NULL);
  g_signal_connect (G_OBJECT (cal_entry_const), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_lin), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_quad), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (zoom_scale), "value-changed", G_CALLBACK (on_zoom_scale_changed), NULL);
  g_signal_connect (G_OBJECT (pan_scale), "value-changed", G_CALLBACK (on_pan_scale_changed), NULL);
  g_signal_connect (G_OBJECT (contract_scale), "value-changed", G_CALLBACK (on_contract_scale_changed), NULL);
  g_signal_connect (G_OBJECT (about_button), "clicked", G_CALLBACK (on_about_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_ok_button), "clicked", G_CALLBACK (on_multiplot_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (multiplot_cr2), "toggled", G_CALLBACK (on_multiplot_cell_toggled), NULL);
  g_signal_connect (G_OBJECT (calibrate_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (multiplot_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (about_dialog), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button

  //set attributes
  gtk_tree_view_column_add_attribute(multiplot_column2,multiplot_cr2, "active",1);

  //set default values
  openedSp = 0;
  lowerLimit = 0;
  upperLimit = S32K - 1;
  scaleLevelMax = 1000.0;
  scaleLevelMin = 0.0;
  xChanFocus = 0;
  zoomLevel = 1.0;
  contractFactor = 1;
  autoScale = 1;
  calMode = 0;
  glob_numSpOpened = 0;
  glob_multiplotMode = 0;
  glob_numMultiplotSp = 1;
  gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
  gtk_adjustment_set_upper(spectrum_selector_adjustment, 0);

  //'gray out' widgets that can't be used yet
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(fit_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(display_button),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(multiplot_button),FALSE);

  //setup UI element appearance at startup
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoscale_button), autoScale);

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main();              //Gtk main loop

  return 0;
}

