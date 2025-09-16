# **SpecFitter**

A program for gamma-ray spectroscopy or similar data analysis.  Uses [GTK](https://www.gtk.org/) for the user interface.  Things should mostly work (I'm using this to analyze real data), but there's no warranty.

A list of features is [here](FEATURES.md).

## Screenshots

![SpecFitter user interface screenshot](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/specfitter.png "SpecFitter user interface")


## Building from source

Detailed build instructions for Linux-based systems are [here](BUILDING.md).

## Usage tips

* Preferences are stored in a plaintext configuration file on a per-user basis at `$XDG_CONFIG_HOME/specfitter/specfitter.conf` (usually `~/.config/specfitter/specfitter.conf`).
* When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `specfitter /path/to/file1 /path/to/file2`).

## Acknowledgements

The peak fitter is based on [RadWare](https://radware.phy.ornl.gov/) code by David C. Radford with some modifications.  The RadWare source code can be obtained from [this](https://github.com/radforddc/rw05) repository.
