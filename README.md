# amt

- clone and install required packages
```
sudo apt-get -y update
```
```
sudo apt-get -y upgrade
```
```
sudo apt install -y git vim alsa-utils cpufrequtils libfftw3-dev python3-pip
```
```
git clone https://github.com/mackron/miniaudio.git
```
```
sudo pip3 install --upgrade adafruit-python-shell
```
```
sudo wget https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/master/i2smic.py
```
```
sudo python3 i2smic.py
```

- to set for boot:
```
sudo vim /etc/init.d/cpufrequtils
```
GOVERNOR="powersave"
2B:
MAX_SPEED="600"
MIN_SPEED="600"
pi zero:
MAX_SPEED="700"
MIN_SPEED="700"

- check sound card number:
```
arecord -l
```

- edit:
```
sudo vim /etc/asound.conf
```
pcm.!default {
 format SE32_LE
 rate 48000
 type hw
 card 0
 device 0
}

- build the executable
```
gcc main.c tools/tools.c audio_proc/audio_proc.c -o amt -ldl -lpthread -lm -latomic -lfftw3
```

- test with DEBUG
```
./amt
```

- add to /etc/rc.local (before last exit line):
```
sudo vim /etc/rc.local
```
sleep 45s && sudo ~/amt/amt &
- maybe increase sleep time for 2B
sleep 120s && sudo ~/amt/amt &