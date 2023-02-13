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

  //setup config file
  char dirPath[256];
  strcpy(dirPath,"");
  strcat(dirPath,g_get_user_config_dir());
  strcat(dirPath,"/specfitter");
  struct stat st = {0};
  if (stat(dirPath, &st) == -1) {
    //config directory doesn't exist, make it
    mkdir(dirPath, 0700);
    printf("Setup configuration file directory: %s\n",dirPath);
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
        printf("ERROR: Could not read config file!\n");
        return 0;
      }
    }else{
      printf("WARNING: Unable to create configuration file, falling back to default values.\n");
    }
  }else{
    readConfigFile(configFile,calpar.calMode);
    fclose(configFile);
  }

  setupUITheme(); //see gui.c

  //startup UI
  gtk_widget_show(GTK_WIDGET(window)); //show the window
  gtk_main(); //start GTK main loop

  return 0;
}

