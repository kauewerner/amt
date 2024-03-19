# amt


## Introduction

This software tool was designed initially to be used in a low cost remote audio recording unit. The following hardware/software components are part of this unit where the tool was initially tested:
- [Raspberry Pi Zero/Zero W](https://www.raspberrypi.com/products/raspberry-pi-zero/)
- [INMP441](https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf) MEMS I2S [microphone module](https://www.bitsandparts.nl/INMP441-MEMS-I2S-Microfoon-module-p1924326?gad_source=1)
- USB Powerbank (> 10000mAh)

## Installation

- install Raspberry Pi OS (Legacy, 32-bit) Lite using the [Raspberry Pi Imager](https://www.raspberrypi.com/software/) in an SD card (> 4GB)
- install git:
```
sudo apt install -y git
```
- clone the repository of this project:
```
git clone https://github.com/kauewerner/amt.git
```
- run the bash installation script:
```
cd amt
sudo bash install_amt.sh
```
this will take a long time and the last step will be related to the installation of the [adafruit I2S package](https://makersportal.com/blog/recording-stereo-audio-on-a-raspberry-pi), which leads to keyboard input from terminal required by the user in two parts as shown below, where the last part will lead to a reboot:
```
This script downloads and installs
I2S microphone support.

RASPBERRY_PI_ZERO_W detected.

Auto load module at boot? [y/n] y

...

install -m644 -b -D snd-i2smic-rpi.ko /lib/modules/6.1.21+/kernel/sound/drivers/snd-i2smic-rpi.ko
depmod -a
DONE.

Settings take effect on next boot.

REBOOT NOW? [Y/n]
```
- after the reboot is finished, set the CPU to run with the lowest clock by opening:
```
sudo vim /etc/init.d/cpufrequtils
```
and edit the following variables
```
GOVERNOR="powersave"
MAX_SPEED="700"
MIN_SPEED="700"
```
## Building

Final steps are related to building the amt executable:
- to build the executable
```
gcc main.c tools/tools.c audio_proc/audio_proc.c -o amt -ldl -lpthread -lm -latomic -lfftw3
```
- to build the executable for debugging with gdb
```
gcc -g main.c tools/tools.c audio_proc/audio_proc.c -o amt -ldl -lpthread -lm -latomic -lfftw3
```
- in order to have a quick debug test (without gdb) with printed messages one can use the DEBUG define which can be enabled in config_defines.h and rebuild
- in order to set the executable to always start with boot (running as root), one can open the following file:
```
sudo vim /etc/rc.local
```
and add the following line before the last exit 0 line:
```
sleep 45s && sudo /home/pi/amt/amt &
```
this will make sure that the amt will run as root in the background everytime the RPI is powered on.