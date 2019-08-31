#https://stackoverflow.com/questions/2604398/compile-multiple-c-files-with-make

#https://stackoverflow.com/questions/8482152/whats-the-difference-between-rpath-and-l
#-Wl,-rpath=/opt/dev-tools-sources/ffmpeg/build/lib is required here as LD_LIBRARY_PATH does not contain the path to some shared libraries (.so). rpath is how it's added.
#LD_LIBRARY_PATH is an environment variable that tells the linker at runtime where to look for dynamic libraries (see stackoverflow reference).
# Workaround would be to add the path to that variable: export LD_LIBRARY_PATH=/opt/dev-tools-sources/ffmpeg/build/lib/:$LD_LIBRARY_PATH

#prog: extract_mvs.o save_hdf5.o
#	gcc -o prog extract_mvs.o save_hdf5.o

prog: save_hdf5.o extract_mvs.o
	gcc -o decode_motion save_hdf5.o extract_mvs.o \
	-L/opt/dev-tools-sources/ffmpeg/build/lib -Wl,-rpath=/opt/dev-tools-sources/ffmpeg/build/lib -lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale \
	-L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp \

save_hdf5.o: save_hdf5.cpp
	gcc -c save_hdf5.cpp \
	-I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp

extract_mvs.o: extract_mvs.cpp
	gcc -c extract_mvs.cpp \
	-lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale -L/opt/dev-tools-sources/ffmpeg/build/lib -I/opt/dev-tools-sources/ffmpeg/build/include -ljson -w \
	-I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp \
	-fpermissive -g #Do not show warnings about nonconfomring code

#gcc -o extract_mvs extract_mvs.c -lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale -L/opt/dev-tools-sources/ffmpeg/build/lib -I/opt/dev-tools-sources/ffmpeg/build/include -ljson -w
