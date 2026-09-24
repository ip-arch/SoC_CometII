/*******************************************************************************
*cmem: data memory for COMET2
*Verilog to NSL interface for MegaWizard generated module
********************************************************************************
* 2011.09.11   Initial version  by Y. Okuyama
* 2015.08.13   Adapted to Comet2 by N.Shimizu
*******************************************************************************/

declare cmem interface {
	input		clock; 			/* clock signal */
	input		address[13];
	input		data   [16];
	func_in		wren(address, data);
	output		q      [16];
}
