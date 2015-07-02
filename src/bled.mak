#
# nmake -f bled.mak

MT=mt
CP=copy
RM=del

INC_DIR=.

CFLAG=/nologo /W3 /DWIN32 /D_WIN32 $(CFLAG) /I $(INC_DIR)
LINKFLAG=/nologo

LIBS=setupapi.lib

default: buildall

buildall:	beacon.exe

clean:
	$(RM) *.obj *.exe *.manifest *.bak

.c.obj:
	$(CC) /c /Fo$@ $< $(CFLAG)

uart.obj:	uart.h

cmd_def.obj:	cmd_def.h

stubs.obj:	cmd_def.h

config_data.obj:	config_data.h

main.obj:	cmd_def.h uart.h

beacon.exe:	main.obj stubs.obj cmd_def.obj uart.obj config_data.obj
	$(CC) /Fe$@ main.obj stubs.obj cmd_def.obj uart.obj config_data.obj $(LINKFLAG) /link $(LIBS)
	IF EXIST $@.manifest $(MT) -manifest $@.manifest -outputresource:$@
