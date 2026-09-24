#!/usr/bin/perl
# $Id: bdf2mif.pl,v 1.1 2011-08-10 15:41:12+09 knish Exp $

print << "__HEADER__";
DEPTH = 4096;
WIDTH = 8;
ADDRESS_RADIX = HEX;
DATA_RADIX = HEX;
CONTENT BEGIN
__HEADER__

while (<>) {
	if (/^STARTCHAR (.*)/) {
		$CHAR = $1;
	} 
	if (/^ENCODING (\d+)/) {
		$CODE = $1;
		printf("%04X :", $CODE * 16);
	} 
	if (/^ENDCHAR/) {
		$flag = 0;
		printf(";  -- %s\n", $CHAR); 
	}
	if ($flag) {
		printf(" %02X", hex($_));
	}
	if (/^BITMAP/) {
		$flag = 1;
	} 
}

printf("END;\n");
