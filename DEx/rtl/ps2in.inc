/*
PS/2 keyboard input (not for mouse)
*/
declare ps2_in {
 input rxd;
 input rxc;
 output data[8];
 func_in port_read():data;
 func_out rxready;
}
