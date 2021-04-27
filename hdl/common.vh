`ifndef common_vh_
`define common_vh_

`define VERSION_MAJOR 4'd0
`define VERSION_MINOR 4'd2

// Pick a board. MojoV3 'hat' is still working but
// it will be dropped soon.
//`define SIMULATOR_BOARD 1
//`define REV_1_BOARD 1
`define MOJOV3_BOARD 1

// Annoying
`ifdef SIMULATOR_BOARD
`define REV_1_BOARD_OR_SIMULATOR_BOARD 1
`endif
`ifdef REV_1__BOARD
`define REV_1_BOARD_OR_SIMULATOR_BOARD 1
`endif

// This shows a test pattern with colors and some text.
// Useful for testing video output from the device without
// it being plugged into a C64. This will use approx 16k
// of block ram for the pixel data.
//`define TEST_PATTERN 1

// Notes on config permutations:
//
// This core can be configured to output video in different ways.
//    DVI/HDMI (8 differential pairs, from scan doubled RGB values)
//    VGA (scan doubled RGB + sync + clock signals)
//    External Composite Encoder (native rgb + csync + color ref signals)
//    LUMA/CHROMA (luma + chroma for a DAC)
//
// LUMA/CHROMA can work simultaneously with any other video output
// method since it works off of pixel index values coming straight
// out of the pixel sequencer.  It requires HAVE_COLOR_CLOCKS. This
// video output mode ignores custom RGB palette registers.  Instead, it
// uses luma, phase, amplitude for each of the 16 colors.
//
// An external composite encoder also requires HAVE_COLOR_CLOCKS.
// For proper video output, use_scan_doubler must be set to false. It
// can be included in one bitstream and work with DVI/HDMI/VGA but
// not simultaneously (you must toggle set use_scan_doubler true for a
// valid DVI/HDMI/VGA picture and set it false for the encoder.)
//
// VGA can work without HAVE_COLOR_CLOCKS but use_scan_doubler is always
// true if color clocks are not available.
//
// Similarly, DVI/HDMI can also work without HAVE_COLOR_CLOCKS but
// use_scan_doubler is always true if color clocks are not available.
//
// The prototype 'hat' for the mojov3 was originally designed with no
// ntsc/pal color clocks going into the board. In this case, we rely soley
// on the on-board 50mhz clock to generate our pixel clocks for both ntsc
// and pal. This caused some routing and placement issues and won't be
// necessary on the final pcb since we have figured out how to properly
// generate dot4x clocks from color clocks. For 'plain' unmodified
// boards to still work, leave HAVE_COLOR_CLOCKS undefined (i.e. the one
// Adrian Black has.)
//
// If HAVE_COLOR_CLOCKS is defined, then either GEN_LUMA_CHROMA or
// HAVE_COMPOSITE_ENCODER (or both) can be defined. Since the board can now
// produce luma/chroma itself, a composite encoder is not necessary but the
// code is kept functional (for now). DVI/VGA will still work as long as
// the free pins has not been exhausted.

// Uncomment to include TMDS outputs and DVI encoder for video
//`define WITH_DVI 1

// Uncomment if the board has both PAL and NTSC color clocks
// available.
`define HAVE_COLOR_CLOCKS 1

// Uncomment if we have an external composite encoder wired
// up. This will output csync and color ref clock signals that
// can feed into an RGB to composite encoder IC. This requires
// HAVE_COLOR_CLOCKS to be defined.
//`define HAVE_COMPOSITE_ENCODER 1

// Uncomment if we shuold generate luma and chroma signals.
// This requires HAVE_COLOR_CLOCKS to be defined.
`define GEN_LUMA_CHROMA 1

// Uncomment to activate registers a0-cf and 80,81 to control
// luma(a#), phase(b#) and amplitudes(c#) for the 16 colors as
// well as blanking level (80) and burst amplitude (81).
`define CONFIGURABLE_LUMAS 1

// Uncomment to activate registers d0-ef to control HDMI/VGA
// timings for all the supported resolutions.  This take up a lot
// space on the device.  Not intended for the release, only to
// be used as a tool to find correct timings.
//`define CONFIGURABLE_TIMING 1

// Uncomment to average the luma values over 4 ticks of the
// dot4x clock. This smooths out transitions between levels.
//`define AVERAGE_LUMAS 1

