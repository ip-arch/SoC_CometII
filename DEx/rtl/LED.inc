/*******************************************************************************
*LED: LED control module
********************************************************************************
* 2011.09.11   Initial version  by Y. Okuyama
*******************************************************************************/

declare LED {
	input		adrs;
	input		datai[16];
	output		datao[16];
	output		LEDR[16];
	output		LEDG[9];
	func_in		write(adrs,datai);
	func_in		read(adrs):datao;
}
