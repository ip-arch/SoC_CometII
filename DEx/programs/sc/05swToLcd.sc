SW  = 0xA001;
LCD = 0xA020;

for(i=0;i<32;i++) *(LCD+i)=0x20;
while(1) 
	for(i=0;i<32;i++)
		*(LCD+i)=*SW;
halt;
