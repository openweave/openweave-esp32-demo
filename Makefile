#
#    Copyright (c) 2018 Nest Labs, Inc.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#    Description:
#      Top-level makefile for building the OpenWeave ESP32 demo application using
#      the Espressif ESP-IDF development environment.
#

# Project Name
PROJECT_NAME            := openweave-esp32-demo

# Location of the openweave-esp32-lwip source tree.
#
# OpenWeave requires an enhanced version of the LwIP library that comes with the ESP-IDF.
# development environment.  This version, known as openweave-esp32-lwip, is incorprated
# into the demo application as a git submodule.
#
# Because of the way the OpenWeave code builds, the component.mk file for OpenWeave
# needs to know the location of openweave-esp32-lwip within the overall project source
# tree.  To achieve this we use the LWIP_COMPONENT_DIR variable to convey the location
# of openweave-esp32-lwip to the included OpenWeave component makefile. 
#  
LWIP_COMPONENT_DIR       = $(PROJECT_PATH)/third_party/lwip
export LWIP_COMPONENT_DIR

# A list of directories containing third-party (i.e. external) components that are used
# by the demo application.  These are incorporated as git submodules in the source tree.
EXTRA_COMPONENT_DIRS    += $(PROJECT_PATH)/third_party/openweave/build/esp32/components \
                           $(PROJECT_PATH)/third_party/ESP32_TFT_library/components \
                           $(PROJECT_PATH)/third_party/QRCode \
                           $(LWIP_COMPONENT_DIR)

# The list of ESP-IDF and external software components used by the demo application. 
COMPONENTS              := app_trace \
                           app_update \
                           aws_iot \
                           bootloader \
                           bootloader_support \
                           bt \
                           console \
                           cxx \
                           driver \
                           esp32 \
                           esp_adc_cal \
                           esp_event \
                           esptool_py \
                           ethernet \
                           expat \
                           fatfs \
                           freertos \
                           heap \
                           jsmn \
                           log \
                           lwip \
                           main \
                           mbedtls \
                           micro-ecc \
                           newlib \
                           nvs_flash \
                           partition_table \
                           pthread \
                           QRCode \
                           esp_ringbuf \
                           sdmmc \
                           smartconfig_ack \
                           soc \
                           spidriver \
                           spi_flash \
                           tft \
                           tcpip_adapter \
                           ulp \
                           vfs \
                           wear_levelling \
                           openweave \
                           wpa_supplicant \
                           xtensa-debug-module \

include $(IDF_PATH)/make/project.mk

