#include <stdio.h>
#include <6502.h>
#include <peekpoke.h>

#include "util.h"
#include "kawari.h"
#include "menu.h"
#include "init.h"

static struct regs r;
static int next_model=0;
static int current_model=0;

static int next_display_flags=0;
static int current_display_flags=0;

static int version;
static char variant[16];

static int line = 0;

void get_chip_model(void)
{
   POKE(VIDEO_MEM_1_LO,CHIP_MODEL);
   current_model = PEEK(VIDEO_MEM_1_VAL);
   next_model = current_model;
}

void get_display_flags(void)
{
   POKE(VIDEO_MEM_1_LO,DISPLAY_FLAGS);
   current_display_flags = PEEK(VIDEO_MEM_1_VAL) & 1;
   next_display_flags = current_display_flags;
}

void get_version(void)
{
   POKE(VIDEO_MEM_1_LO,VERSION);
   version = PEEK(VIDEO_MEM_1_VAL);
}

void get_variant(void)
{
   int t=0;
   char v;
   while (t < 16) {
      POKE(VIDEO_MEM_1_LO,VARIANT+t);
      v = PEEK(VIDEO_MEM_1_VAL);
      if (v == 0) break;
      variant[t++] = v;
   }
   variant[t] = 0;
}

void show_chip_model()
{
    int flip;
    TOXY(17,6);
    if  (line == 0) printf ("%c",18);
    switch (next_model) {
        case 0:
            printf ("6567R8  ");   
            break;
        case 1:
            printf ("6569R5  ");   
            break;
        case 2:
            printf ("6567R56A");   
            break;
        default:
            printf ("6569R1  ");   
            break;
    }

    if (current_model != next_model) {
       printf (" (changed)");
    } else {
       printf ("          ");
    }
    POKE(53305L,DISPLAY_FLAGS);
    flip=PEEK(53307L) & DISPLAY_CHIP_INVERT_SWITCH;

    if (flip)
	    printf ("INV");
    else
	    printf ("   ");

    if  (line == 0) printf ("%c",146);
}

void show_display_bit(int bit, int y, int label)
{
    int next_val = next_display_flags & bit;
    int current_val = current_display_flags & bit;

    // Always show actual value of switch
    if (bit == DISPLAY_CHIP_INVERT_SWITCH)
        next_val = current_display_flags & bit;

    TOXY(17,y);
    if  (line == y-6) printf ("%c",18);
    if (next_val) {
       if (label) printf ("HI "); else printf ("ON ");
    } else {
       if (label) printf ("LO "); else printf ("OFF ");
    }

    if (current_val != next_val) {
       printf (" (changed)");
    } else {
       printf ("          ");
    }
    if  (line == y-6) printf ("%c",146);
}

void show_info_line(void) {
    TOXY(0,21);
    printf ("----------------------------------------");
    if (line == 0) {
        printf ("A change to this setting will take      ");
        printf ("effect on the next cold boot.           ");
    }
    else if (line == 1) {
        printf ("Simulates raster lines for RGB based    ");
        printf ("video. Has no effect on composite video ");
    }
    else if (line == 2) {
        printf ("Change vertical refresh to 15khz. NOTE  ");
        printf ("your monitor must accept this frequency.");
    }
    else if (line == 3) {
        printf ("Use native horizontal resolution. NOTE  ");
        printf ("hires modes will not work if set.       ");
    }
    else if (line == 4) {
        printf ("Ouput CSYNC on the HSYNC analog RGB pin.");
        printf ("NOTE: Your monitor must support this.   ");
    }
    else if (line == 5) {
        printf ("Set polarity on H pin of analog RGB     ");
        printf ("header. Active LO or Active HI.         ");
    }
    else if (line == 6) {
        printf ("Set polarity on V pin of analog RGB     ");
        printf ("header. Active LO or Active HI.         ");
    }
    else if (line == 7) {
        printf ("CHIP toggle switch. If ON, CHIP will be ");
        printf ("opposite video standard of saved value. ");
    }
}

void save_changes(void)
{
   POKE(VIDEO_MEM_FLAGS, PEEK(VIDEO_MEM_FLAGS) | VMEM_FLAG_PERSIST_BIT);
   if (current_display_flags != next_display_flags) {
      POKE(VIDEO_MEM_1_LO, DISPLAY_FLAGS);
      SAFE_POKE(VIDEO_MEM_1_VAL, next_display_flags);
      current_display_flags = next_display_flags;
   }
   if (current_model != next_model) {
      POKE(VIDEO_MEM_1_LO, CHIP_MODEL);
      SAFE_POKE(VIDEO_MEM_1_VAL, next_model);
      current_model = next_model;
   }
   POKE(VIDEO_MEM_FLAGS, PEEK(VIDEO_MEM_FLAGS) & ~VMEM_FLAG_PERSIST_BIT);
}

