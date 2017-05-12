motion: m.o appControlHandler.c	
	cc -o motion -L/homepi/IMU/build/libfreespace/libfreespace.so.0.7.1 -lfreespace -lm -lwiringPi appControlHandler.o m.o
tracking: math/quaternion.o tracking_2.o appControlHandler.o
	cc -o tracking -L/homepi/IMU/build/libfreespace/libfreespace.so.0.7.1 -lfreespace -lm -lpthread -lbluetooth -lwiringPi math/quaternion.o appControlHandler.o tracking_2.o

tracking_2.o: tracking_2.c
	cc -c tracking_2.c

m.o: m.c appControlHandler.h
	cc -c m.c
appControlHandler.o: appControlHandler.h
	cc -c appControlHandler.c
quaternion.o: math/quaternion.c
	cc -c quaternion.c

clean: 
	rm tracking *.o
