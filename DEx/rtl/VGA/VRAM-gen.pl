#!/usr/bin/perl
# $Id: VRAM-gen.pl,v 1.1 2011-08-10 15:41:12+09 knish Exp $

print << "__HEADER__";
DEPTH = 4096;
WIDTH = 16;
ADDRESS_RADIX = HEX;
DATA_RADIX = HEX;
CONTENT BEGIN
__HEADER__

for ($n = 0; $n < 4096; $n++) {
	if ($n % 128 == 0) {
		printf("\n");
	}
	if ($n % 8 == 0) {
		printf("%04X :", $n);
	}
	if ($n % 128 < 80) {
		printf(" F000");
	} else {
		printf(" 0000");
	}
	if ($n % 8 == 7) {
		printf(";\n");
	}
}

printf("\nEND;\n");
