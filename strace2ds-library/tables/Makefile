XML_FILES+=$(wildcard ../xml/*.xml)

../strace2ds-enums.h $(XML_FILES): snia_syscall_fields.src Makefile gen-xml-enums.pl
	perl gen-xml-enums.pl
