.PHONY: all c clean u upload m monitor s snapshot

FIRMWARE_DIR = "./DJC-Firmware"

all:
	pio run -d $(FIRMWARE_DIR)

clean:
	pio run -d $(FIRMWARE_DIR) -t clean

c: clean

upload:
	pio run -d $(FIRMWARE_DIR) -t upload

u: upload

monitor:
	pio device monitor -b 115200 --no-reconnect

m: monitor

snapshot:
	python ./snapshot.py

s: snapshot