ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libstrace2ds.la
libstrace2ds_la_SOURCES=DataSeriesOutputModule.cpp strace2ds.cpp
include_HEADERS = strace2ds.h
noinst_HEADERS = DataSeriesOutputModule.hpp

SNIA_SYSCALL_FIELDS_TABLE_FILE=tables/snia_syscall_fields.table
XML_FILES=			\
	xml/access.xml		\
	xml/chdir.xml		\
	xml/chmod.xml		\
	xml/chown.xml		\
	xml/close.xml		\
	xml/creat.xml		\
	xml/dup.xml		\
	xml/dup2.xml		\
	xml/execve.xml		\
	xml/exit.xml		\
	xml/fcntl.xml		\
	xml/fork.xml		\
	xml/fstat.xml		\
	xml/fsync.xml		\
	xml/getdents.xml	\
	xml/ioctl.xml		\
	xml/link.xml		\
	xml/lseek.xml		\
	xml/lstat.xml		\
	xml/mkdir.xml		\
	xml/mknod.xml		\
	xml/mmap.xml		\
	xml/munmap.xml		\
	xml/open.xml		\
	xml/pipe.xml		\
	xml/pread.xml		\
	xml/pwrite.xml		\
	xml/read.xml		\
	xml/readlink.xml	\
	xml/readv.xml		\
	xml/rename.xml		\
	xml/rmdir.xml		\
	xml/stat.xml		\
	xml/symlink.xml		\
	xml/truncate.xml	\
	xml/unlink.xml		\
	xml/unlinkat.xml	\
	xml/utime.xml		\
	xml/utimes.xml		\
	xml/write.xml		\
	xml/writev.xml

EXTRA_DIST=$(SNIA_SYSCALL_FIELDS_TABLE_FILE) $(XML_FILES) buildall
tablesdir = $(prefix)/tables
tables_DATA = $(SNIA_SYSCALL_FIELDS_TABLE_FILE)
xmldir = $(prefix)/xml
xml_DATA = $(XML_FILES)
