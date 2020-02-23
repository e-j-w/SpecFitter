#include "jf3.h"

#include "jf3-resources.c"
#include "read_data.c"
#include "spectrum_drawing.c"

void on_open_button_clicked(GtkButton *b)
{
  file_open_dialog = gtk_file_chooser_dialog_new ("Open Spectrum File", window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
  file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter,"Spectra (.mca, .fmca, .spe)");
  gtk_file_filter_add_pattern(file_filter,"*.mca");
  gtk_file_filter_add_pattern(file_filter,"*.fmca");
  gtk_file_filter_add_pattern(file_filter,"*.spe");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_open_dialog),file_filter);

  if (gtk_dialog_run(GTK_DIALOG(file_open_dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_open_dialog));
    int numSp = readSpectrumDataFile(filename,hist);
    if(numSp > 0){ //see read_data.c
      openedSp = 1;
      //set the range of selectable spectra values
      gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
      gtk_adjustment_set_upper(spectrum_selector_adjustment, numSp - 1);
      //select the 0th spectrum by default
      gtk_spin_button_set_value(spectrum_selector, 0);
      dispSp = 0;
      gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),TRUE);
      if(numSp > 1){
        gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),TRUE);
      }
      gtk_widget_queue_draw(GTK_WIDGET(window));
    } 
    g_free(filename);
  }

  gtk_widget_destroy(file_open_dialog);
}

void on_contract_button_clicked(GtkButton *b)
{
  gtk_popover_popup(contract_popover); //show the popover menu
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
  }
  printf("Calibration parameters: %f %f %f, calMode: %i, calUnit: %s\n",calpar0,calpar1,calpar2,calMode,calUnit);
  gtk_widget_hide(GTK_WIDGET(calibrate_window)); //close the window
}

void on_calibrate_cancel_button_clicked(GtkButton *b)
{
  gtk_widget_hide(GTK_WIDGET(calibrate_window)); //close the window
}

void on_spectrum_selector_changed(GtkSpinButton *spin_button, gpointer user_data)
{
  dispSp = gtk_spin_button_get_value_as_int(spin_button);
  //printf("Set selected spectrum to %i\n",dispSp);
  gtk_widget_queue_draw(GTK_WIDGET(window));
}

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv); //initialize Gtk

  builder = gtk_builder_new_from_resource("/resources/jf3.glade"); //get UI layout from glade XML file

  window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL); //quit the program when closing the window
  calibrate_window = GTK_WINDOW(gtk_builder_get_object(builder, "calibration_window"));
  gtk_window_set_transient_for(calibrate_window, window); //center options window on main window
  gtk_builder_connect_signals(builder, NULL);                           //build the (button/widget) signal table from the glade XML data

  box1 = GTK_WIDGET(gtk_builder_get_object(builder, "box1"));
  open_button = GTK_WIDGET(gtk_builder_get_object(builder, "open_button"));
  calibrate_button = GTK_WIDGET(gtk_builder_get_object(builder, "calibrate_button"));
  contract_button = GTK_WIDGET(gtk_builder_get_object(builder, "contract_button"));
  contract_popover = GTK_POPOVER(gtk_builder_get_object(builder, "contract_popover"));
  calibrate_ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "options_ok_button"));
  calibrate_cancel_button = GTK_WIDGET(gtk_builder_get_object(builder, "options_cancel_button"));
  spectrum_selector = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spectrumselector"));
  spectrum_selector_adjustment = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "spectrum_selector_adjustment"));
  autoscale_button = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autoscalebutton"));
  status_label = GTK_LABEL(gtk_builder_get_object(builder, "statuslabel"));
  spectrum_drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "spectrumdrawingarea"));
  spectrum_drag_gesture = gtk_gesture_drag_new(spectrum_drawing_area);
  cal_entry_unit = GTK_ENTRY(gtk_builder_get_object(builder, "calibration_unit_entry"));
  cal_entry_const = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_const"));
  cal_entry_lin = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_lin"));
  cal_entry_quad = GTK_ENTRY(gtk_builder_get_object(builder, "cal_entry_quad"));

  //connect signals
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "draw", G_CALLBACK (drawSpectrumArea), NULL);
  g_signal_connect (G_OBJECT (spectrum_drawing_area), "scroll-event", G_CALLBACK (on_spectrum_scroll), NULL);
  g_signal_connect (G_OBJECT (open_button), "clicked", G_CALLBACK (on_open_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_button), "clicked", G_CALLBACK (on_calibrate_button_clicked), NULL);
  g_signal_connect (G_OBJECT (contract_button), "clicked", G_CALLBACK (on_contract_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_ok_button), "clicked", G_CALLBACK (on_calibrate_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_cancel_button), "clicked", G_CALLBACK (on_calibrate_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (spectrum_selector), "value-changed", G_CALLBACK (on_spectrum_selector_changed), NULL);
  g_signal_connect (G_OBJECT (autoscale_button), "toggled", G_CALLBACK (on_toggle_autoscale), NULL);
  gtk_widget_set_events(spectrum_drawing_area, GDK_SCROLL_MASK); //allow mouse scrolling over the drawing area
  g_signal_connect (G_OBJECT (spectrum_drag_gesture), "drag-begin", G_CALLBACK (on_spectrum_drag_begin), NULL);
  g_signal_connect (G_OBJECT (spectrum_drag_gesture), "drag-update", G_CALLBACK (on_spectrum_drag_update), NULL);
  g_signal_connect (G_OBJECT (cal_entry_const), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_lin), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);
  g_signal_connect (G_OBJECT (cal_entry_quad), "preedit-changed", G_CALLBACK (on_cal_par_activate), NULL);

  //set default values
  openedSp = 0;
  dispSp = 0;
  lowerLimit = 0;
  upperLimit = S32K - 1;
  scaleLevelMax = 1000.0;
  scaleLevelMin = 0.0;
  xChanFocus = 0;
  zoomLevel = 1.0;
  autoScale = 1;
  calMode = 0;
  gtk_adjustment_set_lower(spectrum_selector_adjustment, 0);
  gtk_adjustment_set_upper(spectrum_selector_adjustment, 0);

  //'gray out' widgets that can't be used yet
  gtk_widget_set_sensitive(GTK_WIDGET(spectrum_selector),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(autoscale_button),FALSE);

  //setup UI element appearance at startup
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoscale_button), autoScale);

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main();              //Gtk main loop

  return 0;
}

