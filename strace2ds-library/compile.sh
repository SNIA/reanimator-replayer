g++ -g -O0 -std=c++0x -lLintel -lDataSeries -I/home/ubuntu/build/include -I/usr/include/libxml2/ -L/home/ubuntu/build/lib   -c -o DataSeriesOutputModule.o DataSeriesOutputModule.cpp
g++ -g -O0 -std=c++0x -lLintel -lDataSeries -I/home/ubuntu/build/include -I/usr/include/libxml2/ -L/home/ubuntu/build/lib   -c -o strace2ds.o strace2ds.cpp
cp strace2ds.o DataSeriesOutputModule.o ~/fsl-strace
