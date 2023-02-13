# **SpecFitter**

A program for gamma-ray spectroscopy or similar data analysis.  Uses [GTK](https://www.gtk.org/) for the user interface.  Things should mostly work (I'm using this to analyze real data), but results are not guaranteed and all the typical disclaimers apply (ie. there's no warranty).

A list of features is [here](FEATURES.md).

## Screenshots

![SpecFitter user interface screenshot](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/specfitter.png "SpecFitter user interface")


## Getting started

### Compatibility

This program has been tested on CentOS 7, Ubuntu 18.04/20.04, and Arch Linux (as of February 2023) under the GNOME desktop environment.  It should work on any Linux distro that satisfies the listed build dependencies.  It may be possible to build this on other platforms where GTK is [available](https://www.gtk.org/docs/installations/), but that hasn't been tested.

### Build dependencies

* make
* gcc
* pkg-config
* GTK3

In CentOS 7:

```
yum install gcc gtk3-devel
```

In Ubuntu:

```
apt install build-essential libgtk-3-dev
```

In Arch Linux:

```
pacman -S gcc make pkgconf gtk3
```

### Build instructions

A Makefile is provided, build the program using:

```make -j``` 

The resulting `specfitter` executable can be run directly from the command line or your file manager.  Optionally, the program can be installed for all users with:

```sudo make install```

This will place the `specfitter` binary in `/usr/bin`, a data-type definition file in `/usr/share/mime/packages` (to allow opening compatible files directly from the file manager), and a desktop entry file in `/usr/share/applications` (for desktop environment integration).  These changes can be undone with:

```sudo make uninstall```

## Usage tips

* Preferences are stored in a plaintext configuration file on a per-user basis at `$XDG_CONFIG_HOME/specfitter/specfitter.conf` (usually `~/.config/specfitter/specfitter.conf`).
* When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `specfitter /path/to/file1 /path/to/file2`).

## Acknowledgements

The peak fitter is based on [RadWare](https://radware.phy.ornl.gov/) code by David C. Radford with some modifications.  The RadWare source code can be obtained from [this](https://github.com/radforddc/rw05) repository.