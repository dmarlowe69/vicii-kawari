#include <stdio.h>
#include <6502.h>
#include <peekpoke.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "kawari.h"
#include "menu.h"

// SPI_REG - Used for both direct (slow) access to SPI devices
//           and bulk SPI operation for Kawari to execute.
//
// (Write)
// Bit 1 : FLASH Select
// Bit 2 : SPI Clock
// Bit 3 : SPI Data Out
// Bit 4 : EEPROM Select
// Bit 5 : unused
// Bit 6 : unused
// Bit 7 : unused
// Bit 8 : Write/Verify 16k block from 0x00000 video ram (*)
//
// (Read)
// Bit 1 - SPI Data In
// Bit 2 - N/A for eeprom
// Bit 3 - N/A for eeprom
// Bit 4 - SPI lock status
// Bit 5 - Extensions lock status
// Bit 6 - Persistence lock status
// Bit 7 - unused
// Bit 8 - unused

// SPI_REG
// All combinations of D/C/S bits for EEPROM.
// Make sure FLASH select (Bit 1) is always held high.
#define D1_C1_S1 15 // 1111
#define D1_C1_S0  7 // 0111
#define D1_C0_S1 13 // 1101
#define D1_C0_S0  5 // 0101
#define D0_C1_S1 11 // 1011
#define D0_C1_S0  3 // 0011
#define D0_C0_S1  9 // 1001
#define D0_C0_S0  1 // 0001

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SET(val) asm("lda #"STRINGIFY(val)); asm ("sta $d034");

#define DEVICE_ID_INSTR       0x83
#define READ_INSTR            0x03
#define WREN_INSTR            0x06
#define READ_STATUS1_INSTR    0x05
#define WRITE_INSTR           0x02
#define ID_READ               0x83  // a10 = 0
#define ID_WRITE              0x82  // a10 = 0
#define ID_LOCK_READ          0x83  // a10 = 1
#define ID_PERM_LOCK          0x82  // a10 = 1

#define TO_EXIT 0
#define TO_CONTINUE 1
#define TO_TRY_AGAIN 2
#define TO_NOTHING 3

unsigned char data_out[32];
unsigned char data_in[32];
unsigned char data_all[1024];

#define SCRATCH_SIZE 32
unsigned char scratch[SCRATCH_SIZE];

