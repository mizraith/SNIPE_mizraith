I'm putting this here to help others that may be struggling with the same issue. I'm working on identifying a fix for arduino-cmake Downloaded CLion and the CLion-Arduino 1.2.1 plugin (which uses arduino-cmake), but wanted to get down my workaround for others to find.

I'll follow up with a possible workaround, but the crux of it is that the arduino-cmake doesn't seem to be able to process the new boards.txt format when there are core variants. I'm on a Mac (10.11) and have several Arduino SDK installs (1.06, 1.56, 1.6x) under /Applications. [ I truly wish the Arduino maintainers would reign in development efforts so that the SDKs don't all behave so differently and have arbitrarily different internal workings! ]

First Lesson -- arduino-cmake looks for /Applications/Arduino.app So rename your preferred SDK as "Arduino.app". Arduino 1.06 and 1.5+ have a different internal structure...but Arduino-Cmake seem stop handle this okay.

Second Lesson -- I tried at least a dozen different settings (CFLAGS, CXXFLAGS, ARDUINO FLAGS, etc etc), but kept getting a warning the the avr-g++ compiler was not getting the "-mmcu" flag. Sure enough, when you look at the cmake log, the line contains "-mmcu= " (nothing afterwards. Again, tried every setting....but was only able to compile under 1.06.

QUESTION: Is there an Arduino-Cmake setting that allows you to pass in a -mmcu flag to the avr-g++ compiler without deleting the other flags already built up?

Third Lesson -- something about the Clion-Arduino or the Arduino-Cmake process retains too much info, even after a quit. When I found a clear workaround, it was not reloading the boards.txt file. I seem to also not see a result from print_board_list() every time either. This inconsistency makes it very hard to debug. Workaround: I ended up going into the CMakeFiles directory and selectively deleting stuff (your mileage will vary).

Workaround steps to compile for nano328 on Arduino 1.5+
I opened the 1.08 boards.txt file and simply copied the nano328 section into the 1.5+ boards.txt file. It looks like this:

``

nano328b.name=Arduino Nano w/ ATmega328

nano328b.upload.protocol=arduino
nano328b.upload.maximum_size=30720
nano328b.upload.speed=57600

nano328b.bootloader.low_fuses=0xFF
nano328b.bootloader.high_fuses=0xDA
nano328b.bootloader.extended_fuses=0x05
nano328b.bootloader.path=atmega
nano328b.bootloader.file=ATmegaBOOT_168_atmega328.hex
nano328b.bootloader.unlock_bits=0x3F
nano328b.bootloader.lock_bits=0x0F

nano328b.build.mcu=atmega328p
nano328b.build.f_cpu=16000000L
nano328b.build.core=arduino
nano328b.build.variant=eightanaloginputs


Again -- if you don't see "nano328" from your print_board_list() call, then you'll need to do some cleaning of CMakeFiles

With the above workaround, I don't need to edit any of the internal arduino-cmake files and my CMakeLists.txt file looks like:

cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME test_clion3)
project(${PROJECT_NAME})

print_board_list()

set(ARDUINO_SKETCH_FOLDER   /Users/mizraith/Documents/ARDUINO_DEVELOPMENT)
link_directories(${ARDUINO_SKETCH_FOLDER}/libraries )

set(ARDUINO_DEFAULT_BAORD nano328)
set(ARDUINO_DEFAULT_PORT /dev/tty.usbmodem1411)
set (Wire_RECURSE True)


set(${CMAKE_PROJECT_NAME}_SKETCH test_clion3.ino)
generate_arduino_firmware(${CMAKE_PROJECT_NAME}
        BOARD nano328
        PORT /dev/tty.usbmodem1411)