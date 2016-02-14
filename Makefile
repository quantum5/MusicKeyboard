SRCDIR=src
INCDIR=include

CXX=cl /nologo
LD=link /nologo
CXXFLAGS=/c /O1 /I$(INCDIR) /W4 /Zi /DWIN32_LEAN_AND_MEAN /DWINVER=0x0601 /D_WIN32_WINNT=0x0601 /DUNICODE /D_UNICODE
LDFLAGS=/subsystem:windows /debug /incremental:no /opt:REF
RC=rc /nologo
RCFLAGS=/i$(INCDIR)
LIBS=

!IFDEF DEBUG
BUILD=Debug
CXXFLAGS=$(CXXFLAGS) /DDEBUG
LDFLAGS=$(LDFLAGS)
!ELSE
BUILD=Release
LDFLAGS=$(LDFLAGS)
!ENDIF

OUTDIR=build\$(BUILD)
DISTDIR=dist\$(BUILD)
FILES=$(OUTDIR)\Keyboard.obj \
      $(OUTDIR)\MainWindow.obj \
      $(OUTDIR)\PianoControl.obj \
      $(OUTDIR)\Window.obj \
      $(OUTDIR)\midifile.obj \
      $(OUTDIR)\keyboard.res

all: initdir $(DISTDIR)\Keyboard.exe

upx: all
	upx --best $(DISTDIR)\Keyboard.exe

initdir:
	@if not exist build md build
	@if not exist $(OUTDIR) md $(OUTDIR)
	@if not exist build md dist
	@if not exist $(DISTDIR) md $(DISTDIR)

$(INCDIR)\MainWindow.hpp: $(INCDIR)\Window.hpp $(INCDIR)\PianoControl.hpp $(INCDIR)\midifile.h $(INCDIR)\mkntapi.h
$(INCDIR)\midifile.h: $(INCDIR)\midiinfo.h

$(SRCDIR)\MainWindow.cpp: $(INCDIR)\MainWindow.hpp
$(SRCDIR)\PianoControl.cpp: $(INCDIR)\PianoControl.hpp
$(SRCDIR)\Keyboard.cpp: $(INCDIR)\MainWindow.hpp
$(SRCDIR)\Window.cpp: $(INCDIR)\Window.hpp
$(SRCDIR)\midifile.c: $(INCDIR)\midifile.h
keyboard.rc: keyboard.ico

$(OUTDIR)\keyboard.res: keyboard.rc
	$(RC) $(RCFLAGS) /fo$@ $**

{$(SRCDIR)}.cpp{$(OUTDIR)}.obj::
	$(CXX) $(CXXFLAGS) /Fo$(OUTDIR)\ /Fd$(OUTDIR)\ $<

{$(SRCDIR)}.c{$(OUTDIR)}.obj::
	$(CXX) $(CXXFLAGS) /Fo$(OUTDIR)\ /Fd$(OUTDIR)\ $<

$(DISTDIR)\Keyboard.exe: $(FILES)
	$(LD) /out:$@ $(LDFLAGS) $** $(LIBS)
