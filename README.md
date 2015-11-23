
This project contains two part:

- An android application that will load a shared library that is going to be attacked/targeted
- A program that will "spy" the shared library for collecting memory reads/accesses timings

The android application can be compiled within Android-studio and the APK can be directly installed
on the device.

The shared library that will be embedded within the application will be called libtarget.so. Source
code is located in app/src/main/jni/target.cpp

The spy program is located in app/src/main/jni/spy.c.

The makefile is located at app/src/main/jni/Android.mk


While compiled, the spy program can be launched like this:

./spy path_to_shared_library output_file

path_to_shared_library == > Generally libraries are dropped at
/data/app-lib/application.package.name/libs/*.so

output_file =====>  results/timings will be stored in this file

