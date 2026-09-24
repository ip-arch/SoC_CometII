VGA=0xC000;
COLOR=0xF800;

int vgaGetAdrs(xin, yin){
	return VGA | ((yin & 0x1F)<<7) | (xin & 0x7F);
}

void cls() {
	*(VGA|0xFEC) = COLOR;
	while (*(VGA|0xFEC));
}

cls();

for (y = 0; y < 16; y++) {
	for (x = 0; x < 16; x++) {
		*(vgaGetAdrs(x,y)) = COLOR | (y << 4) | x;
	}
}

count=0;
for (x = 0; x < 80; x++) {
	*(vgaGetAdrs(x,20)) = COLOR | 0x30 | count;
	if (count < 9) {
		count++;
	} else {
		count = 0;
	}
}

halt;