void main_menu(void)
{
    int need_refresh = 0;
    unsigned char sw;

    POKE(VIDEO_MEM_FLAGS, VMEM_FLAG_REGS_BIT);
    get_version();
    get_variant();

    CLRSCRN;
    printf ("VIC-II Kawari Config Utility\n\n");
 
    printf ("Utility Version: %s\n",UTILITY_VERSION);
    printf ("Kawari Version : %d.%d\n",version >> 4, version & 15);
    printf ("Variant        : %s\n",variant);
    printf ("\n");
    printf ("Chip Model     :\n");
    printf ("Raster Lines   :\n");
    printf ("RGB 15khz      :\n");
    printf ("DVI/RGB 1xWidth:\n");
    printf ("RGB CSYNC      :\n");
    printf ("VSync polarity :\n");
    printf ("HSync polarity :\n");
    printf ("External Switch::\n");
    printf ("\n");

    get_chip_model();
    get_display_flags();

    printf ("CRSR to navigate | SPACE to change\n");
    printf ("S to save        | D for defaults\n");
    printf ("Q to quit\n");

    need_refresh = 1;
    for (;;) {
       if (need_refresh) {
          show_chip_model();
          show_display_bit(DISPLAY_SHOW_RASTER_LINES_BIT, 7, 0);
          show_display_bit(DISPLAY_IS_NATIVE_Y_BIT, 8, 0);
          show_display_bit(DISPLAY_IS_NATIVE_X_BIT, 9, 0);
          show_display_bit(DISPLAY_ENABLE_CSYNC_BIT, 10, 0);
          show_display_bit(DISPLAY_VPOLARITY_BIT, 11, 1);
          show_display_bit(DISPLAY_HPOLARITY_BIT, 12, 1);
          show_display_bit(DISPLAY_CHIP_INVERT_SWITCH, 13, 0);
	  show_info_line();
          need_refresh = 0;
       }

       r.a = wait_key_or_switch();

       if (r.a == 'q') {
          CLRSCRN;
          return;
       } else if (r.a == ' ') {
          if (line == 0) {
             next_model=next_model+1;
             if (next_model > 3) next_model=0;
             show_chip_model();
	  }
          else if (line == 1) {
             next_display_flags ^= DISPLAY_SHOW_RASTER_LINES_BIT;
             show_display_bit(DISPLAY_SHOW_RASTER_LINES_BIT, 7, 0);
	  }
          else if (line == 2) {
             next_display_flags ^= DISPLAY_IS_NATIVE_Y_BIT;
             show_display_bit(DISPLAY_IS_NATIVE_Y_BIT, 8, 0);
	  }
          else if (line == 3) {
             next_display_flags ^= DISPLAY_IS_NATIVE_X_BIT;
             show_display_bit(DISPLAY_IS_NATIVE_X_BIT, 9, 0);
	  }
          else if (line == 4) {
             next_display_flags ^= DISPLAY_ENABLE_CSYNC_BIT;
             show_display_bit(DISPLAY_ENABLE_CSYNC_BIT, 10, 0);
	  }
          else if (line == 5) {
             next_display_flags ^= DISPLAY_VPOLARITY_BIT;
             show_display_bit(DISPLAY_VPOLARITY_BIT, 11, 1);
	  }
          else if (line == 6) {
             next_display_flags ^= DISPLAY_HPOLARITY_BIT;
             show_display_bit(DISPLAY_HPOLARITY_BIT, 12, 1);
	  }
       } else if (r.a == 's') {
          save_changes();
          need_refresh=1;
       } else if (r.a == CRSR_DOWN) {
          line++; if (line > 7) line = 7;
          need_refresh=1;
       } else if (r.a == CRSR_UP) {
          line--; if (line < 0) line = 0;
          need_refresh=1;
       } else if (r.a == 'd') {
          next_display_flags = DEFAULT_DISPLAY_FLAGS;
          need_refresh=1;
       } else if (r.a == '*') {
          // external switch has changed
          get_display_flags();
          need_refresh=1;
       }
   }
}
