#!/usr/bin/awk -f

BEGIN {
	FS = ","
	rwratio_accuracy = 10
	devsize = "10g"
	devname = "sdc"
	dotfdir = "dotfdir"
	cdffilesdir = "/root/vass/bench-scripts2/cdffiles"
	min = -1
}

! /\#/ {
	dotfname=sprintf("%s/%06d.f", dotfdir, num)

	readnum=$1
	writenum=$2
	riosize=$3
	wiosize=$4

	if (readnum == 0) {
		readnum_norm = 0
		writenum_norm = writenum
	} else {
		writenum_norm = int(writenum/readnum  * rwratio_accuracy + 0.5)
		readnum_norm = rwratio_accuracy
	}

	if (min == -1)
		min = readnum + writenum

	if (readnum + writenum <= min) {
		min = readnum + writenum
		printf "Min at %d:%d\n", num, min
	}

#	printf "%d:%d\n", writenum_norm, readnum_norm

#	printf "Generating %s...\n", dotfname

	printf "define file name=%s,path=/dev,size=%s\n\n",
				devname, devsize > dotfname

	if (readnum != 0)  {
		printf "define cvar name=$riosize_cv,type=rand-empirical,parameters=cdf_filename:%s/%d.iosize.read.dist\n", cdffilesdir, num > dotfname
	}

	if (writenum != 0) 
		printf "define cvar name=$wiosize_cv,type=rand-empirical,parameters=cdf_filename:%s/%d.iosize.write.dist\n", cdffilesdir, num > dotfname

	if ((writenum != 0) || (readnum != 0))
		printf "define cvar name=$offset_cv,type=rand-empirical,parameters=cdf_filename:%s/%d.offset.dist\n\n", cdffilesdir, num > dotfname


	print "define process name=myproc {" > dotfname
	print "  thread name=mythread,memsize=10m {" > dotfname

	if (readnum_norm != 0) {
		if (readnum_norm == 1)
			printf "    flowop read name=myread,filename=%s,iosize=$riosize_cv,offset=$offset_cv,directio\n",\
					 devname > dotfname
		else
			printf "    flowop read name=myread,filename=%s,iosize=$riosize_cv,iters=%d,offset=$offset_cv,directio\n",\
					 devname, readnum_norm - 1 > dotfname
	}

	if (writenum_norm != 0) {
		if (writenum_norm == 1) 
			printf "    flowop write name=mywrite,filename=%s,iosize=$wiosize_cv,offset=$offset_cv,directio\n",\
					 devname > dotfname
		else
			printf "    flowop write name=mywrite,filename=%s,iosize=$wiosize_cv,iters=%d,offset=$offset_cv,directio\n",\
					 devname, writenum_norm - 1 > dotfname
	}

#	if ((readnum_norm * writenum_norm) != 0)
#		oncount = int(readnum/readnum_norm + 0.5) + int(writenum/writenum_norm + 0.5)
#	else if (readnum_norm != 0)
#		oncount = int(readnum/readnum_norm + 0.5)
#	else if (writenum_norm != 0 )
#		oncount = int(writenum/writenum_norm + 0.5)
#	else
#		oncount = 0

	oncount = readnum + writenum
		
	printf "    flowop finishoncount name=myfinish,value=%d\n", oncount > dotfname

	print "}" > dotfname
	print "}\n" > dotfname

	printf "run 100000" > dotfname

	num += 1
}
