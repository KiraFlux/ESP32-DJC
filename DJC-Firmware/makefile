.PHONY: all c clean u upload m monitor

all:
	pio run

clean:
	pio run --target clean

c: clean

upload:
	pio run --target upload

u: upload

monitor:
	pio device monitor

m: monitor

s:
	python ./snapshot.py