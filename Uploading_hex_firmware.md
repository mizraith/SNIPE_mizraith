# Loading .hex Without Compiling
*The compiled hex file for an ArduinoNano is included.  Others could readily be complied as needed (since I'm using PlatformIO).  But how to upload them?*
* Download and install Arduino CLI
    * https://arduino.github.io/arduino-cli/0.35/installation/
* Create a config file, <optional> edit if you feel you need to.
    * arduino-cli config init
*  optional: Update the local cache of platforms and devices (not necessary every time)
    *  arduino-cli core update-index
* Find the boards.
    *	arduino-cli board list
    *   arduino-cli board listall mkr  (this didn't work for me or for older arduinos or clones)

* UPLOAD (if you know the port of your board)
    * NEWER arduino nanos use:
        * arduino-cli upload --verbose -p COM11 -b arduino:avr:nano -i C:\path\to\firmware.hex

    * OLDER bootloader nanos use:
        * arduino-cli upload --verbose -p COM11 -b arduino:avr:nano:cpu=atmega328old -i C:\path\to\firmware.hex
		
		
		
		