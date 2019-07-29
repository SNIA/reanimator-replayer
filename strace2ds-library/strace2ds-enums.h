/*
 * This file was auto generated on Sun Jul 28 00:35:40 2019.
 * DO NOT EDIT BY HAND.
 */
#ifdef USE_ENUMS
enum syscall_names {
	SYSCALL_NAME_COMMON = 0,
	SYSCALL_NAME_READ = 1,
	SYSCALL_NAME_WRITE = 2,
	SYSCALL_NAME_LSEEK = 3,
	SYSCALL_NAME_MMAP = 4,
	SYSCALL_NAME_FSTAT = 5,
	SYSCALL_NAME_FSTATAT = 6,
	SYSCALL_NAME_PREAD = 7,
	SYSCALL_NAME_PWRITE = 8,
	SYSCALL_NAME_READV = 9,
	SYSCALL_NAME_WRITEV = 10,
	SYSCALL_NAME_FCNTL = 11,
	SYSCALL_NAME_IOCTL = 12,
	SYSCALL_NAME_FSYNC = 13,
	SYSCALL_NAME_FDATASYNC = 14,
	SYSCALL_NAME_FALLOCATE = 15,
	SYSCALL_NAME_READAHEAD = 16,
	SYSCALL_NAME_OPEN = 17,
	SYSCALL_NAME_CLOSE = 18,
	SYSCALL_NAME_STAT = 19,
	SYSCALL_NAME_LSTAT = 20,
	SYSCALL_NAME_ACCESS = 21,
	SYSCALL_NAME_FACCESSAT = 22,
	SYSCALL_NAME_EXECVE = 23,
	SYSCALL_NAME_TRUNCATE = 24,
	SYSCALL_NAME_FTRUNCATE = 25,
	SYSCALL_NAME_CHDIR = 26,
	SYSCALL_NAME_CHROOT = 27,
	SYSCALL_NAME_MKDIR = 28,
	SYSCALL_NAME_MKDIRAT = 29,
	SYSCALL_NAME_RMDIR = 30,
	SYSCALL_NAME_CREAT = 31,
	SYSCALL_NAME_LINK = 32,
	SYSCALL_NAME_UNLINK = 33,
	SYSCALL_NAME_SYMLINK = 34,
	SYSCALL_NAME_SYMLINKAT = 35,
	SYSCALL_NAME_READLINK = 36,
	SYSCALL_NAME_CHMOD = 37,
	SYSCALL_NAME_CHOWN = 38,
	SYSCALL_NAME_UTIME = 39,
	SYSCALL_NAME_UTIMES = 40,
	SYSCALL_NAME_MKNOD = 41,
	SYSCALL_NAME_MKNODAT = 42,
	SYSCALL_NAME_MUNMAP = 43,
	SYSCALL_NAME_PIPE = 44,
	SYSCALL_NAME_EXIT = 45,
	SYSCALL_NAME_FORK = 46,
	SYSCALL_NAME_DUP = 47,
	SYSCALL_NAME_DUP2 = 48,
	SYSCALL_NAME_DUP3 = 49,
	SYSCALL_NAME_RENAME = 50,
	SYSCALL_NAME_UNLINKAT = 51,
	SYSCALL_NAME_GETDENTS = 52,
	SYSCALL_NAME_OPENAT = 53,
	SYSCALL_NAME_LINKAT = 54,
	SYSCALL_NAME_FCHMOD = 55,
	SYSCALL_NAME_FCHMODAT = 56,
	SYSCALL_NAME_FCHDIR = 57,
	SYSCALL_NAME_UTIMENSAT = 58,
	SYSCALL_NAME_STATFS = 59,
	SYSCALL_NAME_FSTATFS = 60,
	SYSCALL_NAME_CLONE = 61,
	SYSCALL_NAME_VFORK = 62,
	SYSCALL_NAME_UMASK = 63,
	SYSCALL_NAME_FLOCK = 64,
	SYSCALL_NAME_SETXATTR = 65,
	SYSCALL_NAME_LSETXATTR = 66,
	SYSCALL_NAME_GETXATTR = 67,
	SYSCALL_NAME_LGETXATTR = 68,
	SYSCALL_NAME_FSETXATTR = 69,
	SYSCALL_NAME_FGETXATTR = 70,
	SYSCALL_NAME_LISTXATTR = 71,
	SYSCALL_NAME_LLISTXATTR = 72,
	SYSCALL_NAME_FLISTXATTR = 73,
	SYSCALL_NAME_REMOVEXATTR = 74,
	SYSCALL_NAME_LREMOVEXATTR = 75,
	SYSCALL_NAME_FREMOVEXATTR = 76,
	SYSCALL_NAME_SETSID = 77,
	SYSCALL_NAME_SETPGID = 78,
	SYSCALL_NAME_GETRLIMIT = 79,
	SYSCALL_NAME_SETRLIMIT = 80,
	SYSCALL_NAME_SOCKET = 81,
	SYSCALL_NAME_EPOLL_CREATE = 82,
	SYSCALL_NAME_EPOLL_CREATE1 = 83,
	SYSCALL_NAME_CONNECT = 84,
	SYSCALL_NAME_BIND = 85,
	SYSCALL_NAME_ACCEPT = 86,
	SYSCALL_NAME_ACCEPT4 = 87,
	SYSCALL_NAME_GETSOCKNAME = 88,
	SYSCALL_NAME_GETPEERNAME = 89,
	SYSCALL_NAME_LISTEN = 90,
	SYSCALL_NAME_SHUTDOWN = 91,
	SYSCALL_NAME_GETSOCKOPT = 92,
	SYSCALL_NAME_SETSOCKOPT = 93,
	SYSCALL_NAME_SOCKETPAIR = 94,
	SYSCALL_NAME_RECV = 95,
	SYSCALL_NAME_RECVFROM = 96,
	SYSCALL_NAME_RECVMSG = 97,
	SYSCALL_NAME_SEND = 98,
	SYSCALL_NAME_SENDTO = 99,
	SYSCALL_NAME_SENDMSG = 100,
	SYSCALL_NAME_MMAPPREAD = 101,
	MAX_SYSCALL_NAMES = 102
};

