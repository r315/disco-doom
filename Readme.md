# DOOM for discovery

Doom for discovery is another port of the game doom to the STM32F769 discovery development kit.
Main goal of this project was to explore doom engine and implement a simple touch controller using the devkit touch screen.

This project based on version 1.08 and is sporadically updated, so expect weird bugs. For a more in-depth I recommend reading Fabien Sanglard's book https://fabiensanglard.net/gebbdoom/


![In game](/doc/photo4.jpg)

## Build

### Requirements

- arm-none-eabi-gcc 9.2.1
- openocd v0.10.0

````
$ cd target/disco
$ make
$ make program
````
## Running

For running the game a wad file is required, shareware version can be used as others.
Command line arguments can be passed by file, create a file named doom.arg on sdcard root and add your arguments as needed, all arguments should be on a single line. Sdcard file structure and example doom.arg can be found on this repository.

Build and program the board with the above commands, if card is inserted the game should start running.

A serial port is available with the game output and errors.

### Controls

Controls are simple touch implementation and a maximum of two points are supported, due to basic implementation these controls are a bit clumsy.

- Left direction pad controls forward, backward, strafeleft, straferight
- Right thumbstick controls left and right
- B fire
- A use
- USER Button opens menu

These controls are simple bmp files and can be customized, however bit per pixel should be kept as the original file. 

### Parameters

Added parameters to exiting ones

"-basedir \<directory>" this command is similar to quake command to specify a directory were to find wad files.

"-bgcolor \<AARRGGBB>" specifies background fill color for display in hexadecimal
