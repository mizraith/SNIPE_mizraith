cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set( PROJECT_NAME SNIPE_mizraith )
project(${PROJECT_NAME})

print_board_list()



set(ARDUINO_SKETCH_FOLDER   /Users/red/Documents/Computer_Science/ARDUINO_DEVELOPMENT)
link_directories(${ARDUINO_SKETCH_FOLDER}/libraries )


set(ARDUINO_DEFAULT_BAORD nano328)
#set(ARDUINO_DEFAULT_PORT /dev/tty.usbmodem1411)
set (Wire_RECURSE True)

set(${CMAKE_PROJECT_NAME}_SKETCH SNIPE_mizraith.ino)
generate_arduino_firmware(${CMAKE_PROJECT_NAME}
        BOARD nano328
        PORT /dev/tty.usbserial-A702Y23H)
