MAIN = plugin-img

GIMPTOOL = gimptool-2.0
GIMP = gimp
GIMPCONSOLE = gimp-console
CC = cc
LD = cc
CFLAGS = -O2 -Wall -Wextra

SRC = img-load.c img-save.c img-save-dialog.c plugin-img.c
HDR = $(SRC:.c=.h)
OBJ = $(SRC:.c=.o)

$(MAIN): $(OBJ)
	$(LD) $(OBJ) `$(GIMPTOOL) --libs` -o $@

$(OBJ): $(HDR) Makefile

.c.o:
	$(CC) `$(GIMPTOOL) --cflags` $(CFLAGS) -c $< -o $@

.PHONY: view
view: install
	$(GIMP) samples/*.img

.PHONY: test
test: test.scm install
	rm -f samples/success
	$(GIMPCONSOLE) --no-interface --batch - < $<
	test -f samples/success

.PHONY: install
install: $(MAIN)
	$(GIMPTOOL) --install-bin $(MAIN)

.PHONY: uninstall
uninstall:
	$(GIMPTOOL) --uninstall-bin $(MAIN)

.PHONY: clean
clean:
	rm -f *.o *.i *.s $(MAIN)
	rm -f samples/recoded-*
	rm -f samples/success

