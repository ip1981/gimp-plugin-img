MAIN = plugin-img

GIMPTOOL = gimptool-2.0
CC = gcc
LD = gcc
CFLAGS = $(shell $(GIMPTOOL) --cflags) \
		 -O2 -Wall -Wextra -Wno-attributes \
		 -Wno-unused-parameter

LIBS = $(shell $(GIMPTOOL) --libs)

SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
OBJ = $(subst .c,.o,$(SRC))

build: $(MAIN)

help:
	@echo "MAKE targets:"
	@echo ""
	@echo "build   - build plugin"
	@echo "install - install plugin"
	@echo ""
	@echo "indent  - beatify sources"
	@echo "clean   - remove build garbage"
	@echo ""
	@echo "test    - run gimp with samples"
	@echo "test-fu - run non-interactively gimp for batch file recoding"


install: $(MAIN)
	$(GIMPTOOL) --install-bin $(MAIN)

indent: $(SRC) $(HDR)
	indent $(INDENT_OPT) $?

clean:
	rm -f *.o  *.i *.s $(MAIN)

$(MAIN): $(OBJ)
	$(LD) $(OBJ) $(LIBS) -o $@

%.o: %.c $(HDR) Makefile
	$(CC) -c $(CFLAGS) $< -o $@


test: install
	gimp samples/*.img

test-fu: install img-fu
	gimp  -i -b - < img-fu

.PHONY: install build test test-fu indent clean

