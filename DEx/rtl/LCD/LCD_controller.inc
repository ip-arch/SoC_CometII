/*******************************************************************************
*LCD_controller: Generic LCD controller for CFAH1602B-TMC-JP
*                Verilog to NSL interface
********************************************************************************
* 2011.09.11   Initial version  by Y. Okuyama
*******************************************************************************/

declare LCD_controller interface {
   // Host Side
   input     cpu_clk;          /*clock for cpu bus*/
   input     sys_clk;          /* 50MHz clock for timing generation*/
   input     rst;              /*reset signal */
   input     adrs    [5];
   input     from_cpu[16];
   output    to_cpu  [16];
   func_in   we(adrs,from_cpu); 
   func_in   cs;
   // LCD Side
   output    LCD_DATA[8];
   func_out  LCD_RW;
   func_out  LCD_EN;
   func_out  LCD_RS;
}
