// ============================================================================
//   Ver  :| Author					:| Mod. Date :| Changes Made:
//   V1.1 :| Alexandra Du			:| 06/01/2016:| Added Verilog file
// ============================================================================


//=======================================================
//  This code is generated by Terasic System Builder
//=======================================================

//`define ENABLE_ADC_CLOCK
`define ENABLE_CLOCK1
//`define ENABLE_CLOCK2
//`define ENABLE_SDRAM
`define ENABLE_HEX0
`define ENABLE_HEX1
`define ENABLE_HEX2
`define ENABLE_HEX3
//`define ENABLE_HEX4
//`define ENABLE_HEX5
`define ENABLE_KEY
//`define ENABLE_LED
//`define ENABLE_SW
`define ENABLE_VGA
//`define ENABLE_ACCELEROMETER
//`define ENABLE_ARDUINO
`define ENABLE_GPIO

module DE10_LITE_Golden_Top(

	//////////// ADC CLOCK: 3.3-V LVTTL //////////
`ifdef ENABLE_ADC_CLOCK
	input 		          		ADC_CLK_10,
`endif
	//////////// CLOCK 1: 3.3-V LVTTL //////////
`ifdef ENABLE_CLOCK1
	input 		          		MAX10_CLK1_50,
`endif
	//////////// CLOCK 2: 3.3-V LVTTL //////////
`ifdef ENABLE_CLOCK2
	input 		          		MAX10_CLK2_50,
`endif

	//////////// SDRAM: 3.3-V LVTTL //////////
`ifdef ENABLE_SDRAM
	output		    [12:0]		DRAM_ADDR,
	output		     [1:0]		DRAM_BA,
	output		          		DRAM_CAS_N,
	output		          		DRAM_CKE,
	output		          		DRAM_CLK,
	output		          		DRAM_CS_N,
	inout 		    [15:0]		DRAM_DQ,
	output		          		DRAM_LDQM,
	output		          		DRAM_RAS_N,
	output		          		DRAM_UDQM,
	output		          		DRAM_WE_N,
`endif

	//////////// SEG7: 3.3-V LVTTL //////////
`ifdef ENABLE_HEX0
	output		     [7:0]		HEX0,
`endif
`ifdef ENABLE_HEX1
	output		     [7:0]		HEX1,
`endif
`ifdef ENABLE_HEX2
	output		     [7:0]		HEX2,
`endif
`ifdef ENABLE_HEX3
	output		     [7:0]		HEX3,
`endif
`ifdef ENABLE_HEX4
	output		     [7:0]		HEX4,
`endif
`ifdef ENABLE_HEX5
	output		     [7:0]		HEX5,
`endif

	//////////// KEY: 3.3 V SCHMITT TRIGGER //////////
`ifdef ENABLE_KEY
	input 		     [1:0]		KEY,
`endif

	//////////// LED: 3.3-V LVTTL //////////
`ifdef ENABLE_LED
	output		     [9:0]		LEDR,
`endif

	//////////// SW: 3.3-V LVTTL //////////
`ifdef ENABLE_SW
	input 		     [9:0]		SW,
`endif

	//////////// VGA: 3.3-V LVTTL //////////
`ifdef ENABLE_VGA
	output		     [3:0]		VGA_B,
	output		     [3:0]		VGA_G,
	output		          		VGA_HS,
	output		     [3:0]		VGA_R,
	output		          		VGA_VS,
`endif

	//////////// Accelerometer: 3.3-V LVTTL //////////
`ifdef ENABLE_ACCELEROMETER
	output		          		GSENSOR_CS_N,
	input 		     [2:1]		GSENSOR_INT,
	output		          		GSENSOR_SCLK,
	inout 		          		GSENSOR_SDI,
	inout 		          		GSENSOR_SDO,
`endif

	//////////// Arduino: 3.3-V LVTTL //////////
`ifdef ENABLE_ARDUINO
	inout 		    [15:0]		ARDUINO_IO,
	inout 		          		ARDUINO_RESET_N,
`endif

	//////////// GPIO, GPIO connect to GPIO Default: 3.3-V LVTTL //////////
`ifdef ENABLE_GPIO
	inout 		    [35:0]		GPIO
