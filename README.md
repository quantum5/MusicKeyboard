# MusicKeyboard [![Jenkins](https://img.shields.io/jenkins/s/https/ci.quantum2.xyz/job/MusicKeyboard.svg)](https://ci.quantum2.xyz/job/MusicKeyboard/) ![GPLv3](https://img.shields.io/badge/license-GPLv3-blue.svg)

This free and open source program emulates a piano, but can use all available MIDI instruments, instead of just the piano. You can use this program to play almost any music you like.

## Features

![MusicKeyboard Screenshot](https://guanzhong.ca/assets/projects/MusicKeyboard-16b281f66a99af3d8849427148fe0a193e3defdef6438912603113af2cd52e5f.png)

* Full chording support in program, but due to the keyboard chording limitations, only 3 - 7 keys may be pressed simultaneously, so your milage may vary.
* Multitouch with hardware support.
* Recording music into a MIDI file.
* Key signature changing.
* Different keyboard layouts.
* Using Windows's builtin beeping functionality when MIDI is unavailable.

## Download

The latest download is available on [Jenkins](https://ci.quantum2.xyz/job/MusicKeyboard/) ([direct link](https://ci.quantum2.xyz/job/MusicKeyboard/lastSuccessfulBuild/artifact/dist/Release/Keyboard.exe)).

## Compliation

Using Visual C++ command line:

```
$ nmake
```

The resulting files will be produced in `dist\Release\Keyboard.exe`.

Using MinGW:

```
$ make
```

The resulting files will be produced in `MusicKeyboard.exe`.

Tested to compile on Visual C++ 2010, 2013, and MinGW.
