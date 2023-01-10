# DOOM for discovery

Doom for discovery is another port of the game doom to the STM32F769 discovery development kit.
This still is a wip project, at this stage it runs but has many bugs


![In game](/doc/photo1.JPG)

![I2C Controller](/doc/photo2.JPG)

![Running demo](/doc/photo3.JPG)

## Build

### Requirements

- arm-none-eabi-gcc 9.2.1
- openocd v0.10.0

````
$ cd target/disco
$ make
````
## Running

Build and program the board with the above commands then copy a shareware doom1.wad file to sdcard and insert card on the board. After resetting the board the game should start and audio is available on the headset jack.

### Controls

For this demo, a I2C controller was put together and connected to AUX I2C bus (Pins D14 and D15).
<pre>
              +-----------------+ 
I2C ==========|              IO0|- UP
              |              IO1|- DOWN
              |              IO2|- LEFT
              |              IO3|- RIGTH
              |  PCF8574     IO4|- ENTER
              |              IO5|- FIRE
              |              IO6|- Escape
              |              IO7|- Use
              +-----------------+
              
</pre>

A driver for LIS302 accelerometer is also available but doesn't work very well.
