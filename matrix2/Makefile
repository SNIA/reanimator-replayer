#
# Static linking with sqlite (using sqlite.o) and dynamic with DataSeries.
#

SQLITE_INCLUDE_DIR = /root/sqlite-autoconf-3071300/
SQLITE_LIB_DIR = /root/sqlite-autoconf-3071300/
SQLITE_OBJECT_FILE = $(SQLITE_LIB_DIR)/sqlite3.o

DS_INCLUDE_DIR = /root/build/opt-centos-6.2-x86_64/include/
DS_LIB_DIR = /root/build/opt-centos-6.2-x86_64/lib/

LIBXML_INCLUDE_DIR = /usr/include/libxml2/

INCLUDES = -I $(SQLITE_INCLUDE_DIR) -I $(DS_INCLUDE_DIR) -I $(LIBXML_INCLUDE_DIR)
LIBS = -L $(DS_LIB_DIR)


CC = gcc
CFLAGS = -Wall -ldl -lrt

CXX = g++
CXXFLAGS = -Wall -ldl -lrt

TARGETS = fun_with_sqlite load_client iterate_client dsMultiExtentTypeRead dsLoadMatrix

.PHONEY: all
all: $(TARGETS)

fun_with_sqlite: fun_with_sqlite.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o fun_with_sqlite.o fun_with_sqlite.c
	$(CC) $(CFLAGS) fun_with_sqlite.o $(SQLITE_OBJECT_FILE) -o fun_with_sqlite

feature_matrix.o: feature_matrix.h feature_matrix_utils.h feature_matrix.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o feature_matrix.o feature_matrix.c

feature_matrix_utils.o: feature_matrix.h feature_matrix_utils.h \
		feature_matrix_utils.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o feature_matrix_utils.o \
		feature_matrix_utils.c

feature_matrix_iterator.o: feature_matrix.h feature_matrix_utils.h \
		feature_matrix_iterator.h feature_matrix_iterator.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o feature_matrix_iterator.o \
		feature_matrix_iterator.c

load_client: feature_matrix.o feature_matrix_utils.o \
		feature_matrix_iterator.o load_client.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o load_client.o load_client.c
	$(CC) $(CFLAGS) load_client.o feature_matrix.o feature_matrix_utils.o \
		feature_matrix_iterator.o $(SQLITE_OBJECT_FILE) -o load_client

iterate_client: feature_matrix.h feature_matrix.o feature_matrix_iterator.o \
			feature_matrix_utils.o iterate_client.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o iterate_client.o iterate_client.c
	$(CC) $(CFLAGS) iterate_client.o feature_matrix.o feature_matrix_utils.o \
		feature_matrix_iterator.o $(SQLITE_OBJECT_FILE) -o iterate_client

dsMultiExtentTypeRead: dsMultiExtentTypeRead.o
	$(CXX) $(CXXFLAGS) $(LIBS) -lDataSeries dsMultiExtentTypeRead.o -o dsMultiExtentTypeRead

dsMultiExtentTypeRead.o: dsMultiExtentTypeRead.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) dsMultiExtentTypeRead.cpp -c -o dsMultiExtentTypeRead.o

dsLoadMatrix: dsLoadMatrix.o feature_matrix.o feature_matrix_utils.o feature_matrix_iterator.o
	$(CXX) $(CXXFLAGS) $(LIBS) -lDataSeries dsLoadMatrix.o feature_matrix.o feature_matrix_utils.o feature_matrix_iterator.o $(SQLITE_OBJECT_FILE) -o dsLoadMatrix
	
dsLoadMatrix.o: dsLoadMatrix.cpp
	 $(CXX) $(CXXFLAGS) $(INCLUDES) -c dsLoadMatrix.cpp -o dsLoadMatrix.o

.PHONEY: clean
clean:
	rm -f $(TARGETS) *.o
