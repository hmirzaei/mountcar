all:
	g++ -c -I/usr/include/opencv hdw.cpp -o hdw.o
	g++ -c -std=c++11 udpClient.cpp -o udpClient.o
	g++ -std=c++11 -lopencv_core -lopencv_highgui -lopencv_imgproc -lwiringPi -ldl mountcar.cpp hdw.o udpClient.o -o mountcar
	gcc -Wall -fPIC rlalg.c -shared -Wl,-soname,librlalg.so -o librlalg.so 


clean:
	rm mountcar
	rm *.so *.o
