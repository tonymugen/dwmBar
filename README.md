# Overview

`dwmbar` is a status bar for [dwm](https://dwm.suckless.org/) similar to [dwmblocks](https://github.com/torrinfail/dwmblocks). I wrote it in C++ just to troll the [suckless](https://suckless.org/sucks/) people. It has some built-in modules, but can also be extended with external scripts.

Each module can be set to update after a separate interval. Modules run as separate threads and alert the main thread to print to the root window when a change occurs. You can also run a module by issuing a real-time signal with `pkill`, e.g.

	pkill --signal RTMIN+1 -x dwmbar

The signal ID is set per module during configuration (see below). Modules that are running on a schedule can still be activated by a signal.

`dwm` supports two status bars (bottom and top) if you have the `dwm-extrabar` patch.

# Install

To install clone this repository and use `make`:

	cd dwmBar
	make
	sudo make install

This will put the `dwmbar` binary in `/usr/local/bin/` and assumes gcc is the compiler on the system. If you have llvm instead, use

	make CXX=c++

# Configure

`dwmbar` is configured by editing the `config.hpp` file. Comments within the file explain what to do and the available options. If you want to customize further, full interface documentation is [here](https://tonymugen.github.io/dwmBar), or you can run `doxygen` in the source code directory.