// THIS IS REV_4 INTI DATA
unsigned char compare[1024] = {
0x56,0x49,0x43,0x32,0x80,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0xff,0x3f,0x3f,0x3f,0xff,
0x2b,0x0a,0x0a,0xff,0x18,0x36,0x33,0xff,
0x2c,0x0f,0x2d,0xff,0x12,0x31,0x12,0xff,
0x0d,0x0e,0x31,0xff,0x39,0x3b,0x13,0xff,
0x2d,0x16,0x07,0xff,0x1a,0x0e,0x02,0xff,
0x3a,0x1d,0x1b,0xff,0x13,0x13,0x13,0xff,
0x21,0x21,0x21,0xff,0x29,0x3e,0x27,0xff,
0x1c,0x1f,0x39,0xff,0x2d,0x2d,0x2d,0xff,
0x0c,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x18,0x3f,0x2f,0x39,0x32,0x36,0x2c,0x3c,
0x32,0x2c,0x36,0x2f,0x35,0x3c,0x35,0x39,
0x00,0x00,0x50,0xd0,0x20,0xa0,0xf1,0x80,
0x60,0x70,0x50,0x00,0x00,0xa0,0xf1,0x00,
0x00,0x00,0x0d,0x0a,0x0c,0x0b,0x0b,0x0f,
0x0f,0x0b,0x0c,0x00,0x00,0x0d,0x0d,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0xff,0x3f,0x3f,0x3f,0xff,
0x2b,0x0a,0x0a,0xff,0x18,0x36,0x33,0xff,
0x2c,0x0f,0x2d,0xff,0x12,0x31,0x12,0xff,
0x0d,0x0e,0x31,0xff,0x39,0x3b,0x13,0xff,
0x2d,0x16,0x07,0xff,0x1a,0x0e,0x02,0xff,
0x3a,0x1d,0x1b,0xff,0x13,0x13,0x13,0xff,
0x21,0x21,0x21,0xff,0x29,0x3e,0x27,0xff,
0x1c,0x1f,0x39,0xff,0x2d,0x2d,0x2d,0xff,
0x0c,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x08,0x3f,0x2a,0x37,0x2d,0x33,0x25,0x3b,
0x2d,0x25,0x33,0x2a,0x32,0x3b,0x32,0x37,
0x00,0x00,0x50,0xd0,0x20,0xa0,0xf1,0x80,
0x60,0x70,0x50,0x00,0x00,0xa0,0xf1,0x00,
0x00,0x00,0x0d,0x0a,0x0c,0x0b,0x0b,0x0f,
0x0f,0x0b,0x0c,0x00,0x00,0x0d,0x0d,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0xff,0x3f,0x3f,0x3f,0xff,
0x2b,0x0a,0x0a,0xff,0x18,0x36,0x33,0xff,
0x2c,0x0f,0x2d,0xff,0x12,0x31,0x12,0xff,
0x0d,0x0e,0x31,0xff,0x39,0x3b,0x13,0xff,
0x2d,0x16,0x07,0xff,0x1a,0x0e,0x02,0xff,
0x3a,0x1d,0x1b,0xff,0x13,0x13,0x13,0xff,
0x21,0x21,0x21,0xff,0x29,0x3e,0x27,0xff,
0x1c,0x1f,0x39,0xff,0x2d,0x2d,0x2d,0xff,
0x0c,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x18,0x3f,0x2b,0x3b,0x35,0x35,0x2b,0x3b,
0x35,0x2b,0x35,0x2b,0x35,0x3b,0x35,0x3b,
0x00,0x00,0x50,0xd0,0x20,0xa0,0xf1,0x80,
0x60,0x70,0x50,0x00,0x00,0xa0,0xf1,0x00,
0x00,0x00,0x0d,0x0a,0x0c,0x0b,0x0b,0x0f,
0x0f,0x0b,0x0c,0x00,0x00,0x0d,0x0d,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0xff,0x3f,0x3f,0x3f,0xff,
0x2b,0x0a,0x0a,0xff,0x18,0x36,0x33,0xff,
0x2c,0x0f,0x2d,0xff,0x12,0x31,0x12,0xff,
0x0d,0x0e,0x31,0xff,0x39,0x3b,0x13,0xff,
0x2d,0x16,0x07,0xff,0x1a,0x0e,0x02,0xff,
0x3a,0x1d,0x1b,0xff,0x13,0x13,0x13,0xff,
0x21,0x21,0x21,0xff,0x29,0x3e,0x27,0xff,
0x1c,0x1f,0x39,0xff,0x2d,0x2d,0x2d,0xff,
0x0c,0x0c,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x08,0x3f,0x1f,0x39,0x30,0x30,0x1f,0x39,
0x30,0x1f,0x30,0x1f,0x30,0x39,0x30,0x39,
0x00,0x00,0x50,0xd0,0x20,0xa0,0xf1,0x80,
0x60,0x70,0x50,0x00,0x00,0xa0,0xf1,0x00,
0x00,0x00,0x0d,0x0a,0x0c,0x0b,0x0b,0x0f,
0x0f,0x0b,0x0c,0x00,0x00,0x0d,0x0d,0x00,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};

static struct regs r;

void press_any_key(int code) {
   printf ("Press any key");
   if (code == TO_CONTINUE) printf (" to continue.\n");
   if (code == TO_EXIT) printf (" to exit.\n");
   if (code == TO_TRY_AGAIN) printf (" try again.\n");
   if (code == TO_NOTHING) printf (".\n");
   WAITKEY;
}

// Helper to read 8 bits from SPI device
void read8() {
   unsigned char b;
   unsigned char value = 0;
   unsigned int bit;
   for (b=128;b>=1;b=b/2) {
        SET(D0_C0_S0);
        SET(D0_C1_S0);
        bit = PEEK(SPI_REG) & 1;
        if (bit)
	   value = value + b;
   }
   data_in[0] = value;
}

