// $Id: VGA_ctrl.inc,v 1.2 2011-08-23 22:54:42+09 knish Exp $

declare VGA_ctrl interface {
	output		R[4], G[4], B[4], HS, VS;
	output		CLOCK, BLANK, SYNC;

	input		adrs[12], w_data[16];
	output		r_data[16];

	func_in		write(adrs, w_data);
	func_in		read(adrs);
	input		m_clock,p_reset;
}


// Video RAM - 16bit/4Kword/Dual-Port RAM
declare RAM2P_16x4K interface {
	input		clock;
	input		address_a[12];
	input		address_b[12];
	input		data_a[16];
	input		data_b[16];
	output		q_a[16];
	output		q_b[16];
	func_in		wren_a();
	func_in		wren_b();
}

// Character Generator ROM - 8bit/4Kword/ROM
declare ROM_8x4K interface {
	input		clock;
	input		address[12];
	output		q[8];
}
