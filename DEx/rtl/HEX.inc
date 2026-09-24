/*******************************************************************************
*HEX: 7-segment LED controller
********************************************************************************
* 2011.09.11   Initial version  by Y. Okuyama
*******************************************************************************/
declare HEX {
	input		adrs[3];
	input 		datai[16];
	output 		datao[16];
	output		HEX0[8];
	output		HEX1[8];
	output		HEX2[8];
	output		HEX3[8];
	output		HEX4[8];
	output		HEX5[8];
	output		HEX6[8];
	output		HEX7[8];
	func_in		write(adrs,datai);
	func_in		read(adrs):datao;
}
