
The current code has been tested under Arch Linux and Debian 12 as of September 2025, though most recent Linux distros should work as well. Non-Linux platforms are not supported.

## Manual build


### Get build dependencies

* make
* gcc
* pkg-config
* GTK3

In Ubuntu / Debian:

```
apt install build-essential libgtk-3-dev
```

In Arch Linux:

```
pacman -S gcc make pkgconf gtk3
```

In CentOS:

```
yum install gcc gtk3-devel
```

### Build instructions

A Makefile is provided, build the program using:

```make -j``` 

The resulting `specfitter` executable can be run directly from the command line or your file manager.  Optionally, the program can be installed for all users with:

```sudo make install```

This will place the `specfitter` binary in `/usr/bin`, a data-type definition file in `/usr/share/mime/packages` (to allow opening compatible files directly from the file manager), and a desktop entry file in `/usr/share/applications` (for desktop environment integration).  These changes can be undone with:

```sudo make uninstall```

## Flatpak

This will build a sandboxed [Flatpak](https://flatpak.org/) package, which can then be installed on any Linux machine which already has `flatpak`.

### Get Flatpak build dependencies

On Arch Linux:

```
sudo pacman -Syu flatpak git
```

On Debian/Ubuntu:

```
sudo apt install flatpak git
```

### Build and install

Setup `flatpak` with the Flathub repo and get the build metadata (from [this](https://github.com/e-j-w/SpecFitter-flatpak) repo):

```
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
sudo flatpak install -y flathub org.flatpak.Builder
git clone https://github.com/e-j-w/SpecFitter-flatpak
cd SpecFitter-flatpak
```

Build the application (if this command fails with a free disk space error, run `ostree --repo=repo config set core.min-free-space-percent 0` and then try again):

```
flatpak run org.flatpak.Builder --force-clean --sandbox --user --install-deps-from=flathub --disable-rofiles-fuse --ccache --repo=repo flatpak_build io.github.e_j_w.SpecFitter-master.yml
```

Then package the application into a [single file bundle](https://docs.flatpak.org/en/latest/single-file-bundles.html):

```
flatpak build-bundle repo SpecFitter.flatpak io.github.e_j_w.SpecFitter
```

The `SpecFitter.flatpak` bundle produced here can be used to install the application on any Linux machine with `flatpak`.  To install, run:

```
flatpak install SpecFitter.flatpak
```

Clean up the build to save disk space (optional):

```
rm -rf flatpak_build .flatpak-builder repo
```

The "SpecFitter" application should now be available in your application menu and/or via the application search interface (depending on your desktop environment). Or you can run the application directly from the terminal:

```
flatpak run io.github.e_j_w.SpecFitter
```

To uninstall the application, run:

```
flatpak uninstall io.github.e_j_w.SpecFitter
```

Have fun!