// Generic routine to talk to SPI device
// 8-bit instruction
// optional 16 bit address
// write_count num bytes to write (from data[] array)
// read_count num bytes to read (into data[] array)
void talk(unsigned char instr,
          unsigned char with_addr, unsigned long addr,
	  unsigned int read_count, unsigned int write_count,
	  unsigned char close)
{
    unsigned char value;
    unsigned long b;
    int n;
    unsigned long bit;

    asm("sei");
    SET(D1_C1_S1);
    SET(D0_C1_S0);

    // 8 bit instruction
    for (b=128L;b>=1L;b=b/2L) {
       bit = instr & b;
       if (bit) {
	       SET(D1_C0_S0);
               SET(D1_C1_S0);
       } else {
	       SET(D0_C0_S0);
               SET(D0_C1_S0);
       }
    }

    // Should we shift a 16 bit address now?
    if (with_addr) {
       // 16 bit address
       for (b=32768L;b>=1L;b=b/2L) {
          bit = addr & b;
          if (bit) {
	       SET(D1_C0_S0);
               SET(D1_C1_S0);
          } else {
	       SET(D0_C0_S0);
               SET(D0_C1_S0);
          }
       }
    }

    for (n=0;n<write_count;n++) {
       for (b=128L;b>=1L;b=b/2L) {
          bit = data_out[n] & b;
          if (bit) {
	       SET(D1_C0_S0);
               SET(D1_C1_S0);
          } else {
	       SET(D0_C0_S0);
               SET(D0_C1_S0);
          }
       }
    }

    // count is num bytes to read
    for (n=0;n<read_count;n++) {
       // 8 bit data
       value = 0;
       for (b=128L;b>=1L;b=b/2L) {
	   SET(D0_C0_S0);
	   SET(D0_C1_S0);
           bit = PEEK(SPI_REG) & 1;
           if (bit)
	      value = value + b;
       }
       data_in[n] = value;
    }

    if (close) {
       SET(D0_C1_S0); // leave clock high before S high
       SET(D1_C1_S1); // drive S high again
    }

    asm("cli");
}

// Wait for flash device to be ready
void wait_busy(void) {
    // INSTR + 1 BYTE READ + NO CLOSE
    talk(READ_STATUS1_INSTR,
	0, 0L /* no addr */,
	1 /* read 1 */,
	0 /* write 0 */,
	0 /* no close */);

    while (data_in[0] & 1) {
       read8();
    }

    // close
    SET(D0_C0_S0);
    SET(D1_C1_S1);
}

// Read the flash device id bytes
void read_device(void) {
    // INSTR + 16 bit 0 + 3 READ BYTES + CLOSE
    talk(DEVICE_ID_INSTR,
         1 /* withaddr */, 0,
	 3 /* read 3 */,
	 0 /* write 0 */,
	 1 /* close */);
}

void read_page(unsigned long addr) {
    // INSTR + 16 bit addr + 32 read bytes + CLOSE
    talk(READ_INSTR,
	1 /* withaddr */, addr,
	32 /* read 32 */,
	0 /* write 0 */,
	1 /* close */);
}

void write_page(unsigned long addr) {
    // INSTR + 16 bit addr + 32 write bytes + CLOSE
    talk(WRITE_INSTR,
	1 /* withaddr */, addr,
	0 /* read 0 */,
	32 /* write 32 */,
	1 /* close */);

    wait_busy();
}

// Write enable
void wren(void) {
    // INSTR + CLOSE
    talk(WREN_INSTR,
	0, 0L /* no addr */,
	0 /* read 0 */,
	0 /* write 0 */,
	1 /* close */);
}

unsigned long input_string(void) {
   unsigned n = 0;
   for (;;) {
      WAITKEY;
      printf("%c",r.a);
      if (r.a == 20 && n > 0) {
         scratch[n-1] = '\0';
         n--;
      }
      else if (r.a == 0x0d) {
         break;
      } else {
         scratch[n++] = r.a;
      }
      if (n > 30) break;
   }
   scratch[n] = '\0';
   return atol(scratch);
}

void erase_all(void)
{
    unsigned long addr;
    int c;
    for (c = 0; c < 32; c=c+1) { data_out[c] = 0xff; }

    printf ("Erase all EEPROM memory?\n");
    printf ("Q to quit. Any other key to continue\n");
    WAITKEY;
    if (r.a == 'q') return;

    for (addr = 0; addr < 1024; addr=addr+32) {
        printf ("Write @ %04x\n",(int)addr);
        wren();
        write_page(addr);
    }
}