// Uncomment if board has serial link between MCU and FPGA
`define HAVE_SERIAL_LINK 1


// DATA_DAV
//
// When to read from the data bus for both char/pixel and sprite dma
// in terms of phi_phase_start index.  This can't be changed without
// some serious rework of xpos, read delays in the pixel sequencer
// and many other timing values elsewhere. Zero value tells the bus
// access module to read data on the edge of phi as indicated
// by the datasheet. But we use a much earlier value for VICE
// simuation comparison.

// PIXEL_LATCH
//
// When to transfer char/pixel data into the final delayed register
// from the delay pipeline.  This is chosen so that the data
// in the final delay register is first available when load_pixels
// rises (xpos_mod_8 == 0)

`define DATA_DAV 0
`define DATA_DAV_PLUS_1 1
`define DATA_DAV_PLUS_2 2
`define M2CLR_CHECK 1
`define M2CLR_PHASE !clk_phi
`define SPRITE_CRUNCH_CYCLE_CHECK 15
`define PIXEL_LATCH 0

// Will never change but used in loops
`define NUM_SPRITES 8

// cycle_bit values for sprite pixels
`define SPRITE_PIXEL_0 2
`define SPRITE_PIXEL_1 3
`define SPRITE_PIXEL_2 4
`define SPRITE_PIXEL_3 5
`define SPRITE_PIXEL_4 6
`define SPRITE_PIXEL_5 7
`define SPRITE_PIXEL_6 0
`define SPRITE_PIXEL_7 1

// dot_rising values for pixel ticks
`define PIXEL_TICK_0 1
`define PIXEL_TICK_1 2
`define PIXEL_TICK_2 3
`define PIXEL_TICK_3 0

// Chip types
`define CHIP6567R8   0
`define CHIP6569R5   1
`define CHIP6567R56A 2
`define CHIP6569R1   3

// Cycle types
`define VIC_LP     0  // low phase, sprite pointer
`define VIC_LPI2   1  // low phase, sprite idle
`define VIC_LS2    2  // low phase, sprite dma byte 2
`define VIC_LR     3  // low phase, dram refresh
`define VIC_LG     4  // low phase, g-access
`define VIC_HS1    5  // high phase, sprite dma byte 1
`define VIC_HPI1   6  // high phase, sprite idle
`define VIC_HPI3   7  // high phase, sprite idle
`define VIC_HS3    8  // high phase, sprite dma byte 3
`define VIC_HRI    9  // high phase, refresh idle
`define VIC_HRC    10  // high phase, c-access after r
`define VIC_HGC    11  // high phase, c-access after g
`define VIC_HGI    12  // high phase, cached-c-access after g
`define VIC_HI     13  // high phase, idle
`define VIC_LI     14  // low phase, idle
`define VIC_HRX    15  // high phase, cached-c-access after r

`define TRUE	1'b1
`define FALSE	1'b0

// Colors
`define BLACK        4'd0
`define WHITE        4'd1
`define RED          4'd2
`define CYAN         4'd3
`define PURPLE       4'd4
`define GREEN        4'd5
`define BLUE         4'd6
`define YELLOW       4'd7
`define ORANGE       4'd8
`define BROWN        4'd9
`define PINK         4'd10
`define DARK_GREY    4'd11
`define GREY         4'd12
`define LIGHT_GREEN  4'd13
`define LIGHT_BLUE   4'd14
`define LIGHT_GREY   4'd15

// Registers
`define REG_SPRITE_X_0                6'h00
`define REG_SPRITE_Y_0                6'h01
`define REG_SPRITE_X_1                6'h02
`define REG_SPRITE_Y_1                6'h03
`define REG_SPRITE_X_2                6'h04
`define REG_SPRITE_Y_2                6'h05
`define REG_SPRITE_X_3                6'h06
`define REG_SPRITE_Y_3                6'h07
`define REG_SPRITE_X_4                6'h08
`define REG_SPRITE_Y_4                6'h09
`define REG_SPRITE_X_5                6'h0A
`define REG_SPRITE_Y_5                6'h0B
`define REG_SPRITE_X_6                6'h0C
`define REG_SPRITE_Y_6                6'h0D
`define REG_SPRITE_X_7                6'h0E
`define REG_SPRITE_Y_7                6'h0F
`define REG_SPRITE_X_BIT_8            6'h10
`define REG_SCREEN_CONTROL_1          6'h11
`define REG_RASTER_LINE               6'h12
`define REG_LIGHT_PEN_X               6'h13
`define REG_LIGHT_PEN_Y               6'h14
`define REG_SPRITE_ENABLE             6'h15
`define REG_SCREEN_CONTROL_2          6'h16
`define REG_SPRITE_EXPAND_Y           6'h17
`define REG_MEMORY_SETUP              6'h18
`define REG_INTERRUPT_STATUS          6'h19
`define REG_INTERRUPT_CONTROL         6'h1a
`define REG_SPRITE_PRIORITY           6'h1b
`define REG_SPRITE_MULTICOLOR_MODE    6'h1c
`define REG_SPRITE_EXPAND_X           6'h1d
`define REG_SPRITE_2_SPRITE_COLLISION 6'h1e
`define REG_SPRITE_2_DATA_COLLISION   6'h1f
`define REG_BORDER_COLOR              6'h20
`define REG_BACKGROUND_COLOR_0        6'h21
`define REG_BACKGROUND_COLOR_1        6'h22
`define REG_BACKGROUND_COLOR_2        6'h23
`define REG_BACKGROUND_COLOR_3        6'h24
`define REG_SPRITE_MULTI_COLOR_0      6'h25
`define REG_SPRITE_MULTI_COLOR_1      6'h26
`define REG_SPRITE_COLOR_0            6'h27
`define REG_SPRITE_COLOR_1            6'h28
`define REG_SPRITE_COLOR_2            6'h29
`define REG_SPRITE_COLOR_3            6'h2A
`define REG_SPRITE_COLOR_4            6'h2B
`define REG_SPRITE_COLOR_5            6'h2C
`define REG_SPRITE_COLOR_6            6'h2D
`define REG_SPRITE_COLOR_7            6'h2E

