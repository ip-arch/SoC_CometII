LED = 0xA003;
int wait(){
	for(j=0; j< 0x3; j++)
		for(i=0; i< 0x7FFF; i++);
}
c=0;
while(1){
	wait();
	*LED=c;
	c++;
}
halt;

