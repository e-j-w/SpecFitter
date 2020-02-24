# **jf3**

**A Spectrum Viewer in GTK3**

Developer/Maintainer: Jonathan Williams

## Description

An attempt at writing a (1D) spectrum viewer app using GTK, with functionality inspired by the `gf3` program in RadWare (https://radware.phy.ornl.gov/).  This is still in the early stages: spectra appear to be drawing correctly but many functions are not usable yet.

## Features

### Support for various file formats

* **.mca** - An .mca file is simply a 2D array of integers, with the first index denoting a spectrum number (up to 100) and the second index denoting a bin number (up to 32768).
* **.fmca** - An .fmca file has the same format as .mca except it uses floats rather than integers.
* **.spe** -  Spectrum format used by the RadWare software package (available at: https://radware.phy.ornl.gov/), for example when using the 'ws' command in `gf3`.

### Various spectrum display options

* Zooming and panning, using the mouse (mouse wheel, click and drag) or manual controls.
* Rebinning of spectra, with results displayed in real time.

## How to install

So far the program has only been tested with Linux-based systems.  A Makefile is provided, use `make` to compile.  This should work fine on most recent Linux distros - the build process has been tested on CentOS 7 and Arch Linux (as of Feb 2020).

### Build dependencies

* GNU make
* gcc
* GTK 3 (`gtk3-devel` in CentOS)
* GLib
* Cairo (used for drawing spectra)

## Future plans

* Plotting of multiple spectra
* Peak labels?
* Peak fitting?
