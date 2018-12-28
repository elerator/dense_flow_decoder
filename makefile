#https://stackoverflow.com/questions/2604398/compile-multiple-c-files-with-make
#prog: extract_mvs.o save_hdf5.o
#	gcc -o prog extract_mvs.o save_hdf5.o

prog: save_hdf5.o extract_mvs.o
	gcc -o decode_motion save_hdf5.o extract_mvs.o \
	-lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale -L/opt/dev-tools-sources/ffmpeg/build/lib -ljson -w \
	-L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp \

save_hdf5.o: save_hdf5.cpp
	gcc -c save_hdf5.cpp \
	-I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp

extract_mvs.o: extract_mvs.cpp
	gcc -c extract_mvs.cpp \
	-lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale -L/opt/dev-tools-sources/ffmpeg/build/lib -I/opt/dev-tools-sources/ffmpeg/build/include -ljson -w \
	-I /usr/include/hdf5/serial/ -L/usr/lib/x86_64-linux-gnu/hdf5/serial/ -lstdc++ -lhdf5 -lhdf5_cpp \
	-fpermissive -g

#gcc -o extract_mvs extract_mvs.c -lavcodec -lavformat -lavutil -lavdevice -lswresample -lm -lz -lswscale -L/opt/dev-tools-sources/ffmpeg/build/lib -I/opt/dev-tools-sources/ffmpeg/build/include -ljson -w
