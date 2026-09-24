VGA=0xC000;
COLOR=0xF000;
numToAscii=0x30;
int vgaGetAdrs(xin, yin){
	return VGA | ((yin & 0x1F)<<7) | (xin & 0x7F);
}
int inc10(in) {
	if (in>8) return 0;
	else return in+1;
}
count=0;
for(y = 0 ; y < 30 ; y++){
	count = 0;
	for(x = 0; x < 80; x++){
		*(vgaGetAdrs(x,y))= COLOR | numToAscii | count;
		count = inc10(count);
	}
}
halt;