void read_all(void)
{
    unsigned long addr;
    int c;

    for (addr = 0; addr < 1024; addr=addr+32) {
       read_page(addr);

       for (c=0;c<32;c++) {
           if (c % 8 == 0) {
               printf ("%04x: ", (int)(addr+c));
           }
           printf ("%02x ", data_in[c]);
           if (c ==7 || c==15 || c==23 || c==31) printf ("\n");
       }
       press_any_key(TO_CONTINUE);
       if (r.a == 'q') break;
    }
}

// This verifies we read the right value from eeprom into
// the register at boot for a given chip model.
void check(unsigned int bank, unsigned int reg) {
    unsigned char got;
    POKE(VIDEO_MEM_1_LO, reg);

    got = PEEK(VIDEO_MEM_1_VAL);
    if (data_all[256*bank+reg] != got) {
        printf ("REG READ %02x != %02x, got %02x bank %d\n",
                 reg, data_all[256*bank + reg], got, bank);
    }
}

// This first verifies things were read properly
// for the chip, then compares all eeprom stored
// values against known init values to make sure
// init worked properly.
void verify(void)
{
    unsigned int addr;
    int c;
    unsigned int bank;
    printf ("Checking...\n");

    // First copy actual EEPROM data into data_all.
    for (addr = 0; addr < 1024; addr=addr+32) {
       read_page(addr);

       for (c=0;c<32;c++) {
          data_all[addr+c] = data_in[c];
       }
    }

    // Now verify we booted properly with what
    // we should have read from EEPROM.

    // First check cross chip settings
    POKE(VIDEO_MEM_FLAGS, VMEM_FLAG_REGS_BIT);
    check(0, MAGIC_0);
    check(0, MAGIC_1);
    check(0, MAGIC_2);
    check(0, MAGIC_3);
    check(0, DISPLAY_FLAGS);

    // Now the bank we booted on
    POKE(VIDEO_MEM_1_LO, EEPROM_BANK);
    bank = PEEK(VIDEO_MEM_1_VAL);

    for (c=0;c<64;c++) {
        if (c % 4 == 3) continue;
        check(bank, RGB_START+c);
    }
    check(bank, BLACK_LEVEL);
    check(bank, BURST_AMPLITUDE);
    for (c=0;c<16;c++) {
        check(bank, LUMA_START+c);
        check(bank, PHASE_START+c);
        check(bank, AMPLITUDE_START+c);
    }

    // Now to what known values should
    // be after init.  These will only match
    // after an init was done. But is also useful
    // to verify only the settings that got
    // changed in one of the editors actually
    // got changed.
    for (addr=0;addr<1024;addr++) {
       if (addr == CHIP_MODEL || addr == EEPROM_BANK) continue;

       if (data_all[addr] != compare[addr]) {
            printf ("EEPROM %02x != EXPECTED INIT %02x @ %04x\n",
                 data_all[addr], compare[addr], addr);
       }
    }
}


void write_byte(void)
{
   unsigned long addr;
   unsigned char value;

   printf ("Location:");
   addr = input_string(); // no type checking on this
   printf ("\nValue:");
   value = input_string(); // no type checking on this
   printf ("\nWrite %d to %d? ", addr, value);
   WAITKEY;
   printf ("\n");
   if (r.a == 'y') {
      wren();
      data_out[0] = value;
      talk(WRITE_INSTR,
	1 /* withaddr */, addr,
	0 /* read 0 */,
	1 /* write 1 */,
	1 /* close */);
      wait_busy();
   } else {
      printf ("Abort\n");
   }
}

// Read the id page into data_in
void read_id_page(void) {
    // INSTR + 16 bit 0 + 32 READ BYTES + CLOSE
    talk(ID_READ,
         1 /* withaddr */, 0,
	 32 /* read 32 */,
	 0 /* write 0 */,
	 1 /* close */);
}

