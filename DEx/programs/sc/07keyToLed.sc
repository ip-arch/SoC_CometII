KEY  = 0xA000;
LED  = 0xA003;
KEY0 = 0x0001; // cannot use
KEY1 = 0x0002;
KEY2 = 0x0004;
KEY3 = 0x0008; // not implemented on DE0

while(1) {
	flag=0;
	if ((*KEY & KEY2)==0) flag= 0xf0 | flag; 
	if ((*KEY & KEY1)==0) flag= 0x0f | flag;
	*LED=flag;
}
halt;


