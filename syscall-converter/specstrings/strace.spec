# This spec string is for strace
Common(time_called,errno_number,return_value,unique_id);
read(descriptor,data_read,bytes_requested);
write(descriptor,data_written,bytes_requested);
close(descriptor);
open(given_pathname,open_value,flag_read_only,flag_write_only,flag_read_and_write,flag_append,flag_async,flag_close_on_exec,flag_create,flag_direct,flag_directory,flag_exclusive,flag_largefile,flag_no_access_time,flag_no_controlling_terminal,flag_no_follow,flag_no_blocking_mode,flag_no_delay,flag_synchronous,flag_truncate,mode_value,mode_uid,mode_gid,mode_sticky_bit,mode_R_user,mode_W_user,mode_X_user,mode_R_group,mode_W_group,mode_X_group,mode_R_others,mode_W_others,mode_X_others);
lseek(descriptor,offset,whence);
pread(descriptor,data_read,bytes_requested,offset);
truncate(given_pathname,truncate_length);
unlink(given_pathname);
chdir(given_pathname);
symlink(target_pathname,given_pathname);
link(given_oldpathname,given_newpathname);
rmdir(given_pathname);
creat(given_pathname,mode_value,mode_uid,mode_gid,mode_sticky_bit,mode_R_user,mode_W_user,mode_X_user,mode_R_group,mode_W_group,mode_X_group,mode_R_others,mode_W_others,mode_X_others);
stat(given_pathname,stat_result_dev,stat_result_ino,stat_result_mode,stat_result_nlink,stat_result_uid,stat_result_gid,stat_result_blksize,stat_result_blocks,stat_result_size,stat_result_atime,stat_result_mtime,stat_result_ctime);
mkdir(given_pathname,mode_value,mode_R_user,mode_W_user,mode_X_user,mode_R_group,mode_W_group,mode_X_group,mode_R_others,mode_W_others,mode_X_others);

