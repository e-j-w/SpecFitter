/* © J. Williams, 2020-2023 */
// SpecFitter - A spectrum viewer/analysis app using GTK3, for gamma-ray spectroscopy or similar

//definitions and global variables
#include "specfitter.h"
#include "specfitter-resources.c"

//routines
#include "spectrum_data.c" //functions which access imported spectrum/histogram data
#include "fit_data.c" //functions for fitting imported data
#include "spectrum_drawing.c" //functions for drawing imported data
//read/write routines
#include "read_data.c"
#include "write_data.c"
#include "read_config.c" //functions for reading/writing user preferences 
//GTK interaction routines
#include "gui.c"

int main(int argc, char *argv[]){
  
  gtk_init(&argc, &argv); //initialize GTK
  iniitalizeUIElements(); //see gui.c

  //setup config file
  //do this prior to reading in spectra,
  //as processing of spectra may depend
  //on the configuration
  char dirPath[256];
  strcpy(dirPath,"");
  strcat(dirPath,g_get_user_config_dir());
  strcat(dirPath,"/specfitter");
  GStatBuf st = {0};
  if(g_stat(dirPath, &st) == -1){
    //config directory doesn't exist, make it
    if(g_mkdir_with_parents(dirPath, 0700)==0){
      printf("Setup configuration file directory: %s\n",dirPath);
    }else{
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error: cannot set up configuration file");
      char errMsg[256];
      snprintf(errMsg,256,"Couldn't create the configuration file directory at: %s",dirPath);
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"%s",errMsg);
      gtk_dialog_run(GTK_DIALOG(message_dialog));
      gtk_widget_destroy(message_dialog);
      return 0;
    }
  }
  FILE *configFile;
  strcat(dirPath,"/specfitter.conf");
  if((configFile = fopen(dirPath, "r")) == NULL){ //open the config file
    printf("Creating configuration file at: %s\n", dirPath);
    configFile = fopen(dirPath, "w");
    if(configFile != NULL){
      writeConfigFile(configFile); //write the default configuration values
      fclose(configFile);
      if((configFile = fopen(dirPath, "r")) != NULL){ //open the config file
        readConfigFile(configFile,calpar.calMode);
      }else{
        GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Error: cannot read configuration file");
        char errMsg[256];
        snprintf(errMsg,256,"Couldn't read the configuration file at: %s",dirPath);
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"%s",errMsg);
        gtk_dialog_run(GTK_DIALOG(message_dialog));
        gtk_widget_destroy(message_dialog);
        return 0;
      }
    }else{
      GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
      GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "Cannot read configuration file");
      char warnMsg[256];
      snprintf(warnMsg,256,"Couldn't read the configuration file at: %s, will fall back to default values.",dirPath);
      gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"%s",warnMsg);
      gtk_dialog_run(GTK_DIALOG(message_dialog));
      gtk_widget_destroy(message_dialog);
    }
  }else{
    readConfigFile(configFile,calpar.calMode);
    fclose(configFile);
  }

  //open a file if requested from the command line
  if(argc > 1){
    currentFolderSelection = g_path_get_dirname(argv[1]); //set current directory (for file chooser) to the location of the first file
    openSingleFile(argv[1],0);
    for(int32_t i=2;i<argc;i++){
      openSingleFile(argv[i],1);
    }
    //set headerbar info for opened files
    if(argc>2){
      char headerBarSub[256];
      rawdata.numFilesOpened = (uint8_t)(argc - 1);
      snprintf(headerBarSub,256,"%i files loaded",rawdata.numFilesOpened);
      gtk_header_bar_set_subtitle(header_bar,headerBarSub);
    }else{
      gtk_header_bar_set_subtitle(header_bar,argv[1]);
    }
    gtk_widget_set_sensitive(GTK_WIDGET(append_button),TRUE);
  }

  setupUITheme(); //see gui.c

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main(); //start GTK main loop

  return 0;
}

