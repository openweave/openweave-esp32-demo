#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := weave-esp32-test

COMPONENTS :=                     \
app_trace                         \
app_update                        \
aws_iot                           \
bootloader                        \
bootloader_support                \
bt                                \
console                           \
cxx                               \
driver                            \
esp32                             \
esp_adc_cal                       \
esptool_py                        \
ethernet                          \
expat                             \
fatfs                             \
freertos                          \
heap                              \
jsmn                              \
log                               \
lwip                              \
main                              \
mbedtls							  \
newlib                            \
nvs_flash                         \
partition_table                   \
sdmmc                             \
soc                               \
spi_flash                         \
spiffs                            \
tcpip_adapter                     \
ulp                               \
vfs                               \
wear_levelling                    \
weave                             \
wpa_supplicant                    \
xtensa-debug-module               \
pthread							  \
weave-platform-esp32			  \


include $(IDF_PATH)/make/project.mk

