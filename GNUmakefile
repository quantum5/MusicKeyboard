ifdef CROSS
KROSS=$(CROSS)-
endif
CC=$(KROSS)gcc
CXX=$(KROSS)g++
RC=$(KROSS)windres

LINK = $(CXX)
CFLAGS = -O3 -Iinclude -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DWINVER=0x0601 -D_WIN32_WINNT=0x0601
RCFLAGS = -Iinclude
LDFLAGS = -s -static -static-libstdc++ -mwindows

FILES=build/Keyboard.o build/MainWindow.o build/Window.o \
      build/PianoControl.o build/midifile.o build/resources.o

all: initdir MusicKeyboard.exe

initdir:
	[ -d build ] || mkdir build

MusicKeyboard.exe: $(FILES)
	$(LINK) $(CFLAGS) $(LDFLAGS) $(FILES) -o $@ -lgdi32 -lwinmm -lcomctl32 -lcomdlg32

include/MainWindow.hpp: include/Window.hpp include/PianoControl.hpp include/midifile.h include/mkntapi.h
include/PianoControl.hpp: include/Window.hpp
include/midifile.h: include/midiinfo.h

build/Keyboard.o: src/Keyboard.cpp include/MainWindow.hpp
build/MainWindow.o: src/MainWindow.cpp include/MainWindow.hpp
build/Window.o: src/Window.cpp include/Window.hpp
build/PianoControl.o: src/PianoControl.cpp include/PianoControl.hpp
build/midifile.o: src/midifile.c include/midifile.h

build/resources.o: keyboard.rc include/resource.h commctrl6.manifest
	$(RC) $(RCFLAGS) $< -o $@

build/%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build/*.o
	rm -f MusicKeyboard.exe
