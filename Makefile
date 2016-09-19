SERIAL ?= /dev/ttyUSB0
CODESTAND_HOST ?= codestand.io
CODESTAND_PORT ?= 80

TEMP_WIFI_SSID     = __WIFI_SSID__REPLACE_ME__
TEMP_WIFI_PASSWORD = __WIFI_PASSWORD__REPLACE_ME__
TEMP_DEVICE_NAME   = __DEVICE_NAME__REPLACE_ME__

CC      = xtensa-lx106-elf-gcc
CXX     = xtensa-lx106-elf-g++
AR      = xtensa-lx106-elf-ar
LINK    = xtensa-lx106-elf-gcc
OBJCOPY = xtensa-lx106-elf-objcopy
ESPTOOL = ./esptool-ck-0.4.9/esptool
CMDECHO = printf "  \e[1;35m%s\t\t\e[0;33m%s\e[m\n"

COMMON = \
    -Isdk/include \
    -Isdk/lwip/include \
    -Icores/esp8266 \
    -Ivariants/generic \
    -Icore \
    -Ilibraries/ESP8266WiFi/src \
    -DCODESTAND_HOST='"$(CODESTAND_HOST)"' \
    -DCODESTAND_PORT=$(CODESTAND_PORT) \
    -DDEVICE_NAME='"$(TEMP_DEVICE_NAME)"' \
    -DWIFI_SSID='"$(TEMP_WIFI_SSID)"' \
    -DWIFI_PASSWORD='"$(TEMP_WIFI_PASSWORD)"' \
    -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ \
    -DF_CPU=80000000L -DLWIP_OPEN_SRC  -DDEBUG_ESP_CORE -DDEBUG_ESP_SSL \
    -DDEBUG_ESP_WIFI -DDEBUG_ESP_HTTP_CLIENT -DDEBUG_ESP_HTTP_UPDATE -DDEBUG_ESP_HTTP_SERVER -DDEBUG_ESP_UPDATER \
    -DDEBUG_ESP_OTA -DDEBUG_TLS_MEM -DARDUINO=10609 -DARDUINO_ESP8266_ESP01 -DARDUINO_ARCH_ESP8266 \
    -DARDUINO_BOARD="ESP8266_ESP01" -DESP8266 \
    -Wall -g3 -Os -mlongcalls -mtext-section-literals \
    -falign-functions=4 -ffunction-sections -fdata-sections

CFLAGS   = $(COMMON) -std=gnu99
CXXFLAGS = $(COMMON) -std=c++11 -fno-exceptions -fno-rtti
LINKFLAGS = $(COMMON) -nostdlib -Wl,--no-check-sections -u call_user_start \
    -Wl,--gc-sections -Wl,-wrap,system_restart_local -Wl,-wrap,register_chipv6_phy \
    -Wl,-static -Lsdk/lib -Lsdk/ld -Tfirmware.lds -Wl,-Map=firmware.map

