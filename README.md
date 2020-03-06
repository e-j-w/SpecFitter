# **jf3**

**A Spectrum Viewer in GTK3**

## Description

A (1D) spectrum viewer app using GTK3, with functionality inspired by the `gf3` program for gamma-ray spectroscopy in the RadWare software package (https://radware.phy.ornl.gov/).

NOTE: This is still in the early stages - spectra appear to be drawing correctly but some functions are not usable yet.

## Features

### Support for various file formats

* **.mca** - An .mca file is simply a 2D array of integers, with the first index denoting a spectrum number (array length up to 100) and the second index denoting a bin number (array length 32768 ie. 2<sup>15</sup>).
* **.fmca** - An .fmca file has the same format as .mca except it uses floats rather than integers.
* **.spe** -  Spectrum format used by the RadWare software package (available at: https://radware.phy.ornl.gov/), for example when using the 'ws' command in `gf3`.

### Various spectrum display options

* Import spectra from multiple files and plot multiple spectra simultaneously:
    * Sum spectra together.
    * Overlay spectra, with either common or independent scaling on the y-axis.
    * Show spectra in a "stacked" view (tiled vertically with a common x-axis).
* Zoom and pan using the mouse (mouse wheel, click and drag).
* Rebin spectra, with results displayed in real time.

### Cool user interface

* Designed to be as simple and intuitive as possible given the supported featureset.

## How to install

So far the program has only been tested with Linux-based systems.  A Makefile is provided, use `make` to compile.  This should work fine on most recent Linux distros - the build process has been tested on CentOS 7 and Arch Linux (as of Feb 2020).

### Build dependencies

* GNU make
* gcc
* GTK 3 (`gtk3-devel` in CentOS)

## Tips and tricks

When running the program from the command line, it is possible to automatically open files by specifying the filename(s) as arguments (eg. `jf3 /path/to/file1 /path/to/file2`).


## Future plans

* Peak fitting
* Peak labels?

## Credits

Developer/Maintainer: Jonathan Williams
