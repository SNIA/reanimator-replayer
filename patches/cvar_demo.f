# 
# An example of custom variable usage. 
#
# Author: Santhosh Kumar Koundinya (santhosh@fsl.cs.sunysb.edu)
#

set $dir=/tmp

define cvar name=$iosize,type=rand-triangular,parameters=lower:512;upper:8192;mode:4096,round=512
define cvar name=$offset,type=rand-empirical,parameters=cdf_filename:/home/santhosh/tmp/sample.cdf

define file name=f1,path=$dir,size=1g,prealloc,reuse,paralloc

define process name=p1,instances=1
{
  thread name=t1,memsize=5m,instances=1
  {
    flowop read name=r1,filename=f1,iosize=$iosize,offset=$offset
  }
}

echo  "Custom variable usage example personality successfully loaded"
usage "Usage: set \$dir=<dir>          defaults to $dir"
usage "       set \$directio=<0 disable or 1 enable>"
usage " "
usage "       run runtime (e.g. run 60)"

run 10
