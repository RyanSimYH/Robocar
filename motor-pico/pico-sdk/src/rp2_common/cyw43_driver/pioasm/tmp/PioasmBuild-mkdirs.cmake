# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/voidky/Documents/Development/pico-sdk/tools/pioasm"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pioasm"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/tmp"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src"
  "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/voidky/Documents/GitHub/Robocar/barcode/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
