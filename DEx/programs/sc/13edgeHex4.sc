KEY =0xA000;
KEY3=0x0004;
HEX =0xA008;
count=0;

int edge[2];
edge[0] =1; edge[1] =1;

int dec[16];
dec[ 0] = 0x40; dec[ 1] = 0x79; dec[ 2] = 0x24; dec[ 3] = 0x30;
dec[ 4] = 0x19; dec[ 5] = 0x12; dec[ 6] = 0x02; dec[ 7] = 0x58;
dec[ 8] = 0x00; dec[ 9] = 0x18; dec[10] = 0x08; dec[11] = 0x03;
dec[12] = 0x27; dec[13] = 0x21; dec[14] = 0x06; dec[15] = 0x0e;

int digit[4];
digit[0]=0; digit[1]=0; digit[2]=0; digit[3]=0;

int digitmax[4];
digitmax[0]=9; digitmax[1]=9; digitmax[2]=5; digitmax[3]=9;


int edgeDetect(){
	edge[1]=edge[0];
	edge[0]=(*KEY & KEY3)>>2;
	if(edge[0]==0 && edge[1]==1) return 1;
	else return 0;
}

int segDec(int in){
	if (in>0xF) return dec[in & 0x0F];
	else        return dec[in] | 0x80;
}

int dispDigit(){
	for( i = 0 ; i < 4 ; i++){
		*(HEX+i)=segDec(digit[i]);
	}
}

void countup(){
	for (i = 0; i < 4; i++) {
		if (digit[i] < digitmax[i]) {
			digit[i] = digit[i] + 1;
			return;
		} else {
			digit[i] = 0;
		}
	}
}

while(1){
	dispDigit();
	if(edgeDetect()) countup();
}

halt;