enum syscall_fields {
	SYSCALL_FIELD_ACCESS_TIME = 0,
	SYSCALL_FIELD_ALLOCATE_LEN = 1,
	SYSCALL_FIELD_ALLOCATE_OFFSET = 2,
	SYSCALL_FIELD_ARGUMENT = 3,
	SYSCALL_FIELD_ARGUMENT_DESCRIPTOR_FLAG_EXEC_CLOSE = 4,
	SYSCALL_FIELD_ARGUMENT_LEASE_READ = 5,
	SYSCALL_FIELD_ARGUMENT_LEASE_REMOVE = 6,
	SYSCALL_FIELD_ARGUMENT_LEASE_WRITE = 7,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_ACCESS = 8,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_ATTRIBUTE = 9,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_CREATE = 10,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_DELETE = 11,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_MODIFY = 12,
	SYSCALL_FIELD_ARGUMENT_NOTIFY_RENAME = 13,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_APPEND = 14,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_ASYNC = 15,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_CREATE = 16,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECT = 17,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECTORY = 18,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_EXCLUSIVE = 19,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_LARGEFILE = 20,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_ACCESS_TIME = 21,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_BLOCKING_MODE = 22,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_CONTROLLING_TERMINAL = 23,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_DELAY = 24,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_FOLLOW = 25,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_AND_WRITE = 26,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_ONLY = 27,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_SYNCHRONOUS = 28,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_TRUNCATE = 29,
	SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_WRITE_ONLY = 30,
	SYSCALL_FIELD_ARGUMENT_VALUE = 31,
	SYSCALL_FIELD_BACKLOG = 32,
	SYSCALL_FIELD_BUFFER_NOT_CAPTURED = 33,
	SYSCALL_FIELD_BUFFER_SIZE = 34,
	SYSCALL_FIELD_BYTES_REQUESTED = 35,
	SYSCALL_FIELD_CHILD_STACK_ADDRESS = 36,
	SYSCALL_FIELD_CHILD_THREAD_ID = 37,
	SYSCALL_FIELD_COMMAND_DUP = 38,
	SYSCALL_FIELD_COMMAND_GET_DESCRIPTOR_FLAGS = 39,
	SYSCALL_FIELD_COMMAND_GET_LEASE = 40,
	SYSCALL_FIELD_COMMAND_GET_LOCK = 41,
	SYSCALL_FIELD_COMMAND_GET_PROCESS_ID = 42,
	SYSCALL_FIELD_COMMAND_GET_SIGNAL = 43,
	SYSCALL_FIELD_COMMAND_GET_STATUS_FLAGS = 44,
	SYSCALL_FIELD_COMMAND_NOTIFY = 45,
	SYSCALL_FIELD_COMMAND_SET_DESCRIPTOR_FLAGS = 46,
	SYSCALL_FIELD_COMMAND_SET_LEASE = 47,
	SYSCALL_FIELD_COMMAND_SET_LOCK = 48,
	SYSCALL_FIELD_COMMAND_SET_LOCK_WAIT = 49,
	SYSCALL_FIELD_COMMAND_SET_PROCESS_ID = 50,
	SYSCALL_FIELD_COMMAND_SET_SIGNAL = 51,
	SYSCALL_FIELD_COMMAND_SET_STATUS_FLAGS = 52,
	SYSCALL_FIELD_COMMAND_VALUE = 53,
	SYSCALL_FIELD_CONTINUATION_NUMBER = 54,
	SYSCALL_FIELD_COUNT = 55,
	SYSCALL_FIELD_DATA_READ = 56,
	SYSCALL_FIELD_DATA_WRITTEN = 57,
	SYSCALL_FIELD_DESCRIPTOR = 58,
	SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY = 59,
	SYSCALL_FIELD_DEV = 60,
	SYSCALL_FIELD_DIRENT_BUFFER = 61,
	SYSCALL_FIELD_DOMAIN = 62,
	SYSCALL_FIELD_ENVIRONMENT = 63,
	SYSCALL_FIELD_EPOLL_FLAGS = 64,
	SYSCALL_FIELD_EPOLL_SIZE = 65,
	SYSCALL_FIELD_ERRNO_NUMBER = 66,
	SYSCALL_FIELD_ERRNO_STRING = 67,
	SYSCALL_FIELD_EXECUTING_PGID = 68,
	SYSCALL_FIELD_EXECUTING_PID = 69,
	SYSCALL_FIELD_EXECUTING_PPID = 70,
	SYSCALL_FIELD_EXECUTING_SID = 71,
	SYSCALL_FIELD_EXECUTING_TID = 72,
	SYSCALL_FIELD_EXECUTING_UID = 73,
	SYSCALL_FIELD_EXIT_STATUS = 74,
	SYSCALL_FIELD_FLAG_32BIT = 75,
	SYSCALL_FIELD_FLAG_ANONYMOUS = 76,
	SYSCALL_FIELD_FLAG_APPEND = 77,
	SYSCALL_FIELD_FLAG_ASYNC = 78,
	SYSCALL_FIELD_FLAG_AT_SYMLINK_NOFOLLOW = 79,
	SYSCALL_FIELD_FLAG_CHILD_CLEARTID = 80,
	SYSCALL_FIELD_FLAG_CHILD_SETTID = 81,
	SYSCALL_FIELD_FLAG_CLOSE_ON_EXEC = 82,
	SYSCALL_FIELD_FLAG_CMSG_CLOEXEC = 83,
	SYSCALL_FIELD_FLAG_CONFIRM = 84,
	SYSCALL_FIELD_FLAG_CREATE = 85,
	SYSCALL_FIELD_FLAG_DENYWRITE = 86,
	SYSCALL_FIELD_FLAG_DIRECT = 87,
	SYSCALL_FIELD_FLAG_DIRECTORY = 88,
	SYSCALL_FIELD_FLAG_DONTROUTE = 89,
	SYSCALL_FIELD_FLAG_DONTWAIT = 90,
	SYSCALL_FIELD_FLAG_DONWAIT = 91,
	SYSCALL_FIELD_FLAG_EMPTY_PATH = 92,
	SYSCALL_FIELD_FLAG_EOR = 93,
	SYSCALL_FIELD_FLAG_ERRQUEUE = 94,
	SYSCALL_FIELD_FLAG_EXCLUSIVE = 95,
	SYSCALL_FIELD_FLAG_EXECUTABLE = 96,
	SYSCALL_FIELD_FLAG_FILE = 97,
	SYSCALL_FIELD_FLAG_FILES = 98,
	SYSCALL_FIELD_FLAG_FILESYSTEM = 99,
	SYSCALL_FIELD_FLAG_FIXED = 100,
	SYSCALL_FIELD_FLAG_GROWS_DOWN = 101,
	SYSCALL_FIELD_FLAG_HUGE_TLB = 102,
	SYSCALL_FIELD_FLAG_IO = 103,
	SYSCALL_FIELD_FLAG_LARGEFILE = 104,
	SYSCALL_FIELD_FLAG_LOCKED = 105,
	SYSCALL_FIELD_FLAG_MORE = 106,
	SYSCALL_FIELD_FLAG_NEWIPC = 107,
	SYSCALL_FIELD_FLAG_NEWNET = 108,
	SYSCALL_FIELD_FLAG_NEWNS = 109,
	SYSCALL_FIELD_FLAG_NEWPID = 110,
	SYSCALL_FIELD_FLAG_NEWUSER = 111,
	SYSCALL_FIELD_FLAG_NEWUTS = 112,
	SYSCALL_FIELD_FLAG_NO_ACCESS_TIME = 113,
	SYSCALL_FIELD_FLAG_NO_BLOCKING_MODE = 114,
	SYSCALL_FIELD_FLAG_NO_CONTROLLING_TERMINAL = 115,
	SYSCALL_FIELD_FLAG_NO_DELAY = 116,
	SYSCALL_FIELD_FLAG_NO_FOLLOW = 117,
	SYSCALL_FIELD_FLAG_NO_RESERVE = 118,
	SYSCALL_FIELD_FLAG_NON_BLOCK = 119,
	SYSCALL_FIELD_FLAG_NOSIGNAL = 120,
	SYSCALL_FIELD_FLAG_OOB = 121,
	SYSCALL_FIELD_FLAG_PARENT = 122,
	SYSCALL_FIELD_FLAG_PARENT_SETTID = 123,
	SYSCALL_FIELD_FLAG_PEEK = 124,
	SYSCALL_FIELD_FLAG_POPULATE = 125,
	SYSCALL_FIELD_FLAG_PRIVATE = 126,
	SYSCALL_FIELD_FLAG_PTRACE = 127,
	SYSCALL_FIELD_FLAG_READ_AND_WRITE = 128,
	SYSCALL_FIELD_FLAG_READ_ONLY = 129,
	SYSCALL_FIELD_FLAG_REMOVE_DIRECTORY = 130,
	SYSCALL_FIELD_FLAG_SETTLS = 131,
	SYSCALL_FIELD_FLAG_SHARED = 132,
	SYSCALL_FIELD_FLAG_SIGHAND = 133,
	SYSCALL_FIELD_FLAG_STACK = 134,
	SYSCALL_FIELD_FLAG_SYMLINK_FOLLOW = 135,
	SYSCALL_FIELD_FLAG_SYMLINK_NOFOLLOW = 136,
	SYSCALL_FIELD_FLAG_SYNCHRONOUS = 137,
	SYSCALL_FIELD_FLAG_SYSVSEM = 138,
	SYSCALL_FIELD_FLAG_THREAD = 139,
	SYSCALL_FIELD_FLAG_TRUNC = 140,
	SYSCALL_FIELD_FLAG_TRUNCATE = 141,
	SYSCALL_FIELD_FLAG_UNTRACED = 142,
	SYSCALL_FIELD_FLAG_VALUE = 143,
	SYSCALL_FIELD_FLAG_VFORK = 144,
	SYSCALL_FIELD_FLAG_VM = 145,
	SYSCALL_FIELD_FLAG_WAITALL = 146,
	SYSCALL_FIELD_FLAG_WRITE_ONLY = 147,
	SYSCALL_FIELD_FLAG_XATTR_CREATE = 148,
	SYSCALL_FIELD_FLAG_XATTR_REPLACE = 149,
	SYSCALL_FIELD_FLAGS = 150,
	SYSCALL_FIELD_FLAGS_AT_EACCESS = 151,
	SYSCALL_FIELD_FLAGS_AT_EMPTY_PATH = 152,
	SYSCALL_FIELD_FLAGS_AT_NO_AUTOMOUNT = 153,
	SYSCALL_FIELD_FLAGS_AT_SYMLINK_NOFOLLOW = 154,
	SYSCALL_FIELD_FLAGS_VALUE = 155,
	SYSCALL_FIELD_FULL_NEWNAME = 156,
	SYSCALL_FIELD_FULL_NEWPATHNAME = 157,
	SYSCALL_FIELD_FULL_OLDNAME = 158,
	SYSCALL_FIELD_FULL_OLDPATHNAME = 159,
	SYSCALL_FIELD_FULL_PATHNAME = 160,
	SYSCALL_FIELD_GENERATED = 161,
	SYSCALL_FIELD_GIVEN_NEWNAME = 162,
	SYSCALL_FIELD_GIVEN_NEWPATHNAME = 163,
	SYSCALL_FIELD_GIVEN_OLDNAME = 164,
	SYSCALL_FIELD_GIVEN_OLDPATHNAME = 165,
	SYSCALL_FIELD_GIVEN_PATHNAME = 166,
	SYSCALL_FIELD_HOW = 167,
	SYSCALL_FIELD_IOCTL_BUFFER = 168,
	SYSCALL_FIELD_IOV_DATA_READ = 169,
	SYSCALL_FIELD_IOV_DATA_WRITTEN = 170,
	SYSCALL_FIELD_IOV_NUMBER = 171,
	SYSCALL_FIELD_LENGTH = 172,
	SYSCALL_FIELD_LEVEL = 173,
	SYSCALL_FIELD_LINK_VALUE = 174,
	SYSCALL_FIELD_LIST_SIZE = 175,
	SYSCALL_FIELD_LOCK_LENGTH = 176,
	SYSCALL_FIELD_LOCK_PID = 177,
	SYSCALL_FIELD_LOCK_START = 178,
	SYSCALL_FIELD_LOCK_TYPE = 179,
	SYSCALL_FIELD_LOCK_TYPE_READ = 180,
	SYSCALL_FIELD_LOCK_TYPE_UNLOCKED = 181,
	SYSCALL_FIELD_LOCK_TYPE_WRITE = 182,
	SYSCALL_FIELD_LOCK_WHENCE = 183,
	SYSCALL_FIELD_LOCK_WHENCE_CURRENT = 184,
	SYSCALL_FIELD_LOCK_WHENCE_END = 185,
	SYSCALL_FIELD_LOCK_WHENCE_START = 186,
	SYSCALL_FIELD_MMAP_BYTES_REQUESTED = 187,
	SYSCALL_FIELD_MMAP_DATA_READ = 188,
	SYSCALL_FIELD_MMAP_DESCRIPTOR = 189,
	SYSCALL_FIELD_MMAP_OFFSET = 190,
	SYSCALL_FIELD_MOD_TIME = 191,
	SYSCALL_FIELD_MODE_R_GROUP = 192,
	SYSCALL_FIELD_MODE_R_OTHERS = 193,
	SYSCALL_FIELD_MODE_R_USER = 194,
	SYSCALL_FIELD_MODE_W_GROUP = 195,
	SYSCALL_FIELD_MODE_W_OTHERS = 196,
	SYSCALL_FIELD_MODE_W_USER = 197,
	SYSCALL_FIELD_MODE_X_GROUP = 198,
	SYSCALL_FIELD_MODE_X_OTHERS = 199,
	SYSCALL_FIELD_MODE_X_USER = 200,
	SYSCALL_FIELD_MODE_EXECUTE = 201,
	SYSCALL_FIELD_MODE_EXIST = 202,
	SYSCALL_FIELD_MODE_GID = 203,
	SYSCALL_FIELD_MODE_READ = 204,
	SYSCALL_FIELD_MODE_STICKY_BIT = 205,
	SYSCALL_FIELD_MODE_UID = 206,
	SYSCALL_FIELD_MODE_VALUE = 207,
	SYSCALL_FIELD_MODE_WRITE = 208,
	SYSCALL_FIELD_NEW_DESCRIPTOR = 209,
	SYSCALL_FIELD_NEW_DESCRIPTOR_CURRENT_WORKING_DIRECTORY = 210,
	SYSCALL_FIELD_NEW_GROUP = 211,
	SYSCALL_FIELD_NEW_OWNER = 212,
	SYSCALL_FIELD_NEW_TLS = 213,
	SYSCALL_FIELD_OFFSET = 214,
	SYSCALL_FIELD_OLD_DESCRIPTOR = 215,
	SYSCALL_FIELD_OLD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY = 216,
	SYSCALL_FIELD_OPEN_VALUE = 217,
	SYSCALL_FIELD_OPERATION = 218,
	SYSCALL_FIELD_OPERATION_VALUE = 219,
	SYSCALL_FIELD_OPTION_VALUE = 220,
	SYSCALL_FIELD_OPTNAME = 221,
	SYSCALL_FIELD_PARAMETER = 222,
	SYSCALL_FIELD_PARENT_THREAD_ID = 223,
	SYSCALL_FIELD_PGID = 224,
	SYSCALL_FIELD_PID = 225,
	SYSCALL_FIELD_PROTECTION_EXEC = 226,
	SYSCALL_FIELD_PROTECTION_NONE = 227,
	SYSCALL_FIELD_PROTECTION_READ = 228,
	SYSCALL_FIELD_PROTECTION_VALUE = 229,
	SYSCALL_FIELD_PROTECTION_WRITE = 230,
	SYSCALL_FIELD_PROTOCOL = 231,
	SYSCALL_FIELD_READ_DESCRIPTOR = 232,
	SYSCALL_FIELD_READAHEAD_OFF = 233,
	SYSCALL_FIELD_READAHEAD_SIZE = 234,
	SYSCALL_FIELD_REQUEST = 235,
	SYSCALL_FIELD_RESOURCE = 236,
	SYSCALL_FIELD_RESOURCE_HARD_LIMIT = 237,
	SYSCALL_FIELD_RESOURCE_SOFT_LIMIT = 238,
	SYSCALL_FIELD_RESOURCE_VALUE = 239,
	SYSCALL_FIELD_RETURN_VALUE = 240,
	SYSCALL_FIELD_SIGNAL_ABORT = 241,
	SYSCALL_FIELD_SIGNAL_ALARM = 242,
	SYSCALL_FIELD_SIGNAL_BUS = 243,
	SYSCALL_FIELD_SIGNAL_CHILD = 244,
	SYSCALL_FIELD_SIGNAL_CONTINUE = 245,
	SYSCALL_FIELD_SIGNAL_CPU_EXCEEDED = 246,
	SYSCALL_FIELD_SIGNAL_FILE_SIZE_EXCEEDED = 247,
	SYSCALL_FIELD_SIGNAL_FLOATING_POINT_EXCEPTION = 248,
	SYSCALL_FIELD_SIGNAL_HANGUP = 249,
	SYSCALL_FIELD_SIGNAL_ILLEGAL = 250,
	SYSCALL_FIELD_SIGNAL_IO = 251,
	SYSCALL_FIELD_SIGNAL_IOT_TRAP = 252,
	SYSCALL_FIELD_SIGNAL_KILL = 253,
	SYSCALL_FIELD_SIGNAL_PIPE = 254,
	SYSCALL_FIELD_SIGNAL_POWER = 255,
	SYSCALL_FIELD_SIGNAL_PROF_ALARM = 256,
	SYSCALL_FIELD_SIGNAL_SEGV = 257,
	SYSCALL_FIELD_SIGNAL_STACK_FAULT = 258,
	SYSCALL_FIELD_SIGNAL_STOP = 259,
	SYSCALL_FIELD_SIGNAL_TERMINAL_INTERRUPT = 260,
	SYSCALL_FIELD_SIGNAL_TERMINAL_QUIT = 261,
	SYSCALL_FIELD_SIGNAL_TERMINAL_STOP = 262,
	SYSCALL_FIELD_SIGNAL_TERMINATION = 263,
	SYSCALL_FIELD_SIGNAL_TRACE_TRAP = 264,
	SYSCALL_FIELD_SIGNAL_TTY_READ = 265,
	SYSCALL_FIELD_SIGNAL_TTY_WRITE = 266,
	SYSCALL_FIELD_SIGNAL_URGENT = 267,
	SYSCALL_FIELD_SIGNAL_USER_DEFINED_1 = 268,
	SYSCALL_FIELD_SIGNAL_USER_DEFINED_2 = 269,
	SYSCALL_FIELD_SIGNAL_VIRTUAL_ALARM = 270,
	SYSCALL_FIELD_SIGNAL_WINDOW_SIZE_CHANGE = 271,
	SYSCALL_FIELD_SOCKADDR_BUFFER = 272,
	SYSCALL_FIELD_SOCKADDR_LENGTH = 273,
	SYSCALL_FIELD_START_ADDRESS = 274,
	SYSCALL_FIELD_STAT_RESULT_ATIME = 275,
	SYSCALL_FIELD_STAT_RESULT_BLKSIZE = 276,
	SYSCALL_FIELD_STAT_RESULT_BLOCKS = 277,
	SYSCALL_FIELD_STAT_RESULT_CTIME = 278,
	SYSCALL_FIELD_STAT_RESULT_DEV = 279,
	SYSCALL_FIELD_STAT_RESULT_GID = 280,
	SYSCALL_FIELD_STAT_RESULT_INO = 281,
	SYSCALL_FIELD_STAT_RESULT_MODE = 282,
	SYSCALL_FIELD_STAT_RESULT_MTIME = 283,
	SYSCALL_FIELD_STAT_RESULT_NLINK = 284,
	SYSCALL_FIELD_STAT_RESULT_RAW = 285,
	SYSCALL_FIELD_STAT_RESULT_RDEV = 286,
	SYSCALL_FIELD_STAT_RESULT_SIZE = 287,
	SYSCALL_FIELD_STAT_RESULT_UID = 288,
	SYSCALL_FIELD_STATFS_RESULT_BAVAIL = 289,
	SYSCALL_FIELD_STATFS_RESULT_BFREE = 290,
	SYSCALL_FIELD_STATFS_RESULT_BLOCKS = 291,
	SYSCALL_FIELD_STATFS_RESULT_BSIZE = 292,
	SYSCALL_FIELD_STATFS_RESULT_FFREE = 293,
	SYSCALL_FIELD_STATFS_RESULT_FILES = 294,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS = 295,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_MANDATORY_LOCK = 296,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_ACCESS_TIME = 297,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DEV = 298,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DIRECTORY_ACCESS_TIME = 299,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_EXEC = 300,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_SET_UID = 301,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_READ_ONLY = 302,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_RELATIVE_ACCESS_TIME = 303,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_SYNCHRONOUS = 304,
	SYSCALL_FIELD_STATFS_RESULT_FLAGS_VALID = 305,
	SYSCALL_FIELD_STATFS_RESULT_FRSIZE = 306,
	SYSCALL_FIELD_STATFS_RESULT_FSID = 307,
	SYSCALL_FIELD_STATFS_RESULT_NAMELEN = 308,
	SYSCALL_FIELD_STATFS_RESULT_RAW = 309,
	SYSCALL_FIELD_STATFS_RESULT_TYPE = 310,
	SYSCALL_FIELD_TARGET_PATHNAME = 311,
	SYSCALL_FIELD_TIME_CALLED = 312,
	SYSCALL_FIELD_TIME_RECORDED = 313,
	SYSCALL_FIELD_TIME_RETURNED = 314,
	SYSCALL_FIELD_TRUNCATE_LENGTH = 315,
	SYSCALL_FIELD_TYPE = 316,
	SYSCALL_FIELD_UNIQUE_ID = 317,
	SYSCALL_FIELD_UTIME_NOW = 318,
	SYSCALL_FIELD_UTIME_OMIT = 319,
	SYSCALL_FIELD_VALUE_READ = 320,
	SYSCALL_FIELD_VALUE_SIZE = 321,
	SYSCALL_FIELD_VALUE_WRITTEN = 322,
	SYSCALL_FIELD_WHENCE = 323,
	SYSCALL_FIELD_WHENCE_VALUE = 324,
	SYSCALL_FIELD_WRITE_DESCRIPTOR = 325,
	SYSCALL_FIELD_XATTR_LIST = 326,
	SYSCALL_FIELD_XATTR_NAME = 327,
	MAX_SYSCALL_FIELDS = 328
};
#else /* USE_ENUMS */
#define SYSCALL_NAME_COMMON "Common"
#define SYSCALL_NAME_ACCEPT "accept"
#define SYSCALL_NAME_ACCEPT4 "accept4"
#define SYSCALL_NAME_ACCESS "access"
#define SYSCALL_NAME_BIND "bind"
#define SYSCALL_NAME_CHDIR "chdir"
#define SYSCALL_NAME_CHMOD "chmod"
#define SYSCALL_NAME_CHOWN "chown"
#define SYSCALL_NAME_CHROOT "chroot"
#define SYSCALL_NAME_CLONE "clone"
#define SYSCALL_NAME_CLOSE "close"
#define SYSCALL_NAME_CONNECT "connect"
#define SYSCALL_NAME_CREAT "creat"
#define SYSCALL_NAME_DUP "dup"
#define SYSCALL_NAME_DUP2 "dup2"
#define SYSCALL_NAME_DUP3 "dup3"
#define SYSCALL_NAME_EPOLL_CREATE "epoll_create"
#define SYSCALL_NAME_EPOLL_CREATE1 "epoll_create1"
#define SYSCALL_NAME_EXECVE "execve"
#define SYSCALL_NAME_EXIT "exit"
#define SYSCALL_NAME_FACCESSAT "faccessat"
#define SYSCALL_NAME_FALLOCATE "fallocate"
#define SYSCALL_NAME_FCHDIR "fchdir"
#define SYSCALL_NAME_FCHMOD "fchmod"
#define SYSCALL_NAME_FCHMODAT "fchmodat"
#define SYSCALL_NAME_FCNTL "fcntl"
#define SYSCALL_NAME_FDATASYNC "fdatasync"
#define SYSCALL_NAME_FGETXATTR "fgetxattr"
#define SYSCALL_NAME_FLISTXATTR "flistxattr"
#define SYSCALL_NAME_FLOCK "flock"
#define SYSCALL_NAME_FORK "fork"
#define SYSCALL_NAME_FREMOVEXATTR "fremovexattr"
#define SYSCALL_NAME_FSETXATTR "fsetxattr"
#define SYSCALL_NAME_FSTAT "fstat"
#define SYSCALL_NAME_FSTATAT "fstatat"
#define SYSCALL_NAME_FSTATFS "fstatfs"
#define SYSCALL_NAME_FSYNC "fsync"
#define SYSCALL_NAME_FTRUNCATE "ftruncate"
#define SYSCALL_NAME_GETDENTS "getdents"
#define SYSCALL_NAME_GETPEERNAME "getpeername"
#define SYSCALL_NAME_GETRLIMIT "getrlimit"
#define SYSCALL_NAME_GETSOCKNAME "getsockname"
#define SYSCALL_NAME_GETSOCKOPT "getsockopt"
#define SYSCALL_NAME_GETXATTR "getxattr"
#define SYSCALL_NAME_IOCTL "ioctl"
#define SYSCALL_NAME_LGETXATTR "lgetxattr"
#define SYSCALL_NAME_LINK "link"
#define SYSCALL_NAME_LINKAT "linkat"
#define SYSCALL_NAME_LISTEN "listen"
#define SYSCALL_NAME_LISTXATTR "listxattr"
#define SYSCALL_NAME_LLISTXATTR "llistxattr"
#define SYSCALL_NAME_LREMOVEXATTR "lremovexattr"
#define SYSCALL_NAME_LSEEK "lseek"
#define SYSCALL_NAME_LSETXATTR "lsetxattr"
#define SYSCALL_NAME_LSTAT "lstat"
#define SYSCALL_NAME_MKDIR "mkdir"
#define SYSCALL_NAME_MKDIRAT "mkdirat"
#define SYSCALL_NAME_MKNOD "mknod"
#define SYSCALL_NAME_MKNODAT "mknodat"
#define SYSCALL_NAME_MMAP "mmap"
#define SYSCALL_NAME_MMAPPREAD "mmappread"
#define SYSCALL_NAME_MUNMAP "munmap"
#define SYSCALL_NAME_OPEN "open"
#define SYSCALL_NAME_OPENAT "openat"
#define SYSCALL_NAME_PIPE "pipe"
#define SYSCALL_NAME_PREAD "pread"
#define SYSCALL_NAME_PWRITE "pwrite"
#define SYSCALL_NAME_READ "read"
#define SYSCALL_NAME_READAHEAD "readahead"
#define SYSCALL_NAME_READLINK "readlink"
#define SYSCALL_NAME_READV "readv"
#define SYSCALL_NAME_RECV "recv"
#define SYSCALL_NAME_RECVFROM "recvfrom"
#define SYSCALL_NAME_RECVMSG "recvmsg"
#define SYSCALL_NAME_REMOVEXATTR "removexattr"
#define SYSCALL_NAME_RENAME "rename"
#define SYSCALL_NAME_RMDIR "rmdir"
#define SYSCALL_NAME_SEND "send"
#define SYSCALL_NAME_SENDMSG "sendmsg"
#define SYSCALL_NAME_SENDTO "sendto"
#define SYSCALL_NAME_SETPGID "setpgid"
#define SYSCALL_NAME_SETRLIMIT "setrlimit"
#define SYSCALL_NAME_SETSID "setsid"
#define SYSCALL_NAME_SETSOCKOPT "setsockopt"
#define SYSCALL_NAME_SETXATTR "setxattr"
#define SYSCALL_NAME_SHUTDOWN "shutdown"
#define SYSCALL_NAME_SOCKET "socket"
#define SYSCALL_NAME_SOCKETPAIR "socketpair"
#define SYSCALL_NAME_STAT "stat"
#define SYSCALL_NAME_STATFS "statfs"
#define SYSCALL_NAME_SYMLINK "symlink"
#define SYSCALL_NAME_SYMLINKAT "symlinkat"
#define SYSCALL_NAME_TRUNCATE "truncate"
#define SYSCALL_NAME_UMASK "umask"
#define SYSCALL_NAME_UNLINK "unlink"
#define SYSCALL_NAME_UNLINKAT "unlinkat"
#define SYSCALL_NAME_UTIME "utime"
#define SYSCALL_NAME_UTIMENSAT "utimensat"
#define SYSCALL_NAME_UTIMES "utimes"
#define SYSCALL_NAME_VFORK "vfork"
#define SYSCALL_NAME_WRITE "write"
#define SYSCALL_NAME_WRITEV "writev"
#define SYSCALL_FIELD_ACCESS_TIME "access_time"
#define SYSCALL_FIELD_ALLOCATE_LEN "allocate_len"
#define SYSCALL_FIELD_ALLOCATE_OFFSET "allocate_offset"
#define SYSCALL_FIELD_ARGUMENT "argument"
#define SYSCALL_FIELD_ARGUMENT_DESCRIPTOR_FLAG_EXEC_CLOSE "argument_descriptor_flag_exec_close"
#define SYSCALL_FIELD_ARGUMENT_LEASE_READ "argument_lease_read"
#define SYSCALL_FIELD_ARGUMENT_LEASE_REMOVE "argument_lease_remove"
#define SYSCALL_FIELD_ARGUMENT_LEASE_WRITE "argument_lease_write"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_ACCESS "argument_notify_access"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_ATTRIBUTE "argument_notify_attribute"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_CREATE "argument_notify_create"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_DELETE "argument_notify_delete"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_MODIFY "argument_notify_modify"
#define SYSCALL_FIELD_ARGUMENT_NOTIFY_RENAME "argument_notify_rename"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_APPEND "argument_status_flag_append"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_ASYNC "argument_status_flag_async"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_CREATE "argument_status_flag_create"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECT "argument_status_flag_direct"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_DIRECTORY "argument_status_flag_directory"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_EXCLUSIVE "argument_status_flag_exclusive"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_LARGEFILE "argument_status_flag_largefile"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_ACCESS_TIME "argument_status_flag_no_access_time"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_BLOCKING_MODE "argument_status_flag_no_blocking_mode"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_CONTROLLING_TERMINAL "argument_status_flag_no_controlling_terminal"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_DELAY "argument_status_flag_no_delay"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_NO_FOLLOW "argument_status_flag_no_follow"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_AND_WRITE "argument_status_flag_read_and_write"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_READ_ONLY "argument_status_flag_read_only"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_SYNCHRONOUS "argument_status_flag_synchronous"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_TRUNCATE "argument_status_flag_truncate"
#define SYSCALL_FIELD_ARGUMENT_STATUS_FLAG_WRITE_ONLY "argument_status_flag_write_only"
#define SYSCALL_FIELD_ARGUMENT_VALUE "argument_value"
#define SYSCALL_FIELD_BACKLOG "backlog"
#define SYSCALL_FIELD_BUFFER_NOT_CAPTURED "buffer_not_captured"
#define SYSCALL_FIELD_BUFFER_SIZE "buffer_size"
#define SYSCALL_FIELD_BYTES_REQUESTED "bytes_requested"
#define SYSCALL_FIELD_CHILD_STACK_ADDRESS "child_stack_address"
#define SYSCALL_FIELD_CHILD_THREAD_ID "child_thread_id"
#define SYSCALL_FIELD_COMMAND_DUP "command_dup"
#define SYSCALL_FIELD_COMMAND_GET_DESCRIPTOR_FLAGS "command_get_descriptor_flags"
#define SYSCALL_FIELD_COMMAND_GET_LEASE "command_get_lease"
#define SYSCALL_FIELD_COMMAND_GET_LOCK "command_get_lock"
#define SYSCALL_FIELD_COMMAND_GET_PROCESS_ID "command_get_process_id"
#define SYSCALL_FIELD_COMMAND_GET_SIGNAL "command_get_signal"
#define SYSCALL_FIELD_COMMAND_GET_STATUS_FLAGS "command_get_status_flags"
#define SYSCALL_FIELD_COMMAND_NOTIFY "command_notify"
#define SYSCALL_FIELD_COMMAND_SET_DESCRIPTOR_FLAGS "command_set_descriptor_flags"
#define SYSCALL_FIELD_COMMAND_SET_LEASE "command_set_lease"
#define SYSCALL_FIELD_COMMAND_SET_LOCK "command_set_lock"
#define SYSCALL_FIELD_COMMAND_SET_LOCK_WAIT "command_set_lock_wait"
#define SYSCALL_FIELD_COMMAND_SET_PROCESS_ID "command_set_process_id"
#define SYSCALL_FIELD_COMMAND_SET_SIGNAL "command_set_signal"
#define SYSCALL_FIELD_COMMAND_SET_STATUS_FLAGS "command_set_status_flags"
#define SYSCALL_FIELD_COMMAND_VALUE "command_value"
#define SYSCALL_FIELD_CONTINUATION_NUMBER "continuation_number"
#define SYSCALL_FIELD_COUNT "count"
#define SYSCALL_FIELD_DATA_READ "data_read"
#define SYSCALL_FIELD_DATA_WRITTEN "data_written"
#define SYSCALL_FIELD_DESCRIPTOR "descriptor"
#define SYSCALL_FIELD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY "descriptor_current_working_directory"
#define SYSCALL_FIELD_DEV "dev"
#define SYSCALL_FIELD_DIRENT_BUFFER "dirent_buffer"
#define SYSCALL_FIELD_DOMAIN "domain"
#define SYSCALL_FIELD_ENVIRONMENT "environment"
#define SYSCALL_FIELD_EPOLL_FLAGS "epoll_flags"
#define SYSCALL_FIELD_EPOLL_SIZE "epoll_size"
#define SYSCALL_FIELD_ERRNO_NUMBER "errno_number"
#define SYSCALL_FIELD_ERRNO_STRING "errno_string"
#define SYSCALL_FIELD_EXECUTING_PGID "executing_pgid"
#define SYSCALL_FIELD_EXECUTING_PID "executing_pid"
#define SYSCALL_FIELD_EXECUTING_PPID "executing_ppid"
#define SYSCALL_FIELD_EXECUTING_SID "executing_sid"
#define SYSCALL_FIELD_EXECUTING_TID "executing_tid"
#define SYSCALL_FIELD_EXECUTING_UID "executing_uid"
#define SYSCALL_FIELD_EXIT_STATUS "exit_status"
#define SYSCALL_FIELD_FLAG_32BIT "flag_32bit"
#define SYSCALL_FIELD_FLAG_ANONYMOUS "flag_anonymous"
#define SYSCALL_FIELD_FLAG_APPEND "flag_append"
#define SYSCALL_FIELD_FLAG_ASYNC "flag_async"
#define SYSCALL_FIELD_FLAG_AT_SYMLINK_NOFOLLOW "flag_at_symlink_nofollow"
#define SYSCALL_FIELD_FLAG_CHILD_CLEARTID "flag_child_cleartid"
#define SYSCALL_FIELD_FLAG_CHILD_SETTID "flag_child_settid"
#define SYSCALL_FIELD_FLAG_CLOSE_ON_EXEC "flag_close_on_exec"
#define SYSCALL_FIELD_FLAG_CMSG_CLOEXEC "flag_cmsg_cloexec"
#define SYSCALL_FIELD_FLAG_CONFIRM "flag_confirm"
#define SYSCALL_FIELD_FLAG_CREATE "flag_create"
#define SYSCALL_FIELD_FLAG_DENYWRITE "flag_denywrite"
#define SYSCALL_FIELD_FLAG_DIRECT "flag_direct"
#define SYSCALL_FIELD_FLAG_DIRECTORY "flag_directory"
#define SYSCALL_FIELD_FLAG_DONTROUTE "flag_dontroute"
#define SYSCALL_FIELD_FLAG_DONTWAIT "flag_dontwait"
#define SYSCALL_FIELD_FLAG_DONWAIT "flag_donwait"
#define SYSCALL_FIELD_FLAG_EMPTY_PATH "flag_empty_path"
#define SYSCALL_FIELD_FLAG_EOR "flag_eor"
#define SYSCALL_FIELD_FLAG_ERRQUEUE "flag_errqueue"
#define SYSCALL_FIELD_FLAG_EXCLUSIVE "flag_exclusive"
#define SYSCALL_FIELD_FLAG_EXECUTABLE "flag_executable"
#define SYSCALL_FIELD_FLAG_FILE "flag_file"
#define SYSCALL_FIELD_FLAG_FILES "flag_files"
#define SYSCALL_FIELD_FLAG_FILESYSTEM "flag_filesystem"
#define SYSCALL_FIELD_FLAG_FIXED "flag_fixed"
#define SYSCALL_FIELD_FLAG_GROWS_DOWN "flag_grows_down"
#define SYSCALL_FIELD_FLAG_HUGE_TLB "flag_huge_tlb"
#define SYSCALL_FIELD_FLAG_IO "flag_io"
#define SYSCALL_FIELD_FLAG_LARGEFILE "flag_largefile"
#define SYSCALL_FIELD_FLAG_LOCKED "flag_locked"
#define SYSCALL_FIELD_FLAG_MORE "flag_more"
#define SYSCALL_FIELD_FLAG_NEWIPC "flag_newipc"
#define SYSCALL_FIELD_FLAG_NEWNET "flag_newnet"
#define SYSCALL_FIELD_FLAG_NEWNS "flag_newns"
#define SYSCALL_FIELD_FLAG_NEWPID "flag_newpid"
#define SYSCALL_FIELD_FLAG_NEWUSER "flag_newuser"
#define SYSCALL_FIELD_FLAG_NEWUTS "flag_newuts"
#define SYSCALL_FIELD_FLAG_NO_ACCESS_TIME "flag_no_access_time"
#define SYSCALL_FIELD_FLAG_NO_BLOCKING_MODE "flag_no_blocking_mode"
#define SYSCALL_FIELD_FLAG_NO_CONTROLLING_TERMINAL "flag_no_controlling_terminal"
#define SYSCALL_FIELD_FLAG_NO_DELAY "flag_no_delay"
#define SYSCALL_FIELD_FLAG_NO_FOLLOW "flag_no_follow"
#define SYSCALL_FIELD_FLAG_NO_RESERVE "flag_no_reserve"
#define SYSCALL_FIELD_FLAG_NON_BLOCK "flag_non_block"
#define SYSCALL_FIELD_FLAG_NOSIGNAL "flag_nosignal"
#define SYSCALL_FIELD_FLAG_OOB "flag_oob"
#define SYSCALL_FIELD_FLAG_PARENT "flag_parent"
#define SYSCALL_FIELD_FLAG_PARENT_SETTID "flag_parent_settid"
#define SYSCALL_FIELD_FLAG_PEEK "flag_peek"
#define SYSCALL_FIELD_FLAG_POPULATE "flag_populate"
#define SYSCALL_FIELD_FLAG_PRIVATE "flag_private"
#define SYSCALL_FIELD_FLAG_PTRACE "flag_ptrace"
#define SYSCALL_FIELD_FLAG_READ_AND_WRITE "flag_read_and_write"
#define SYSCALL_FIELD_FLAG_READ_ONLY "flag_read_only"
#define SYSCALL_FIELD_FLAG_REMOVE_DIRECTORY "flag_remove_directory"
#define SYSCALL_FIELD_FLAG_SETTLS "flag_settls"
#define SYSCALL_FIELD_FLAG_SHARED "flag_shared"
#define SYSCALL_FIELD_FLAG_SIGHAND "flag_sighand"
#define SYSCALL_FIELD_FLAG_STACK "flag_stack"
#define SYSCALL_FIELD_FLAG_SYMLINK_FOLLOW "flag_symlink_follow"
#define SYSCALL_FIELD_FLAG_SYMLINK_NOFOLLOW "flag_symlink_nofollow"
#define SYSCALL_FIELD_FLAG_SYNCHRONOUS "flag_synchronous"
#define SYSCALL_FIELD_FLAG_SYSVSEM "flag_sysvsem"
#define SYSCALL_FIELD_FLAG_THREAD "flag_thread"
#define SYSCALL_FIELD_FLAG_TRUNC "flag_trunc"
#define SYSCALL_FIELD_FLAG_TRUNCATE "flag_truncate"
#define SYSCALL_FIELD_FLAG_UNTRACED "flag_untraced"
#define SYSCALL_FIELD_FLAG_VALUE "flag_value"
#define SYSCALL_FIELD_FLAG_VFORK "flag_vfork"
#define SYSCALL_FIELD_FLAG_VM "flag_vm"
#define SYSCALL_FIELD_FLAG_WAITALL "flag_waitall"
#define SYSCALL_FIELD_FLAG_WRITE_ONLY "flag_write_only"
#define SYSCALL_FIELD_FLAG_XATTR_CREATE "flag_xattr_create"
#define SYSCALL_FIELD_FLAG_XATTR_REPLACE "flag_xattr_replace"
#define SYSCALL_FIELD_FLAGS "flags"
#define SYSCALL_FIELD_FLAGS_AT_EACCESS "flags_at_eaccess"
#define SYSCALL_FIELD_FLAGS_AT_EMPTY_PATH "flags_at_empty_path"
#define SYSCALL_FIELD_FLAGS_AT_NO_AUTOMOUNT "flags_at_no_automount"
#define SYSCALL_FIELD_FLAGS_AT_SYMLINK_NOFOLLOW "flags_at_symlink_nofollow"
#define SYSCALL_FIELD_FLAGS_VALUE "flags_value"
#define SYSCALL_FIELD_FULL_NEWNAME "full_newname"
#define SYSCALL_FIELD_FULL_NEWPATHNAME "full_newpathname"
#define SYSCALL_FIELD_FULL_OLDNAME "full_oldname"
#define SYSCALL_FIELD_FULL_OLDPATHNAME "full_oldpathname"
#define SYSCALL_FIELD_FULL_PATHNAME "full_pathname"
#define SYSCALL_FIELD_GENERATED "generated"
#define SYSCALL_FIELD_GIVEN_NEWNAME "given_newname"
#define SYSCALL_FIELD_GIVEN_NEWPATHNAME "given_newpathname"
#define SYSCALL_FIELD_GIVEN_OLDNAME "given_oldname"
#define SYSCALL_FIELD_GIVEN_OLDPATHNAME "given_oldpathname"
#define SYSCALL_FIELD_GIVEN_PATHNAME "given_pathname"
#define SYSCALL_FIELD_HOW "how"
#define SYSCALL_FIELD_IOCTL_BUFFER "ioctl_buffer"
#define SYSCALL_FIELD_IOV_DATA_READ "iov_data_read"
#define SYSCALL_FIELD_IOV_DATA_WRITTEN "iov_data_written"
#define SYSCALL_FIELD_IOV_NUMBER "iov_number"
#define SYSCALL_FIELD_LENGTH "length"
#define SYSCALL_FIELD_LEVEL "level"
#define SYSCALL_FIELD_LINK_VALUE "link_value"
#define SYSCALL_FIELD_LIST_SIZE "list_size"
#define SYSCALL_FIELD_LOCK_LENGTH "lock_length"
#define SYSCALL_FIELD_LOCK_PID "lock_pid"
#define SYSCALL_FIELD_LOCK_START "lock_start"
#define SYSCALL_FIELD_LOCK_TYPE "lock_type"
#define SYSCALL_FIELD_LOCK_TYPE_READ "lock_type_read"
#define SYSCALL_FIELD_LOCK_TYPE_UNLOCKED "lock_type_unlocked"
#define SYSCALL_FIELD_LOCK_TYPE_WRITE "lock_type_write"
#define SYSCALL_FIELD_LOCK_WHENCE "lock_whence"
#define SYSCALL_FIELD_LOCK_WHENCE_CURRENT "lock_whence_current"
#define SYSCALL_FIELD_LOCK_WHENCE_END "lock_whence_end"
#define SYSCALL_FIELD_LOCK_WHENCE_START "lock_whence_start"
#define SYSCALL_FIELD_MMAP_BYTES_REQUESTED "mmap_bytes_requested"
#define SYSCALL_FIELD_MMAP_DATA_READ "mmap_data_read"
#define SYSCALL_FIELD_MMAP_DESCRIPTOR "mmap_descriptor"
#define SYSCALL_FIELD_MMAP_OFFSET "mmap_offset"
#define SYSCALL_FIELD_MOD_TIME "mod_time"
#define SYSCALL_FIELD_MODE_R_GROUP "mode_R_group"
#define SYSCALL_FIELD_MODE_R_OTHERS "mode_R_others"
#define SYSCALL_FIELD_MODE_R_USER "mode_R_user"
#define SYSCALL_FIELD_MODE_W_GROUP "mode_W_group"
#define SYSCALL_FIELD_MODE_W_OTHERS "mode_W_others"
#define SYSCALL_FIELD_MODE_W_USER "mode_W_user"
#define SYSCALL_FIELD_MODE_X_GROUP "mode_X_group"
#define SYSCALL_FIELD_MODE_X_OTHERS "mode_X_others"
#define SYSCALL_FIELD_MODE_X_USER "mode_X_user"
#define SYSCALL_FIELD_MODE_EXECUTE "mode_execute"
#define SYSCALL_FIELD_MODE_EXIST "mode_exist"
#define SYSCALL_FIELD_MODE_GID "mode_gid"
#define SYSCALL_FIELD_MODE_READ "mode_read"
#define SYSCALL_FIELD_MODE_STICKY_BIT "mode_sticky_bit"
#define SYSCALL_FIELD_MODE_UID "mode_uid"
#define SYSCALL_FIELD_MODE_VALUE "mode_value"
#define SYSCALL_FIELD_MODE_WRITE "mode_write"
#define SYSCALL_FIELD_NEW_DESCRIPTOR "new_descriptor"
#define SYSCALL_FIELD_NEW_DESCRIPTOR_CURRENT_WORKING_DIRECTORY "new_descriptor_current_working_directory"
#define SYSCALL_FIELD_NEW_GROUP "new_group"
#define SYSCALL_FIELD_NEW_OWNER "new_owner"
#define SYSCALL_FIELD_NEW_TLS "new_tls"
#define SYSCALL_FIELD_OFFSET "offset"
#define SYSCALL_FIELD_OLD_DESCRIPTOR "old_descriptor"
#define SYSCALL_FIELD_OLD_DESCRIPTOR_CURRENT_WORKING_DIRECTORY "old_descriptor_current_working_directory"
#define SYSCALL_FIELD_OPEN_VALUE "open_value"
#define SYSCALL_FIELD_OPERATION "operation"
#define SYSCALL_FIELD_OPERATION_VALUE "operation_value"
#define SYSCALL_FIELD_OPTION_VALUE "option_value"
#define SYSCALL_FIELD_OPTNAME "optname"
#define SYSCALL_FIELD_PARAMETER "parameter"
#define SYSCALL_FIELD_PARENT_THREAD_ID "parent_thread_id"
#define SYSCALL_FIELD_PGID "pgid"
#define SYSCALL_FIELD_PID "pid"
#define SYSCALL_FIELD_PROTECTION_EXEC "protection_exec"
#define SYSCALL_FIELD_PROTECTION_NONE "protection_none"
#define SYSCALL_FIELD_PROTECTION_READ "protection_read"
#define SYSCALL_FIELD_PROTECTION_VALUE "protection_value"
#define SYSCALL_FIELD_PROTECTION_WRITE "protection_write"
#define SYSCALL_FIELD_PROTOCOL "protocol"
#define SYSCALL_FIELD_READ_DESCRIPTOR "read_descriptor"
#define SYSCALL_FIELD_READAHEAD_OFF "readahead_off"
#define SYSCALL_FIELD_READAHEAD_SIZE "readahead_size"
#define SYSCALL_FIELD_REQUEST "request"
#define SYSCALL_FIELD_RESOURCE "resource"
#define SYSCALL_FIELD_RESOURCE_HARD_LIMIT "resource_hard_limit"
#define SYSCALL_FIELD_RESOURCE_SOFT_LIMIT "resource_soft_limit"
#define SYSCALL_FIELD_RESOURCE_VALUE "resource_value"
#define SYSCALL_FIELD_RETURN_VALUE "return_value"
#define SYSCALL_FIELD_SIGNAL_ABORT "signal_abort"
#define SYSCALL_FIELD_SIGNAL_ALARM "signal_alarm"
#define SYSCALL_FIELD_SIGNAL_BUS "signal_bus"
#define SYSCALL_FIELD_SIGNAL_CHILD "signal_child"
#define SYSCALL_FIELD_SIGNAL_CONTINUE "signal_continue"
#define SYSCALL_FIELD_SIGNAL_CPU_EXCEEDED "signal_cpu_exceeded"
#define SYSCALL_FIELD_SIGNAL_FILE_SIZE_EXCEEDED "signal_file_size_exceeded"
#define SYSCALL_FIELD_SIGNAL_FLOATING_POINT_EXCEPTION "signal_floating_point_exception"
#define SYSCALL_FIELD_SIGNAL_HANGUP "signal_hangup"
#define SYSCALL_FIELD_SIGNAL_ILLEGAL "signal_illegal"
#define SYSCALL_FIELD_SIGNAL_IO "signal_io"
#define SYSCALL_FIELD_SIGNAL_IOT_TRAP "signal_iot_trap"
#define SYSCALL_FIELD_SIGNAL_KILL "signal_kill"
#define SYSCALL_FIELD_SIGNAL_PIPE "signal_pipe"
#define SYSCALL_FIELD_SIGNAL_POWER "signal_power"
#define SYSCALL_FIELD_SIGNAL_PROF_ALARM "signal_prof_alarm"
#define SYSCALL_FIELD_SIGNAL_SEGV "signal_segv"
#define SYSCALL_FIELD_SIGNAL_STACK_FAULT "signal_stack_fault"
#define SYSCALL_FIELD_SIGNAL_STOP "signal_stop"
#define SYSCALL_FIELD_SIGNAL_TERMINAL_INTERRUPT "signal_terminal_interrupt"
#define SYSCALL_FIELD_SIGNAL_TERMINAL_QUIT "signal_terminal_quit"
#define SYSCALL_FIELD_SIGNAL_TERMINAL_STOP "signal_terminal_stop"
#define SYSCALL_FIELD_SIGNAL_TERMINATION "signal_termination"
#define SYSCALL_FIELD_SIGNAL_TRACE_TRAP "signal_trace_trap"
#define SYSCALL_FIELD_SIGNAL_TTY_READ "signal_tty_read"
#define SYSCALL_FIELD_SIGNAL_TTY_WRITE "signal_tty_write"
#define SYSCALL_FIELD_SIGNAL_URGENT "signal_urgent"
#define SYSCALL_FIELD_SIGNAL_USER_DEFINED_1 "signal_user_defined_1"
#define SYSCALL_FIELD_SIGNAL_USER_DEFINED_2 "signal_user_defined_2"
#define SYSCALL_FIELD_SIGNAL_VIRTUAL_ALARM "signal_virtual_alarm"
#define SYSCALL_FIELD_SIGNAL_WINDOW_SIZE_CHANGE "signal_window_size_change"
#define SYSCALL_FIELD_SOCKADDR_BUFFER "sockaddr_buffer"
#define SYSCALL_FIELD_SOCKADDR_LENGTH "sockaddr_length"
#define SYSCALL_FIELD_START_ADDRESS "start_address"
#define SYSCALL_FIELD_STAT_RESULT_ATIME "stat_result_atime"
#define SYSCALL_FIELD_STAT_RESULT_BLKSIZE "stat_result_blksize"
#define SYSCALL_FIELD_STAT_RESULT_BLOCKS "stat_result_blocks"
#define SYSCALL_FIELD_STAT_RESULT_CTIME "stat_result_ctime"
#define SYSCALL_FIELD_STAT_RESULT_DEV "stat_result_dev"
#define SYSCALL_FIELD_STAT_RESULT_GID "stat_result_gid"
#define SYSCALL_FIELD_STAT_RESULT_INO "stat_result_ino"
#define SYSCALL_FIELD_STAT_RESULT_MODE "stat_result_mode"
#define SYSCALL_FIELD_STAT_RESULT_MTIME "stat_result_mtime"
#define SYSCALL_FIELD_STAT_RESULT_NLINK "stat_result_nlink"
#define SYSCALL_FIELD_STAT_RESULT_RAW "stat_result_raw"
#define SYSCALL_FIELD_STAT_RESULT_RDEV "stat_result_rdev"
#define SYSCALL_FIELD_STAT_RESULT_SIZE "stat_result_size"
#define SYSCALL_FIELD_STAT_RESULT_UID "stat_result_uid"
#define SYSCALL_FIELD_STATFS_RESULT_BAVAIL "statfs_result_bavail"
#define SYSCALL_FIELD_STATFS_RESULT_BFREE "statfs_result_bfree"
#define SYSCALL_FIELD_STATFS_RESULT_BLOCKS "statfs_result_blocks"
#define SYSCALL_FIELD_STATFS_RESULT_BSIZE "statfs_result_bsize"
#define SYSCALL_FIELD_STATFS_RESULT_FFREE "statfs_result_ffree"
#define SYSCALL_FIELD_STATFS_RESULT_FILES "statfs_result_files"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS "statfs_result_flags"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_MANDATORY_LOCK "statfs_result_flags_mandatory_lock"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_ACCESS_TIME "statfs_result_flags_no_access_time"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DEV "statfs_result_flags_no_dev"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_DIRECTORY_ACCESS_TIME "statfs_result_flags_no_directory_access_time"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_EXEC "statfs_result_flags_no_exec"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_NO_SET_UID "statfs_result_flags_no_set_uid"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_READ_ONLY "statfs_result_flags_read_only"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_RELATIVE_ACCESS_TIME "statfs_result_flags_relative_access_time"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_SYNCHRONOUS "statfs_result_flags_synchronous"
#define SYSCALL_FIELD_STATFS_RESULT_FLAGS_VALID "statfs_result_flags_valid"
#define SYSCALL_FIELD_STATFS_RESULT_FRSIZE "statfs_result_frsize"
#define SYSCALL_FIELD_STATFS_RESULT_FSID "statfs_result_fsid"
#define SYSCALL_FIELD_STATFS_RESULT_NAMELEN "statfs_result_namelen"
#define SYSCALL_FIELD_STATFS_RESULT_RAW "statfs_result_raw"
#define SYSCALL_FIELD_STATFS_RESULT_TYPE "statfs_result_type"
#define SYSCALL_FIELD_TARGET_PATHNAME "target_pathname"
#define SYSCALL_FIELD_TIME_CALLED "time_called"
#define SYSCALL_FIELD_TIME_RECORDED "time_recorded"
#define SYSCALL_FIELD_TIME_RETURNED "time_returned"
#define SYSCALL_FIELD_TRUNCATE_LENGTH "truncate_length"
#define SYSCALL_FIELD_TYPE "type"
#define SYSCALL_FIELD_UNIQUE_ID "unique_id"
#define SYSCALL_FIELD_UTIME_NOW "utime_now"
#define SYSCALL_FIELD_UTIME_OMIT "utime_omit"
#define SYSCALL_FIELD_VALUE_READ "value_read"
#define SYSCALL_FIELD_VALUE_SIZE "value_size"
#define SYSCALL_FIELD_VALUE_WRITTEN "value_written"
#define SYSCALL_FIELD_WHENCE "whence"
#define SYSCALL_FIELD_WHENCE_VALUE "whence_value"
#define SYSCALL_FIELD_WRITE_DESCRIPTOR "write_descriptor"
#define SYSCALL_FIELD_XATTR_LIST "xattr_list"
#define SYSCALL_FIELD_XATTR_NAME "xattr_name"
#endif /* USE_ENUMS */
