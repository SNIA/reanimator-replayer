#!/usr/bin/perl -w
# Generate xml files in ../xml/<syscall>.xml
# Generate ../strace2ds-enums.h

$debug = 1;

%all_common_fields = ();	# all common fields -> <is_nullable,type>
%all_field_names = ();		# names of all fields -> unique field_id num
%all_syscall_names = ();	# names of all syscalls -> 1
%all_syscalls_fields = ();	# $syscallname{$fieldname} -> <is_nullable,type>

$sysnum = 0;			# syscall number assigned by us
$file = "snia_syscall_fields.src";
open(FILE, "$file") || die("$file: $!");
while(($line = <FILE>)) {
    chop $line;                 # eliminate newline
    printf(STDERR "LINE: %s\n", $line) if $debug > 1;
    next if ($line =~ /^#/); 	# ignore comment lines
    die if ($line =~ /^$/);	# disallow empty lines
    # parse common lines: <syscallname fieldname is_nullable type>
    if ($line =~ /^(\w+)\s+(\w+)\s+([01])\s+(\w+)$/) {
	$syscall_name = $1;
	$field_name = $2;
	$is_nullable = $3;
	$type = $4;
    } elsif ($line =~ /^(\w+)$/) { # special case: syscall with no custom params
	$syscall_name = $1;
	$field_name = undef;
	$is_nullable = undef;
	$type = undef;
    } else {
	die("CANNOT PARSE LINE: \"%s\"\n", $line);
    }
    # now fill in the various structures
    if ($syscall_name eq "Common") {
	die unless defined($field_name);
	$all_common_fields{$field_name} =
	    sprintf("type=\"%s\" opt_nullable=\"%s\"",
		    $type, $is_nullable ? "yes" : "no");
    }
    if (defined($field_name)) {
	$all_field_names{$field_name} = 1;
	printf(STDERR "S=%s N=%s I=%d T=%s\n", $syscall_name, $field_name,
	       $is_nullable, $type) if $debug > 1;
	$all_syscalls_fields{$syscall_name}{$field_name} =
	    sprintf("type=\"%s\" opt_nullable=\"%s\"",
		    $type, $is_nullable ? "yes" : "no");
	printf(STDERR "\t%s\n",
	       $all_syscalls_fields{$syscall_name}{$field_name}) if $debug > 1;
    }
    $all_syscall_names{$syscall_name} = $sysnum++
	if (!defined($all_syscall_names{$syscall_name}));
}
close(FILE);

######################################################################
# assign unique field_id numbers to each field name
$count = 0;
foreach $k (sort {$a cmp $b} keys %all_field_names) {
    $all_field_names{$k} = $count;
    $count++;
}

######################################################################
# now we can generate output files: start with syscall-enums.h
# note: also generate #define's for string names, for backwards compatibility
$file = "../strace2ds-enums.h";
open(FILE, ">$file") || die("$file: $!");
$now_string = localtime;
printf(FILE "/*\n");
printf(FILE " * This file was auto generated on %s.\n", $now_string);
printf(FILE " * DO NOT EDIT BY HAND.\n");
printf(FILE " */\n");
printf(FILE "#ifdef USE_ENUMS\n");
printf(FILE "enum syscall_names {\n");
$count = 0;
foreach $k (sort {$all_syscall_names{$a} <=> $all_syscall_names{$b}}
	    keys %all_syscall_names) {
    printf(FILE "\tSYSCALL_NAME_%s = %d,\n", uc($k), $all_syscall_names{$k});
    $count++;			# to count for max_syscalls
}
printf(FILE "\tMAX_SYSCALL_NAMES = %d\n", $count);
printf(FILE "};\n");
printf(FILE "\n");
printf(FILE "enum syscall_fields {\n");
$count = 0;
foreach $k (sort {$a cmp $b} keys %all_field_names) {
    printf(FILE "\tSYSCALL_FIELD_%s = %d,\n", uc($k), $all_field_names{$k});
    $count++;			# better be 1 more than last $k
}
printf(FILE "\tMAX_SYSCALL_FIELDS = %d\n", $count);
printf(FILE "};\n");
printf(FILE "#else /* USE_ENUMS */\n");
foreach $k (sort {$a cmp $b} keys %all_syscall_names) {
    printf(FILE "#define SYSCALL_NAME_%s \"%s\"\n", uc($k), $k);
}
foreach $k (sort {$a cmp $b} keys %all_field_names) {
    printf(FILE "#define SYSCALL_FIELD_%s \"%s\"\n", uc($k), $k);
}
printf(FILE "#endif /* USE_ENUMS */\n");
close(FILE);

######################################################################
# generate individual xml files by system call
foreach $k (sort {$a cmp $b} keys %all_syscall_names) {
    next if ($k eq "Common"); # not a real syscall
    $file = sprintf("../xml/%s.xml", $k);
    open(FILE, ">$file") || die("$file: $!");
    printf(FILE "<ExtentType name=\"IOTTAFSL::Trace::Syscall::%s\" ", $k);
    printf(FILE "namespace=\"iotta.snia.org\" version=\"1.00\">\n");
    # print common fields
    foreach $c (sort {$a cmp $b} keys %all_common_fields) {
	printf(FILE "  <field ");
	printf(FILE "field_id=\"%d\" ", $all_field_names{$c});
	printf(FILE "name=\"%s\" ", $c);
	printf(FILE "%s/>\n", $all_common_fields{$c});
    }
    # print per-syscall fields (if any, few don't have any args)
    foreach $c (sort {$a cmp $b} keys %{$all_syscalls_fields{$k}}) {
  	printf(FILE "  <field ");
  	printf(FILE "field_id=\"%d\" ", $all_field_names{$c});
  	printf(FILE "name=\"%s\" ", $c);
	printf(FILE "%s/>\n", $all_syscalls_fields{$k}{$c});
    }
    printf(FILE "</ExtentType>\n");
    close(FILE);
}

######################################################################
# now re-read input table file and produce output file with field IDs
$infile = "snia_syscall_fields.src";
$outfile = "snia_syscall_fields.table";
open(INFILE, "$infile") || die("$infile: $!");
open(OUTFILE, ">$outfile") || die("$outfile: $!");
printf(OUTFILE "# SNIA SYSCALL TABLE FILE\n");
printf(OUTFILE "# DO NOT EDIT BY HAND: Auto-generated on %s\n", $now_string);
printf(OUTFILE "# Format: syscall_name syscall_id field_id field_name is_nullable type\n");
while(($line = <INFILE>)) {
    chop $line;                 # eliminate newline
    printf(STDERR "LINE: %s\n", $line) if $debug > 1;
    next if ($line =~ /^#/); 	# ignore comment lines
    die if ($line =~ /^$/);	# disallow empty lines
    # parse common lines: <syscallname fieldname is_nullable type>
    if ($line =~ /^(\w+)\s+(\w+)\s+([01])\s+(\w+)$/) {
	$syscall_name = $1;
	$field_name = $2;
	$is_nullable = $3;
	$type = $4;
    } elsif ($line =~ /^(\w+)$/) { # special case: syscall with no custom params
	$syscall_name = $1;
	$field_name = undef;
	$is_nullable = undef;
	$type = undef;
    } else {
	die("CANNOT PARSE LINE: \"%s\"\n", $line);
    }
    # now output a line
    if (defined($field_name)) {
	printf(OUTFILE "%s\t%d\t%d\t%s\t%d\t%s\n",
	       $syscall_name,
	       $all_syscall_names{$syscall_name},
	       $all_field_names{$field_name},
	       $field_name, $is_nullable, $type);
    } else { # use -1 for field ID if no fields
	printf(OUTFILE "%s\t%d\t%d\n",
	       $syscall_name,
	       $all_syscall_names{$syscall_name},
	       -1);
    }

}
close(INFILE);
close(OUTFILE);
