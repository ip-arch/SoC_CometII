SW =0xA001;
HEX=0xA008;

int dec[16];
dec[ 0] = 0x40; dec[ 1] = 0x79; dec[ 2] = 0x24; dec[ 3] = 0x30;
dec[ 4] = 0x19; dec[ 5] = 0x12; dec[ 6] = 0x02; dec[ 7] = 0x58;
dec[ 8] = 0x00; dec[ 9] = 0x18; dec[10] = 0x08; dec[11] = 0x03;
dec[12] = 0x27; dec[13] = 0x21; dec[14] = 0x06; dec[15] = 0x0e;

int segDec(int in){
	if (in>0xF) return dec[in & 0x0F];
	else        return dec[in] | 0x80;
}                               

while(1)
	*HEX=segDec(*SW);
halt;
