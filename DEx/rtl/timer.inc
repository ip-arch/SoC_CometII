// 2014/03/11  Shimizu and Okuyama

declare timer {
	input		adrs[2];
	input		datai[16];
	output		datao[16];
	output 		debug[32];
	func_in		read(adrs):datao;
	func_in		write(adrs,datai);
	func_out	intr();
}
