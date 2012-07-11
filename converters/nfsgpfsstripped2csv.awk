BEGIN {
	OFS=","
}


/TRACE_VNOP: READ:/ {
	print "read",$1,$8,$14,$16,""
}

/TRACE_VNOP: WRITE:/ {
	print "write",$1,$8,$14,$16,"","","",""
}

END{}
