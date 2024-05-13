# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Raspberry/pico/pico-sdk/tools/pioasm"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pioasm"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/IAR_Projects/STM32F4xx/w25qxx/Examples/RP2040/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