`define REG_UNUSED1                   6'h2F
`define REG_UNUSED2                   6'h30
`define REG_UNUSED3                   6'h31
`define REG_UNUSED4                   6'h32
`define REG_UNUSED5                   6'h33
`define REG_UNUSED6                   6'h34
`define REG_UNUSED7                   6'h35
`define REG_UNUSED8                   6'h36

// --- BEGIN EXTENSIONS ---
`define VMEM_FLAG_PORT1_FUNCTION 1:0
`define VMEM_FLAG_PORT2_FUNCTION 3:2
`define VMEM_FLAG_PERSIST_BUSY 4
`define VMEM_FLAG_REGS_OVERLAY_BIT 5
`define VMEM_FLAG_PERSIST_BIT 6
`define VMEM_FLAG_DISABLE_BIT 7

`define VIDEO_MEM_1_IDX               6'h35
`define VIDEO_MEM_2_IDX               6'h36
`define VIDEO_MODE1                   6'h37
`define VIDEO_MODE2                   6'h38
`define VIDEO_MEM_1_LO                6'h39
`define VIDEO_MEM_1_HI                6'h3A
`define VIDEO_MEM_1_VAL               6'h3B
`define VIDEO_MEM_2_LO                6'h3C
`define VIDEO_MEM_2_HI                6'h3D
`define VIDEO_MEM_2_VAL               6'h3E
`define VIDEO_MEM_FLAGS               6'h3F  // Extra Reg Activation Port

// For VIDEO_MODE_1
`define PALETTE_SELECT_BIT            3
`define HIRES_ENABLE                  4
`define HIRES_TEXT_BITMAP             5
`define HIRES_COLOR_2K_16K            6

`define EXT_REG_BLANKING             8'h80
`define EXT_REG_BURSTAMP             8'h81
`define EXT_REG_CHIP_MODEL           8'h82
`define EXT_REG_VERSION              8'h83
`define EXT_REG_DISPLAY_FLAGS        8'h84
`define EXT_REG_CURSOR_LO            8'h85
`define EXT_REG_CURSOR_HI            8'h86

