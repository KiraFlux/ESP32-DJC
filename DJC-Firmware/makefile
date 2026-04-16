.PHONY: all c clean u upload m monitor

all:
	pio run

clean:
	pio run -t clean

c: clean

upload:
	pio run -t upload

u: upload

monitor:
	pio device monitor --no-reconnect 

m: monitor
