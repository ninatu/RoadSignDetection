all : ./src/detector.cpp
	g++ ./src/detector.cpp -o ./bin/detector -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lzmq -std=c++11 -I ./
parallel : ./src/detector_parallel.cpp
	g++ ./src/detector_parallel.cpp -o ./bin/detector_parallel -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lzmq -pthread -std=c++11 -I ./
gdb:
	g++ ./src/detector_gdb.cpp -o ./bin/detector_gdb -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lzmq -std=c++11 -I ./