`endif
);



//=======================================================
//  REG/WIRE declarations
//=======================================================
wire CLK = MAX10_CLK1_50;
wire PCK;
wire resetn = KEY[1];
wire nes_reset = ~KEY[0];
wire [7:0] key;
wire [15:0] nmi_vec;
wire [15:0] res_vec;
wire [15:0] irq_vec;
wire [15:0] PC;
//wire [7:0] IR;
//wire [7:0] SP;
//wire [31:0] cache;
//wire [255:0] returndata;

assign nmi_vec = 16'h8082;
assign res_vec = 16'h8000;
assign irq_vec = 16'hFFF0;
//assign nmi_vec = 16'hC85F;
//assign res_vec = 16'hC79E;
//assign irq_vec = 16'hFFF0;

wire clk2x;
wire clk;
wire [15:0] VramAddr;
wire [5:0] VramData;

//=======================================================
//  Structural coding
//=======================================================

assign key[0] = ~GPIO[0];
assign key[1] = ~GPIO[1];
assign key[2] = ~GPIO[4];
assign key[3] = ~GPIO[5];
assign key[4] = ~GPIO[6];
assign key[5] = ~GPIO[7];
assign key[6] = ~GPIO[8];
assign key[7] = ~GPIO[9];

reg [2:0] sync_resetn;
always @(posedge clk or negedge resetn) begin
    if (!resetn) begin
        sync_resetn <= 3'b0;
    end else begin
        sync_resetn <= {sync_resetn[1:0], 1'b1};
    end
end


wire [15:0] avmm_1_rw_address;
wire avmm_1_rw_byteenable;
wire avmm_1_rw_read;      
wire [7:0] avmm_1_rw_readdata;
wire avmm_1_rw_write;     
wire [7:0] avmm_1_rw_writedata;

exec_nes nes (
  // Interface: clock (clock end)
  .clock               ( clk ), // 1-bit clk input
  // Interface: reset (reset end)
  .resetn              ( sync_resetn[2] ), // 1-bit reset_n input
  // Interface: clock2x (clock end)
  .clock2x             ( clk2x ), // 1-bit clk input
  // Interface: call (conduit sink)
  .start               ( 1'b1 ), // 1-bit valid input
  .busy                ( ), // 1-bit stall output
  // Interface: return (conduit source)
  .done                ( ), // 1-bit valid output
  .stall               ( 1'b0 ), // 1-bit stall input
  // Interface: returndata (conduit source)
  .returndata          ( PC ), // 16-bit data output
  // Interface: VRAM (conduit sink)
  .VRAM                ( 64'h0 ), // 64-bit data input
  // Interface: nmi_vec (conduit sink)
  .nmi_vec             ( nmi_vec ), // 16-bit data input
  // Interface: res_vec (conduit sink)
  .res_vec             ( res_vec ), // 16-bit data input
  // Interface: irq_vec (conduit sink)
  .irq_vec             ( irq_vec ), // 16-bit data input
  // Interface: key (conduit sink)
  .key                 ( key ), // 8-bit data input
  // Interface: res (conduit sink)
  .res                 ( nes_reset ), // 1-bit data input
  // Interface: avmm_1_rw (avalon start)
  .avmm_1_rw_address   ( avmm_1_rw_address    ), // 16-bit address output
  .avmm_1_rw_byteenable( avmm_1_rw_byteenable ), // 1-bit byteenable output
  .avmm_1_rw_read      ( avmm_1_rw_read       ), // 1-bit read output
  .avmm_1_rw_readdata  ( avmm_1_rw_readdata   ), // 8-bit readdata input
  .avmm_1_rw_write     ( avmm_1_rw_write      ), // 1-bit write output
  .avmm_1_rw_writedata ( avmm_1_rw_writedata  )  // 8-bit writedata output
);

vram vram (
    .clk_clk                        (clk),                        //                 clk.clk
    .reset_reset_n                  (sync_resetn[2]),                  //               reset.reset_n
    .clk_user_clk                   (MAX10_CLK1_50),                   //            clk_user.clk
    .reset_user_reset_n             (sync_resetn[2]),             //          reset_user.reset_n

    .onchip_memory2_0_s2_address    (VramAddr),    // onchip_memory2_0_s2.address
    .onchip_memory2_0_s2_chipselect (1'b1), //                    .chipselect
    .onchip_memory2_0_s2_clken      (1'b1),      //                    .clken
    .onchip_memory2_0_s2_write      (1'b0),      //                    .write
    .onchip_memory2_0_s2_readdata   (VramData),   //                    .readdata
    .onchip_memory2_0_s2_writedata  (8'h0),  //                    .writedata

    .onchip_memory2_0_s1_address    ( avmm_1_rw_address ),    // onchip_memory2_0_s1.address
    .onchip_memory2_0_s1_clken      ( 1'b1 ),      //                    .clken
    .onchip_memory2_0_s1_chipselect ( 1'b1 ), //                    .chipselect
    .onchip_memory2_0_s1_write      ( avmm_1_rw_write ),      //                    .write
    .onchip_memory2_0_s1_readdata   ( avmm_1_rw_readdata ),   //                    .readdata
    .onchip_memory2_0_s1_writedata  ( avmm_1_rw_writedata )   //                    .writedata
);


VGA vga(
    .PCK(PCK),
    .RST(nes_reset),
    .vga_r(VGA_R),
    .vga_g(VGA_G),
    .vga_b(VGA_B), 
    .vga_hs(VGA_HS),
    .vga_vs(VGA_VS),
    .VramAddr(VramAddr),
    .VramData(VramData)
);

SEG7DEC addr0(PC[3:0], HEX0);
SEG7DEC addr1(PC[7:4], HEX1);
SEG7DEC addr2(PC[11:8], HEX2);
SEG7DEC addr3(PC[15:12], HEX3);
//SEG7DEC SP0(SP[3:0], HEX4);
//SEG7DEC SP1(SP[7:4], HEX5);

wire dbg_clk;
PLL	PLL_inst (
	.inclk0 ( MAX10_CLK1_50 ), //50MHz
	.c0 ( clk ), //100MHz
	.c1 ( clk2x ), //200MHz
	.c2 ( PCK ), //25MHz
	.c3 ( dbg_clk )  //5.0MHz
	);

//decode_retdata ret_debug(
//    .returndata(returndata),
//    .PC(PC),
//    .SP(SP)
//);

endmodule

module decode_retdata(
    input [255:0] returndata,

    output [15:0] PC,
    output [7:0] IR,
    output [7:0] SP,
    output [31:0] cache,
    output [7:0] ACC,
    output [7:0] X,
    output [7:0] Y,
    output nmi,

    output [7:0] BGoffset_X,
    output [7:0] BGoffset_Y,
    output VBlank_NMI,
    output BGPtnAddr,
    output SPPtnAddr,
    output NameAddrH,
    output NameAddrL,
    output EnSP,
    output EnBG,
    output SPMSK,
    output BGMSK,
    output VBlank,
    output SPhit,
    output num_ScanSP,

    output [7:0] flag,
    output [7:0] rddata,
    output [15:0] addr
);

assign PC = returndata[15:0];
assign IR = returndata[23:16];
assign SP = returndata[31:24];
assign cache = returndata[63:32];
assign ACC = returndata[71:64];
assign X = returndata[79:72];
assign Y = returndata[87:80];
assign nmi = returndata[88];

assign BGoffset_X = returndata[103:96];
assign BGoffset_Y = returndata[111:104];
assign VBlank_NMI = returndata[112];
assign BGPtnAddr = returndata[120];
assign SPPtnAddr = returndata[128];
assign NameAddrH = returndata[136];
assign NameAddrL = returndata[144];
assign EnSP = returndata[152];
assign EnBG = returndata[160];
assign SPMSK = returndata[168];
assign BGMSK = returndata[176];
assign VBlank = returndata[184];
assign SPhit = returndata[192];
assign num_ScanSP = returndata[200];

assign flag = returndata[215:208];
assign rddata = returndata[223:216];
assign addr = returndata[239:224];

endmodule
