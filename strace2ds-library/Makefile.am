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
	xml/clone.xml		\
	xml/close.xml		\
	xml/creat.xml		\
	xml/dup.xml		\
	xml/dup2.xml		\
	xml/execve.xml		\
	xml/exit.xml		\
	xml/faccessat.xml		\
	xml/fchmod.xml		\
	xml/fchmodat.xml		\
	xml/fcntl.xml		\
	xml/fork.xml		\
	xml/fgetxattr.xml	\
	xml/flistxattr.xml	\
	xml/fremovexattr.xml	\
	xml/fsetxattr.xml	\
	xml/fstat.xml		\
	xml/fstatat.xml		\
	xml/fstatfs.xml		\
	xml/fsync.xml		\
	xml/getdents.xml	\
	xml/getxattr.xml	\
	xml/ioctl.xml		\
	xml/link.xml		\
	xml/linkat.xml		\
	xml/listxattr.xml	\
	xml/lgetxattr.xml	\
	xml/llistxattr.xml	\
	xml/lremovexattr.xml	\
	xml/lseek.xml		\
	xml/lsetxattr.xml	\
	xml/lstat.xml		\
	xml/mkdir.xml		\
	xml/mkdirat.xml		\
	xml/mknod.xml		\
	xml/mknodat.xml		\
	xml/mmap.xml		\
	xml/munmap.xml		\
	xml/open.xml		\
	xml/openat.xml		\
	xml/pipe.xml		\
	xml/pread.xml		\
	xml/pwrite.xml		\
	xml/read.xml		\
	xml/readlink.xml	\
	xml/readv.xml		\
	xml/removexattr.xml	\
	xml/rename.xml		\
	xml/rmdir.xml		\
	xml/setxattr.xml	\
	xml/stat.xml		\
	xml/statfs.xml		\
	xml/symlink.xml		\
	xml/truncate.xml	\
	xml/unlink.xml		\
	xml/unlinkat.xml	\
	xml/utime.xml		\
	xml/utimensat.xml	\
	xml/utimes.xml		\
	xml/vfork.xml		\
	xml/write.xml		\
	xml/writev.xml          \
	xml/umask.xml		\
	xml/setsid.xml		\
	xml/setpgid.xml		\
	xml/getrlimit.xml		\
	xml/setrlimit.xml		

EXTRA_DIST=$(SNIA_SYSCALL_FIELDS_TABLE_FILE) $(XML_FILES) buildall
tablesdir = $(prefix)/tables
tables_DATA = $(SNIA_SYSCALL_FIELDS_TABLE_FILE)
xmldir = $(prefix)/xml
xml_DATA = $(XML_FILES)