`ifdef CONFIGURABLE_LUMAS
`define EXT_REG_LUMA0                8'ha0
`define EXT_REG_LUMA1                8'ha1
`define EXT_REG_LUMA2                8'ha2
`define EXT_REG_LUMA3                8'ha3
`define EXT_REG_LUMA4                8'ha4
`define EXT_REG_LUMA5                8'ha5
`define EXT_REG_LUMA6                8'ha6
`define EXT_REG_LUMA7                8'ha7
`define EXT_REG_LUMA8                8'ha8
`define EXT_REG_LUMA9                8'ha9
`define EXT_REG_LUMA10               8'haa
`define EXT_REG_LUMA11               8'hab
`define EXT_REG_LUMA12               8'hac
`define EXT_REG_LUMA13               8'had
`define EXT_REG_LUMA14               8'hae
`define EXT_REG_LUMA15               8'haf

`define EXT_REG_PHASE0                8'hb0
`define EXT_REG_PHASE1                8'hb1
`define EXT_REG_PHASE2                8'hb2
`define EXT_REG_PHASE3                8'hb3
`define EXT_REG_PHASE4                8'hb4
`define EXT_REG_PHASE5                8'hb5
`define EXT_REG_PHASE6                8'hb6
`define EXT_REG_PHASE7                8'hb7
`define EXT_REG_PHASE8                8'hb8
`define EXT_REG_PHASE9                8'hb9
`define EXT_REG_PHASE10               8'hba
`define EXT_REG_PHASE11               8'hbb
`define EXT_REG_PHASE12               8'hbc
`define EXT_REG_PHASE13               8'hbd
`define EXT_REG_PHASE14               8'hbe
`define EXT_REG_PHASE15               8'hbf

`define EXT_REG_AMPL0                8'hc0
`define EXT_REG_AMPL1                8'hc1
`define EXT_REG_AMPL2                8'hc2
`define EXT_REG_AMPL3                8'hc3
`define EXT_REG_AMPL4                8'hc4
`define EXT_REG_AMPL5                8'hc5
`define EXT_REG_AMPL6                8'hc6
`define EXT_REG_AMPL7                8'hc7
`define EXT_REG_AMPL8                8'hc8
`define EXT_REG_AMPL9                8'hc9
`define EXT_REG_AMPL10               8'hca
`define EXT_REG_AMPL11               8'hcb
`define EXT_REG_AMPL12               8'hcc
`define EXT_REG_AMPL13               8'hcd
`define EXT_REG_AMPL14               8'hce
`define EXT_REG_AMPL15               8'hcf

`define EXT_REG_TIMING_REG_START     8'hd0
`define EXT_REG_TIMING_REG_END       8'hef

`endif

// Bits in display flags
`define SHOW_RASTER_LINES_BIT        0
`define IS_NATIVE_Y_BIT              1     // a.k.a 15khz
`define IS_NATIVE_X_BIT              2

`define EXT_REG_VARIANT_NAME1        8'h90
`define EXT_REG_VARIANT_NAME2        8'h91
`define EXT_REG_VARIANT_NAME3        8'h92
`define EXT_REG_VARIANT_NAME4        8'h93
`define EXT_REG_VARIANT_NAME5        8'h94
`define EXT_REG_VARIANT_NAME6        8'h95
`define EXT_REG_VARIANT_NAME7        8'h96
`define EXT_REG_VARIANT_NAME8        8'h97
`define EXT_REG_VARIANT_NAME9        8'h98
`define EXT_REG_VARIANT_NAME10       8'h99
`define EXT_REG_VARIANT_NAME11       8'h9a
`define EXT_REG_VARIANT_NAME12       8'h9b
`define EXT_REG_VARIANT_NAME13       8'h9c
`define EXT_REG_VARIANT_NAME14       8'h9d
`define EXT_REG_VARIANT_NAME15       8'h9e
`define EXT_REG_VARIANT_NAME16       8'h9f

`define VARIANT_NAME1   8'h4F  // O
`define VARIANT_NAME2   8'h46  // F
`define VARIANT_NAME3   8'h46  // F
`define VARIANT_NAME4   8'h49  // I
`define VARIANT_NAME5   8'h43  // C
`define VARIANT_NAME6   8'h49  // I
`define VARIANT_NAME7   8'h41  // A
`define VARIANT_NAME8   8'h4C  // L

`define HIRES_BLINK_FREQ 5  // every 32 frames

// These are the same as C128's VDC
`define HIRES_BLNK_BIT 4   // color cell attr bit for blink
`define HIRES_UNDR_BIT 5   // color cell attr bit for underline
`define HIRES_RVRS_BIT 6   // color cell attr bit for reverse
`define HIRES_ALTC_BIT 7   // color cell attr bit for alt char set

// --- END EXTENSIONS ---

// Official video modes, source https://www.c64-wiki.com/wiki/Graphics_Modes
`define MODE_STANDARD_CHAR       3'b000
`define MODE_MULTICOLOR_CHAR     3'b001
`define MODE_STANDARD_BITMAP     3'b010
`define MODE_MULTICOLOR_BITMAP   3'b011
`define MODE_EXTENDED_BG_COLOR   3'b100
// "Illegal" invalid modes.
`define MODE_INV_EXTENDED_BG_COLOR_MULTICOLOR_CHAR     3'b101
`define MODE_INV_EXTENDED_BG_COLOR_STANDARD_BITMAP     3'b110
`define MODE_INV_EXTENDED_BG_COLOR_MULTICOLOR_BITMAP   3'b111

`endif // common_vh_
