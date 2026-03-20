t ?= esp32

TERMINAL_SPEED := 115200
# SimpleTimer ya está en inc/ — NO incluir en LIBRARIES
LIBRARIES = Adafruit_GFX Adafruit_BusIO Wire
CPPFLAGS = -DSIMPLE_TIMER_MICROS -DDEBUGGING=0x00 -DTERMINAL_SPEED=$(TERMINAL_SPEED)

ifeq ($t, esp32)
UploadSpeed := 921600
LIBRARIES += FS SPIFFS

ifeq ($b, vga32)
BOARD := esp32dev
SERIAL_PORT := /dev/ttyUSB0
CPPFLAGS += -DVGA_RESOLUTION=MODE400x300 -DROM_SET=series2
LIBRARIES += ESP32Lib

else ifeq ($b, lilygo)
BOARD := ttgo-t7-v14-mini32
SERIAL_PORT := /dev/ttyACM0
CPPFLAGS += -DVGA_RESOLUTION=MODE400x300 -DROM_SET=series4
LIBRARIES += ESP32Lib

else
$(error "Define target: make t=esp32 b=vga32")
endif
endif

include $t.mk
