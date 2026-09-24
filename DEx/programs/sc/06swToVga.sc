VGA     = 0xC000;
SW      = 0xA001;
COLOR   = 0xF000;
VGACLS  = 0x0FEC;

int vgaGetAdrs(xin, yin){
	return VGA | ((yin & 0x1F)<<7) | (xin & 0x7F);
}

void vgaCls() {
	*(VGA|VGACLS) = COLOR;
	while (*(VGA|VGACLS));
}

vgaCls();
while(1)
	for(y = 0 ; y < 30 ; y++)
		for(x = 0 ; x < 80 ; x++)
			*(vgaGetAdrs(x,y))=*SW;
halt;

