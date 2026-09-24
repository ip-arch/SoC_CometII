LED = 0xA003;
PS2DAT = 0xA004;
PS2IN  = 0xA005;

while(1)
  if(*PS2IN) *LED=*PS2DAT;

halt;
