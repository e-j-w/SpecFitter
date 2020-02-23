# **jf3**

**Spectrum viewer**

## Description

An attempt at writing a (1D) spectrum viewer app using GTK, with functionality inspired by the `gf3` program in RadWare (https://radware.phy.ornl.gov/info.html).  This is still in the early stages: spectra appear to be drawing correctly but many functions are not usable yet.

## How to install

A Makefile is provided, use `make` to compile.  This should work fine on most recent Linux distros - the build process has been tested on CentOS 7 and Arch Linux (as of Feb 2020).

### Build dependencies

* GNU make
* gcc
* GTK 3 (`gtk3-devel` in CentOS)
* GLib
* Cairo (used for drawing spectra)

## Future plans

* Contracting spectra
* Peak labels?
* Peak fitting?
