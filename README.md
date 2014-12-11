AudioWalkera
============
Decodes PCM format data from trainer port of Walkera WK-0701 type transmitter via line-in/mic. Uses decoding info from [here](http://www.smartpropoplus.com/Docs/Walkera_Wk-0701_PCM.pdf).

Currently has linux uinput joystick emulation with pretty random channels and hardcoded calibration. Need to copy udev rule 40-uinput-joystick.rules to /etc/udev/rules.d/ or similar.


### Requires:

* alsa-lib-devel/libasound2-dev

### Usage:

Ensure correct audio input selected and volume set.

```
make
sudo modprobe uinput
./audiowalkera
```
