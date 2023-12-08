# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/tommaso/esp/esp-idf/components/bootloader/subproject"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/tmp"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/src/bootloader-stamp"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/src"
  "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/tommaso/esp/workspaces/idf-5.1.2_wss/esp32_mecanum/ESP32_CAM/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
