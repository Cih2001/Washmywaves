WASHMYWAVES
===========
washmywaves is a tool to convert wav files to mp3 format. It uses *lame* encoding library under the hood.

1. washmywaves is platform independent. It can be compiled in Windows/Linux/Mac. However, it is only tested in Linux.
2. Multi-threading is implemented using Linux *pthread*. However, there exists a wrapper in Windows that implements *pthread*, so there should be no problem porting washmywaves to Windows.
3. Supported wave files:
   1. Integer PCM with bits-width between 8 to 32. (tested widths: 8, 12, 16, 24, 32). Even custom widths should work fine, such as 7-bits samples.
   2. IEEE Float PCM.
4. Should be compiled with C++17 or higher version.

##Usage:
```bash
./washmywaves path/to/directory/containing/wav/files
```

##Notes on implementation:
1. It is an IO dependant user-mode application, and it's best to rely on kernel for thread scheduling. For each wav file, we create a separate thread. We do not care how many cores exist on the cpu and let the kernel handle multitasking. If there are enough cores available, each thread will be run on a separate core.
2. For enumorating files inside a directory, we rely on `std::filesystem`. This has become a part of standard library since C++17 and ensures protability of the source code.
3. Lame encoding library is linked statically.
4. Makefile is created using GNU Make. There are some steps in make file that rely on tools which do not exist on Windows by default, such as `grep` and `find`. Altough the code should be portable, it is only tested on Linux Ubuntu 20.04. To compile it on Windows, some additional steps might be required.