// Write 32 byte id page from data_out
void write_id_page(void) {
    // INSTR + 16 bit 0 + 32 WRITE BYTES + CLOSE
    talk(ID_WRITE,
         1 /* withaddr */, 0,
	 0 /* read 0 */,
	 32 /* write 32 */,
	 1 /* close */);
}

// Return 1 if locked, 0 otherwise
unsigned char read_id_lock(void) {
    // INSTR + 16 bit 0 + 32 READ BYTES + CLOSE
    talk(ID_LOCK_READ,
         1 /* withaddr */, 1024, // a10=1
	 1 /* read 1 */,
	 0 /* write 0 */,
	 1 /* close */);
    return data_in[0] & 1;
}

// Locks the id page permenantly
void lock_id(void) {
    // INSTR + 16 bit 0 + 1 WRITE BYTES + CLOSE
    data_out[0] = 2;
    talk(ID_PERM_LOCK,
         1 /* withaddr */, 1024, // a10=1
	 0 /* read 1 */,
	 1 /* write 1 */,
	 1 /* close */);
}

void show_id_page(void) {
   int c;
   read_id_page();
   for (c=0;c<32;c++) {
      if (c % 8 == 0) {
         printf ("\n%04x: ", (int)(c));
      }
      printf ("%02x ", data_in[c]);
   }
   printf ("\n");
}

// Show null terminated string staring at 3
void show_serial(void) {
   int c;
   printf ("Serial:");
   read_id_page();
   for (c=3;c<32;c++) {
       if (data_in[c] == 0) break;
       printf ("%c", data_in[c]);
   }
   printf ("\n");
}

void set_serial(void) {
   int c;
   int pad;
   read_id_page();
   // copy data_in to data_out
   for (c=0;c<32;c++) { data_out[c] = data_in[c]; }
   
   printf ("Enter serial:");
   input_string();
   for (c=3;c<3+strlen(scratch);c++) {
      data_out[c] = scratch[c-3];
   }
   for (pad=c;pad<32;pad++) {
      data_out[pad] = 0;
   }
   printf ("Okay to set serial: %s\n");

   input_string();
   
   if (strlen(scratch) == 3 && strcmp("yes", scratch) == 0) {
      wren();
      write_id_page();
      printf ("\nWritten.\n");
   } else {
      printf ("\nAborted.\n");
   }
}

void lock_serial(void) {
  printf ("WARNING: This is permanent\n");
  printf ("Proceed to lock ID page? (type yes):");
  input_string();
  printf ("\n");
  if (strlen(scratch) == 3 && strcmp("yes", scratch) == 0) {
     wren();
     lock_id();
     printf ("Done\n");
  } else {
     printf ("Aborted\n");
  }
}

void get_lock_status(void) {
  printf ("Lock status:");
  if (read_id_lock()) printf ("LOCKED\n"); else printf ("UNLOCKED\n");
}

void main_menu(void)
{
    clrscr();

    printf ("VIC-II Kawari EEPROM Test Util\n\n");

    // Activate SPI reg.
    POKE(SPI_REG, 83);
    POKE(SPI_REG, 80);
    POKE(SPI_REG, 73);

    read_device();

    printf ("MF=%02x SPI_FAM=%02x DENSITY=%02x\n\n",
        data_in[0], data_in[1], data_in[2]);

    for (;;) {
       printf ("Command:");
       WAITKEY;
       printf ("%c\n",r.a);
       if (r.a == 'q') break;
       else if (r.a == 'r') { read_all(); }
       else if (r.a == 'e') { erase_all(); }
       else if (r.a == 'v') { verify(); }
       else if (r.a == 'w') { write_byte(); }
       else if (r.a == 'i') { show_id_page(); }
       else if (r.a == 'g') { show_serial(); }
       else if (r.a == 's') { set_serial(); }
       else if (r.a == 'l') { lock_serial(); }
       else if (r.a == 'o') { get_lock_status(); }
       else if (r.a == '?') {
           printf ("q quit\n");
           printf ("r read all pages\n");
           printf ("w write a byte\n");
           printf ("e erase all pages\n");
           printf ("v verify init\n");
           printf ("i read id page\n");
           printf ("g get serial\n");
           printf ("s set serial\n");
           printf ("l lock serial\n");
           printf ("o lock status\n");
       }
    }
}