OBJS = \
    setup.S.o \
    main.cpp.o \
    elf.cpp.o \
    aligned_memcpy.cpp.o \
    cores/esp8266/cont.S.o \
    cores/esp8266/cont_util.c.o \
    cores/esp8266/core_esp8266_eboot_command.c.o \
    cores/esp8266/core_esp8266_flash_utils.c.o \
    cores/esp8266/core_esp8266_i2s.c.o \
    cores/esp8266/core_esp8266_noniso.c.o \
    cores/esp8266/core_esp8266_phy.c.o \
    cores/esp8266/core_esp8266_postmortem.c.o \
    cores/esp8266/core_esp8266_si2c.c.o \
    cores/esp8266/core_esp8266_timer.c.o \
    cores/esp8266/core_esp8266_wiring.c.o \
    cores/esp8266/core_esp8266_wiring_analog.c.o \
    cores/esp8266/core_esp8266_wiring_digital.c.o \
    cores/esp8266/core_esp8266_wiring_pulse.c.o \
    cores/esp8266/core_esp8266_wiring_pwm.c.o \
    cores/esp8266/core_esp8266_wiring_shift.c.o \
    cores/esp8266/heap.c.o \
    cores/esp8266/libc_replacements.c.o \
    cores/esp8266/time.c.o \
    cores/esp8266/uart.c.o \
    cores/esp8266/libb64/cdecode.c.o \
    cores/esp8266/libb64/cencode.c.o \
    cores/esp8266/spiffs/spiffs_cache.c.o \
    cores/esp8266/spiffs/spiffs_check.c.o \
    cores/esp8266/spiffs/spiffs_gc.c.o \
    cores/esp8266/spiffs/spiffs_hydrogen.c.o \
    cores/esp8266/spiffs/spiffs_nucleus.c.o \
    cores/esp8266/umm_malloc/umm_malloc.c.o \
    cores/esp8266/Esp.cpp.o \
    cores/esp8266/FS.cpp.o \
    cores/esp8266/HardwareSerial.cpp.o \
    cores/esp8266/IPAddress.cpp.o \
    cores/esp8266/MD5Builder.cpp.o \
    cores/esp8266/Print.cpp.o \
    cores/esp8266/Schedule.cpp.o \
    cores/esp8266/Stream.cpp.o \
    cores/esp8266/StreamString.cpp.o \
    cores/esp8266/Tone.cpp.o \
    cores/esp8266/Updater.cpp.o \
    cores/esp8266/WMath.cpp.o \
    cores/esp8266/WString.cpp.o \
    cores/esp8266/abi.cpp.o \
    cores/esp8266/base64.cpp.o \
    cores/esp8266/cbuf.cpp.o \
    cores/esp8266/core_esp8266_main.cpp.o \
    cores/esp8266/debug.cpp.o \
    cores/esp8266/pgmspace.cpp.o \
    cores/esp8266/spiffs_api.cpp.o \
    cores/esp8266/spiffs_hal.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFi.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFiAP.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFiGeneric.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFiMulti.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFiSTA.cpp.o \
    libraries/ESP8266WiFi/src/ESP8266WiFiScan.cpp.o \
    libraries/ESP8266WiFi/src/WiFiClient.cpp.o \
    libraries/ESP8266WiFi/src/WiFiClientSecure.cpp.o \
    libraries/ESP8266WiFi/src/WiFiServer.cpp.o \
    libraries/ESP8266WiFi/src/WiFiUdp.cpp.o

LIBS = \
    libs/libgcc.a \
    sdk/libc/xtensa-lx106-elf/lib/libc.a \
    sdk/libc/xtensa-lx106-elf/lib/libm.a \
    sdk/lib/libhal.a \
    sdk/lib/libphy.a \
    sdk/lib/libpp.a \
    sdk/lib/libnet80211.a \
    sdk/lib/libwpa.a \
    sdk/lib/libcrypto.a \
    sdk/lib/libmain.a \
    sdk/lib/libwps.a \
    sdk/lib/libaxtls.a \
    sdk/lib/libsmartconfig.a \
    sdk/lib/libmesh.a \
    sdk/lib/libwpa2.a \
    sdk/lib/liblwip_gcc.a \
    sdk/lib/libstdc++.a

$(VERBOSE).SILENT:
.SUFFIXES:
.PHONY: build clean flash

build: firmware.bin

clean:
	rm -f $(OBJS) firmware.bin firmware.elf firmware.map

flash: firmware.bin
	$(ESPTOOL) -vv -cd ck -cb 115200 -cp $(SERIAL) -ca 0x00000 -cf firmware.bin
	./console.py $(SERIAL)


firmware.bin: firmware.elf
	$(CMDECHO) ESPTOOL $@
	$(ESPTOOL) -eo bootloaders/eboot/eboot.elf \
	           -bo firmware.bin -bm dio -bf 40 -bz 512K -bs .text -bp 4096 -ec \
	           -eo firmware.elf -bs .irom0.text -bs .text -bs .data -bs .rodata -bc -ec

firmware.elf: $(OBJS) $(LIBS) firmware.lds Makefile
	$(CMDECHO) LINK $@
	$(LINK) $(LINKFLAGS) -o $@ -Wl,--start-group $(OBJS) $(LIBS) -Wl,--end-group

$(ESPTOOL):
	$(MAKE) -C esptool-ck-0.4.9

%.cpp.o: %.cpp
	$(CMDECHO) CXX $@
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.S.o: %.S
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<

%.c.o: %.c
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<
