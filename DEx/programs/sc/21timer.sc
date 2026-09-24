LED=0xA003;
HEX=0xA008;
TIMER_INTREQ = 0x0080;
TIMER_LOAD   = 0x0008;
TIMER_READ   = 0x0004;
TIMER_INTEN  = 0x0002;
TIMER_CNTEN  = 0x0001;
TIMER        = 0xA100;
SNX_INTEN    = 0xff00;
SNX_INTVEC   = 0xff02;

interrupt foo() {
	*(TIMER+2)= *(TIMER+2) & ~TIMER_INTREQ;
	timer++;
	*HEX=timer;
	return;
}

timer=0;
// intrruption vector
*SNX_INTVEC = foo;

// timer initialization
*(TIMER   )= 0xf080;
*(TIMER +1)= 0x2fa;
*(TIMER +2)= *(TIMER+2) | TIMER_LOAD | TIMER_INTEN | TIMER_CNTEN;
*SNX_INTEN = 1;
*HEX=timer;
while(1); // do nothing
halt;
