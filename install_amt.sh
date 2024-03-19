# check if script is runnning as root 
if [ "$(id -n -u)" != "root" ] ; then
    echo "ERROR: The current sh script needs to be executed as ROOT!"
    exit 1
fi

# check if script is runnning using sudo 
if [ ! "$SUDO_USER" -o "$SUDO_USER" == "root" ] ; then
    echo "ERROR: Run this script as sudo bash $(basename $0)"
    exit 1
fi

# update and upgrade first
apt-get -y update
apt-get -y upgrade

# # install packages
apt install -y vim alsa-utils cpufrequtils libfftw3-dev python3-pip

# # copy sound configuration file to /etc/
scp asound.conf /etc/

# # go to /home/pi/ and clone miniaudio
cd ..
git clone https://github.com/mackron/miniaudio.git

# # install adafruit python packages
pip3 install --upgrade adafruit-python-shell
wget https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/master/i2smic.py
python3 i2smic.py



