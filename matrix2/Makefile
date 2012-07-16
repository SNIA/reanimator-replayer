# if set, compile with BDB instead of SQLite
WITHBDB = 0

# set to SQLite include and library directories
SQLITE_INCLUDE_DIR = /root/sqlite-autoconf-3071300/
SQLITE_LIB_DIR = /root/sqlite-autoconf-3071300/

# set to BDB include and library directories
BDB_INCLUDE_DIR = /root/db-5.3.21/build_unix/
BDB_LIB_DIR = /root/db-5.3.21/build_unix/

ifeq ($(WITHBDB), 1)
	LIBS = -L $(BDB_LIB_DIR) -lsqlite3
	INCLUDES = -I $(BDB_INCLUDE_DIR) -I $(SQLITE_INCLUDE_DIR)
else
	LIBS = -L $(SQLITE_LIB_DIR) -lsqlite3
	INCLUDES = -I $(SQLITE_INCLUDE_DIR)
endif

DS_INCLUDE_DIR = /root/build/opt-centos-6.2-x86_64/include/
DS_LIB_DIR = /root/build/opt-centos-6.2-x86_64/lib/
LIBXML_INCLUDE_DIR = /usr/include/libxml2/

CC = gcc
CFLAGS = -g -Wall 

CXX = g++
CXXFLAGS = -g -Wall

TARGETS = fun_with_sqlite load_client iterate_client dsMultiExtentTypeRead

.PHONEY: all
all: $(TARGETS)

fun_with_sqlite: fun_with_sqlite.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o fun_with_sqlite.o fun_with_sqlite.c
	$(CC) $(CFLAGS) $(LIBS) fun_with_sqlite.o -o fun_with_sqlite

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
		feature_matrix_iterator.o $(LIBS) -o load_client

iterate_client: feature_matrix.h feature_matrix.o feature_matrix_iterator.o \
			feature_matrix_utils.o iterate_client.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o iterate_client.o iterate_client.c
	$(CC) $(CFLAGS) iterate_client.o feature_matrix.o feature_matrix_utils.o \
		feature_matrix_iterator.o $(LIBS) -o iterate_client

dsMultiExtentTypeRead: dsMultiExtentTypeRead.o
	$(CXX) -L $(DS_LIB_DIR) -lDataSeries dsMultiExtentTypeRead.o -o dsMultiExtentTypeRead

dsMultiExtentTypeRead.o: dsMultiExtentTypeRead.cpp
	$(CXX) -I $(DS_INCLUDE_DIR) -I $(LIBXML_INCLUDE_DIR) dsMultiExtentTypeRead.cpp -c  -o dsMultiExtentTypeRead.o

dsLoadMatrix: dsLoadMatrix.o
	$(CXX) -L $(DS_LIB_DIR) $(LIBS) -lDataSeries dsLoadMatrix.o feature_matrix.o feature_matrix_utils.o feature_matrix_iterator.o -o dsLoadMatrix
	
dsLoadMatrix.o: dsLoadMatrix.cpp
	 $(CXX) -I $(DS_INCLUDE_DIR) -I $(LIBXML_INCLUDE_DIR) $(INCLUDES) -c dsLoadMatrix.cpp -o dsLoadMatrix.o

.PHONEY: clean
clean:
	rm -f $(TARGETS) *.o
