ARCH = `uname -snrm`
RELEASE = v0.0.1
VERSION = `git rev-parse --short HEAD 2&>/dev/null || echo $(RELEASE)`

OBJECTS = util.o

LIBS = -lcrypto

all: hashcheck

hashcheck: hashcheck.c $(OBJECTS)
	@echo Building hashcheck $(VERSION) on $(ARCH) 
	$(CC) -Wall -ansi -DVERSION=\"$(VERSION)\" -pedantic -o hashcheck $(LIBS) hashcheck.c $(OBJECTS)

.PHONY: clean
clean:
	rm hashcheck *.o
