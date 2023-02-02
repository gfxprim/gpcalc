CFLAGS?=-W -Wall -Wextra -O2
CFLAGS+=$(shell gfxprim-config --cflags)
LDLIBS=-lm -lgfxprim $(shell gfxprim-config --libs-widgets)
BIN=gpcalc
DEP=$(BIN:=.dep) expr.dep

all: $(DEP) $(BIN)

$(BIN): expr.o

%.dep: %.c
	$(CC) $(CFLAGS) -M $< -o $@

-include $(DEP)

install:
	install -m 644 -D layout.json $(DESTDIR)/etc/gp_apps/$(BIN)/layout.json
	install -D $(BIN) -t $(DESTDIR)/usr/bin/
	install -D -m 644 $(BIN).desktop -t $(DESTDIR)/usr/share/applications/
	install -D -m 644 $(BIN).png -t $(DESTDIR)/usr/share/gpcalc/
clean:
	rm -f $(BIN) *.dep *.o
