# **jf3**

**A simple spectrum viewer in GTK3**

## Description

A (1D) spectrum viewer and fitter app, with functionality inspired by the `gf3` program for gamma-ray spectroscopy in the [RadWare](https://radware.phy.ornl.gov/) software package.

## Features

### Supported file formats

* **.txt** - A plaintext file of tab-separated values, where each row repesents a bin/channel and each column represents a spectrum (single column needed for 1 spectrum, 2 columns for 2, etc.).
* **.mca** - An .mca file is simply a 2D array of integers, with the first index denoting a spectrum number (array length up to 100) and the second index denoting a bin number (array length fixed to 32768 ie. 2<sup>15</sup>).
* **.fmca** - An .fmca file has the same format as .mca except it uses floats rather than integers.
* **.spe** -  Spectrum format used/generated by the [RadWare](https://radware.phy.ornl.gov/) software package, for example when using the 'ws' command in `gf3`.

Some sample files that the program can open are available [here](https://mega.nz/#!yUtRBAYR!ATst0ngazksR-g-P-Qdsw2rd4lpHJXBpd6nJq6pW77I) (.zip archive).

Conversion codes for some of these data formats are available in the [FileConvTools](https://github.com/e-j-w/FileConvTools) repository.

### Spectrum display options

* Import spectra from multiple files and plot multiple spectra simultaneously:
    * Sum spectra together.
    * Overlay spectra, with either common or independent scaling on the y-axis.
    * Show spectra in a "stacked" view (tiled vertically with a common x-axis).
* Zoom and pan using the mouse (mouse wheel, click and drag).
* Display in linear or logarithmic scale on the y-axis.
* Rescale spectra, to perform operations such as background subtraction.
* Rebin spectra, with results displayed in real time.

### Peak fitting

* Fit multiple Gaussian peak shapes on quadratic background (iterative least-squares fitter).
* Relative peak widths may be fixed (recommended for gamma-ray spectroscopy) or allowed to freely vary.
* Weight by the data (taking background subtraction into account) or by the fit function.  Or don't weight the fit at all.

## Getting started

### Compatibility

The build process has been tested on CentOS 7 and Arch Linux (as of May 2020) under the GNOME desktop environment.  It should more-or-less work on any Linux distro that satisfies the build dependencies below.  It may be possible to build this on other platforms (OSX, *BSD, WSL, etc.), but that hasn't been tested.

### Build dependencies

* GNU make
* gcc
* pkg-config (`pkgconf` in Arch Linux)
* GTK3 (`gtk3-devel` in CentOS)

### Build instructions

A Makefile is provided, build the program using:

```make``` 

The resulting `jf3` executable can be run directly from the command line or your file manager.  Optionally, the program can be installed for all users with:

```sudo make install```

This will place the `jf3` binary in `/usr/bin` and a desktop entry file in `/usr/share/applications` (for desktop environment integration).  These changes can be undone with:

```sudo make uninstall```


## Usage tips

* Preferences are stored in a plaintext configuration file on a per-user basis at `$HOME/.config/jf3/jf3.conf`.
* When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `jf3 /path/to/file1 /path/to/file2`).
* After fitting a spectrum, the onscreen fit can be cleared using the right mouse button.

## Screenshot

![jf3 user interface screenshot](https://raw.githubusercontent.com/e-j-w/e-j-w.github.io/master/media/jf3.png "jf3 user interface")
