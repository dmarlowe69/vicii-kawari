`ifndef config_vh_
`define config_vh_

`define VERSION_MAJOR 4'd0
`define VERSION_MINOR 4'd2

// Pick a board.
//`define SIMULATOR_BOARD 1
//`define CMOD_BOARD 1
//`define REV_2_BOARD 1
`define REV_3_BOARD 1

// Notes on config permutations:
//
// This core can be configured to output video in different ways.
//    DVI/HDMI (8 differential pairs, from scan doubled RGB values)
//    VGA (scan doubled RGB + sync + clock signals)
//    LUMA/CHROMA (luma + chroma, luma needs voltage conv + DAC)
//
// LUMA/CHROMA can work simultaneously with any other video output
// method since it works off of pixel index values coming straight
// out of the pixel sequencer.  This video output mode ignores
// custom RGB palette registers. Instead it uses luma, phase,
// amplitude for each of the 16 colors.
//

// WITH_64K
// --------
// Uncomment to use all block ram for video memory. This makes
// some dual port ram use distributed mem rather than block
// ram and increases LUT usage.  This will guarantee the core
// will NOT fit on an X9.  The most we can get on X9 is 32k.
`define WITH_64K 1

// TEST_PATTERN
// ------------
// This shows a test pattern with colors and some text.
// Useful for testing video output from the device without
// it being plugged into a C64. This will use approx 16k
// of block ram for the pixel data.
//`define TEST_PATTERN 1

// WITH_DVI
// --------
// Uncomment to include TMDS outputs and DVI encoder for video
// Enabling will automatically enable NEED_RGB.
`define WITH_DVI 1

// GEN_LUMA_CHROMA
// ---------------
// Uncomment if we shuold generate luma and chroma signals.
`define GEN_LUMA_CHROMA 1

// CONFIGURABLE_LUMAS
// ------------------
// Uncomment to activate registers 0xa0-0xcf and 0x80,0x81 to
// control luma(a#), phase(0xb#) and amplitudes(0xc#) for the 16
// colors as well as blanking level (0x80) and burst amplitude (0x81).
//`define CONFIGURABLE_LUMAS 1

// CONFIGURABLE_RGB
// ------------------
// Uncomment to activate registers 0x00-0x3f to control
// 18-bit RGB values for the 16 colors.  If RGB is not
// configurable, a single static palette is used.
//`define CONFIGURABLE_RGB 1

// CONFIGURABLE_TIMING
// -------------------
// Uncomment to activate registers 0xd0-0xef to control HDMI/VGA
// timings for all the supported resolutions.  This take up a lot
// space on the device.  Not intended for the release, only to
// be used as a tool to find correct timings.
//`define CONFIGURABLE_TIMING 1

// AVERAGE_LUMAS
// -------------
// Uncomment to average the luma values over 4 ticks of the
// dot4x clock. This smooths out transitions between levels.
//`define AVERAGE_LUMAS 1

// HAVE_EEPROM
// ----------------
// Uncomment if board has EEPROM for persistence
// via connected SPI bus.
//`define HAVE_EEPROM 1

// HAVE_FLASH
// Uncomment if board has FLASH directly connected
// to FPGA via SPI bus.
//`define HAVE_FLASH 1

// NEED_RGB
// --------
// Uncomment if RGB signals are needed internally for
// VGA, DVI or external composite encoder.  This enables
// hsync, vsync, active, red, green, blue but does not
// require them to leave the device via any pins.
`define NEED_RGB 1

// GEN_RGB
// -------
// Uncomment if RGB signals are leaving the device via
// pins (hsync,vsync,active,red,green,blue,clock). Setting
// GEN_RGB will automatically set NEED_RGB.
`define GEN_RGB 1

// HIRES_MODES
// -----------
// Uncomment to enable hires modes (including 80 column
// mode).  X resolution will be confined to 1X unless
// this is enabled.
`define HIRES_MODES

// HIRES_RESET
// ----------------------
// Uncomment to reset the VIC if it is in hires mode
// and the board detects the CPU reset line has gone
// low. For this to work, the reset pin must be connected
// to the CPU reset pin.  But since the reset pin is pulled
// high there should be no issue with this enabled even if
// there is no connection.
`define HIRES_RESET

`endif // config_vh_
