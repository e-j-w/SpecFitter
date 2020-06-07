/* J. Williams, 2020 */

#include "jf3.h"

#include "jf3-resources.c"
#include "utils.c"
#include "read_data.c"
#include "read_config.c"
#include "fit_data.c"
#include "spectrum_drawing.c"
#include "gui.c"

int main(int argc, char *argv[])
{
  
  gtk_init(&argc, &argv); //initialize Gtk

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
  multiplot_window = GTK_WINDOW(gtk_builder_get_object(builder, "multiplot_window"));
  gtk_window_set_transient_for(multiplot_window, window); //center multiplot window on main window
  preferences_window = GTK_WINDOW(gtk_builder_get_object(builder, "preferences_window"));
  gtk_window_set_transient_for(preferences_window, window); //center preferences window on main window
  shortcuts_window = GTK_SHORTCUTS_WINDOW(gtk_builder_get_object(builder, "shortcuts_window"));
  gtk_window_set_transient_for(GTK_WINDOW(shortcuts_window), window); //center shortcuts window on main window
  about_dialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "about_dialog"));
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), window); //center about dialog on main window
  main_window_accelgroup = GTK_ACCEL_GROUP(gtk_builder_get_object(builder, "main_window_accelgroup"));
  gtk_window_add_accel_group (window, main_window_accelgroup);
  
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
  display_button_icon = GTK_IMAGE(gtk_builder_get_object(builder, "display_button_icon"));
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
  multiplot_cr3 = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "multiplot_cr3"));
  multiplot_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "multiplot_mode_combobox"));

  //preferences window UI elements
  preferences_button = GTK_MODEL_BUTTON(gtk_builder_get_object(builder, "preferences_button"));
  preferences_notebook = GTK_NOTEBOOK(gtk_builder_get_object(builder, "preferences_notebook"));
  discard_empty_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "discard_empty_checkbutton"));
  bin_errors_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "bin_errors_checkbutton"));
  round_errors_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "round_errors_checkbutton"));
  autozoom_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "autozoom_checkbutton"));
  dark_theme_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "dark_theme_checkbutton"));
  spectrum_label_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "spectrum_label_checkbutton"));
  relative_widths_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "relative_widths_checkbutton"));
  weight_mode_combobox = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "weight_mode_combobox"));
  popup_results_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "popup_results_checkbutton"));
  animation_checkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "animation_checkbutton"));
  preferences_apply_button = GTK_BUTTON(gtk_builder_get_object(builder, "preferences_apply_button"));

  //display menu UI elements
  multiplot_button = GTK_WIDGET(gtk_builder_get_object(builder, "multiplot_button"));
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
  g_signal_connect (G_OBJECT (multiplot_button), "clicked", G_CALLBACK (on_multiplot_button_clicked), NULL);
  g_signal_connect (G_OBJECT (sum_all_button), "clicked", G_CALLBACK (on_sum_all_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_button), "clicked", G_CALLBACK (on_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_fit_button), "clicked", G_CALLBACK (on_fit_fit_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_cancel_button), "clicked", G_CALLBACK (on_fit_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (fit_preferences_button), "clicked", G_CALLBACK (on_fit_preferences_button_clicked), NULL);
  g_signal_connect (G_OBJECT (display_button), "clicked", G_CALLBACK (on_display_button_clicked), NULL);
  g_signal_connect (G_OBJECT (calibrate_ok_button), "clicked", G_CALLBACK (on_calibrate_ok_button_clicked), NULL);
  g_signal_connect (G_OBJECT (remove_calibration_button), "clicked", G_CALLBACK (on_remove_calibration_button_clicked), NULL);
  g_signal_connect (G_OBJECT (spectrum_selector), "value-changed", G_CALLBACK (on_spectrum_selector_changed), NULL);
  g_signal_connect (G_OBJECT (autoscale_button), "toggled", G_CALLBACK (on_toggle_autoscale), NULL);
  g_signal_connect (G_OBJECT (logscale_button), "toggled", G_CALLBACK (on_toggle_logscale), NULL);
  g_signal_connect (G_OBJECT (cursor_draw_button), "toggled", G_CALLBACK (on_toggle_cursor), NULL);
  g_signal_connect (G_OBJECT (discard_empty_checkbutton), "toggled", G_CALLBACK (on_toggle_discard_empty), NULL);
  g_signal_connect (G_OBJECT (bin_errors_checkbutton), "toggled", G_CALLBACK (on_toggle_bin_errors), NULL);
  g_signal_connect (G_OBJECT (round_errors_checkbutton), "toggled", G_CALLBACK (on_toggle_round_errors), NULL);
  g_signal_connect (G_OBJECT (dark_theme_checkbutton), "toggled", G_CALLBACK (on_toggle_dark_theme), NULL);
  g_signal_connect (G_OBJECT (spectrum_label_checkbutton), "toggled", G_CALLBACK (on_toggle_spectrum_label), NULL);
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
  g_signal_connect (G_OBJECT (multiplot_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (preferences_window), "delete-event", G_CALLBACK (on_preferences_cancel_button_clicked), NULL);
  g_signal_connect (G_OBJECT (preferences_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (shortcuts_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button
  g_signal_connect (G_OBJECT (about_dialog), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL); //so that the window is hidden, not destroyed, when hitting the x button

  //setup keyboard shortcuts
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_f, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_fit_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_c, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_calibrate_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_a, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_multiplot_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_l, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(toggle_logscale), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_o, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(on_open_button_clicked), NULL, 0));
  gtk_accel_group_connect (main_window_accelgroup, GDK_KEY_z, (GdkModifierType)0, GTK_ACCEL_VISIBLE, g_cclosure_new(G_CALLBACK(toggle_cursor), NULL, 0));

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
  drawing.multiplotMode = 0;
  drawing.numMultiplotSp = 1;
  drawing.highlightedPeak = -1;
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
  gui.fittingSp = 0;
  gui.deferSpSelChange = 0;
  gui.deferToggleRow = 0;
  gui.draggingSp = 0;
  gui.drawSpCursor = -1; //disabled by default
  gui.drawSpLabels = 1; //enabled by default
  gui.showBinErrors = 1;
  gui.roundErrors = 0;
  gui.autoZoom = 1;
  gui.preferDarkTheme = 0;
  gui.popupFitResults = 1;
  gui.useZoomAnimations = 1;
  gui.framesSinceZoom = -1;
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

  //set whether dark theme is preferred
  g_object_set(gtk_settings_get_default(),"gtk-application-prefer-dark-theme", gui.preferDarkTheme, NULL);
  if(gui.preferDarkTheme){
    gtk_image_set_from_pixbuf(display_button_icon, spIconPixbufDark);
  }else{
    gtk_image_set_from_pixbuf(display_button_icon, spIconPixbuf);
  }

  gtk_window_set_default_icon(appIcon);
  gtk_window_set_icon(window,appIcon);
  gtk_about_dialog_set_logo(about_dialog, appIcon);

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main();              //Gtk main loop

  return 0;
}

