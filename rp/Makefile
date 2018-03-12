all:
#	g++ -c -I/usr/include/opencv hdw.cpp -o hdw.o
#	g++ -lopencv_core -lopencv_highgui -lopencv_imgproc -lwiringPi -ldl mountcar.cpp hdw.o -o mountcar
	gcc -Wall -fPIC rlalg.c -shared -Wl,-soname,librlalg.so -o librlalg.so 


clean:
	rm mountcar
	rm *.so *.o
