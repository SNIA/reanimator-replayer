#!/usr/bin/awk -f

BEGIN {
	FS = ","
	rwratio_accuracy = 10
	devsize = "6000m"
	devname = "sdc"
	dotfdir = "dotfdir"
	min = -1
}

! /\#/ {
	num += 1
	dotfname=sprintf("%s/%06d.f", dotfdir, num)

	readnum=$1
	writenum=$2
	riosize=$3
	wiosize=$4

	writenum_norm = int(writenum/readnum  * rwratio_accuracy + 0.5)
	readnum_norm = rwratio_accuracy

	if (min == -1)
		min = readnum + writenum

	if (readnum + writenum < min) {
		min = readnum + writenum
		printf "Min at %d:%d\n", num, min
	}

#	printf "%d:%d\n", writenum_norm, readnum_norm

#	printf "Generating %s...\n", dotfname

	printf "define file name=%s,path=/dev,size=%s\n\n",
				devname, devsize > dotfname

	print "define process name=myproc {" > dotfname
	print "  thread name=mythread,memsize=10m {" > dotfname

	if (readnum_norm != 0) {
		if (readnum_norm == 1)
			printf "    flowop read name=myread,filename=%s,iosize=%d,random\n",\
					 devname, riosize > dotfname
		else
			printf "    flowop read name=myread,filename=%s,iosize=%d,iters=%d,random\n",\
					 devname, riosize, readnum_norm - 1 > dotfname
	}

	if (writenum_norm != 0) {
		if (writenum_norm == 1) 
			printf "    flowop write name=mywrite,filename=%s,iosize=%d,random\n",\
					 devname, riosize > dotfname
		else
			printf "    flowop write name=mywrite,filename=%s,iosize=%d,iters=%d,random\n",\
					 devname, riosize, writenum_norm - 1 > dotfname
	}

	printf "    flowop finishoncount name=myfinish,value=%d\n", readnum + writenum > dotfname

	print "}" > dotfname
	print "}\n" > dotfname

	printf "run 100000" > dotfname
}

