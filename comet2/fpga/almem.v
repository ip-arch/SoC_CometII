/*
 Produced by NSL Core(version=20150718), IP ARCH, Inc. Thu Aug 13 09:46:39 2015
 Licensed to :EVALUATION USER
*/
/*
 DO NOT USE ANY PART OF THIS FILE FOR COMMERCIAL PRODUCTS. 
*/

module sys ( p_reset , m_clock );
  input p_reset, m_clock;
  wire p_reset, m_clock;
  wire [11:0] _a_address_a;
  wire _a_clock0;
  wire [15:0] _a_data_a;
  wire _a_wren_a;
  wire [15:0] _a_q_a;
  wire _a_aclr0;
  wire _a_aclr1;
  wire [11:0] _a_address_b;
  wire _a_addressstall_a;
  wire _a_addressstall_b;
  wire _a_byteena_a;
  wire _a_byteena_b;
  wire _a_clock1;
  wire _a_clocken0;
  wire _a_clocken1;
  wire _a_clocken2;
  wire _a_clocken3;
  wire [15:0] _a_data_b;
  wire _a_eccstatus;
  wire [15:0] _a_q_b;
  wire _a_rden_a;
  wire _a_rden_b;
  wire _a_wren_b;
altsyncram a (.wren_b(_a_wren_b), .rden_b(_a_rden_b), .rden_a(_a_rden_a), .q_b(_a_q_b), .eccstatus(_a_eccstatus), .data_b(_a_data_b), .clocken3(_a_clocken3), .clocken2(_a_clocken2), .clocken1(_a_clocken1), .clocken0(_a_clocken0), .clock1(_a_clock1), .byteena_b(_a_byteena_b), .byteena_a(_a_byteena_a), .addressstall_b(_a_addressstall_b), .addressstall_a(_a_addressstall_a), .address_b(_a_address_b), .aclr1(_a_aclr1), .aclr0(_a_aclr0), .q_a(_a_q_a), .wren_a(_a_wren_a), .data_a(_a_data_a), .clock0(_a_clock0), .address_a(_a_address_a));
defparam a.width_b = 16;
defparam a.widthad_b = 12;
defparam a.width_byteena_a = 1;
defparam a.width_a = 16;
defparam a.widthad_a = 12;
defparam a.read_during_write_mode_port_a = "NEW_DATA_NO_NBE_READ";
defparam a.power_up_uninitialized = "FALSE";
defparam a.outdata_reg_a = "UNREGISTERED";
defparam a.outdata_aclr_a = "NONE";
defparam a.operation_mode = "SINGLE_PORT";
defparam a.numwords_a = 4096;
defparam a.lpm_type = "altsyncram";
defparam a.lpm_hint = "ENABLE_RUNTIME_MOD=YES,INSTANCE_NAME=dmem";
defparam a.intended_device_family = "Cyclone III";
defparam a.clock_enable_output_a = "BYPASS";
defparam a.clock_enable_input_a = "BYPASS";

   assign  _a_aclr0 = 1'b0;
   assign  _a_aclr1 = 1'b0;
   assign  _a_address_b = 12'b000000000000;
   assign  _a_addressstall_a = 1'b0;
   assign  _a_addressstall_b = 1'b0;
   assign  _a_byteena_a = 1'b1;
   assign  _a_byteena_b = 1'b1;
   assign  _a_clock1 = 1'b1;
   assign  _a_clocken0 = 1'b1;
   assign  _a_clocken1 = 1'b1;
   assign  _a_clocken2 = 1'b1;
   assign  _a_clocken3 = 1'b1;
   assign  _a_data_b = 16'b0000000000000001;
   assign  _a_rden_a = 1'b1;
   assign  _a_rden_b = 1'b1;
   assign  _a_wren_b = 1'b0;
endmodule
/*
 Produced by NSL Core(version=20150718), IP ARCH, Inc. Thu Aug 13 09:46:39 2015
 Licensed to :EVALUATION USER
*/
