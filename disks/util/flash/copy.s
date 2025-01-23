KAWARI_VMODE1 = $d037
KAWARI_VMODE2 = $d038
KAWARI_PORT = $d03f
VMEM_A_IDX = $d035
VMEM_A_HI = $d03a
VMEM_A_LO = $d039
VMEM_A_VAL = $d03b
VMEM_B_IDX = $d036
VMEM_B_HI = $d03d
VMEM_B_LO = $d03c
VMEM_B_VAL = $d03e

_copy_6000_0000:
        ; put first arg into pg_size
        sta pg_size+1
        sei         ; disable interrupts

        lda $fc
        sta savefc
        lda $fb
        sta savefb

        lda #$60    ; load high byte of $6000
        sta $fc     ; store it in a free location we use as vector
        ldy #$00    ; init counter with 0
        sty $fb     ; store it as low byte in the $FB/$FC vector

        lda #$00    ; we're going to copy it to video ram $0000
        sta VMEM_A_IDX
        sta VMEM_A_HI
        sta VMEM_A_LO
        lda #1      ; use auto increment
        sta KAWARI_PORT

        ; page size is overwritten by incoming arg in accum
pg_size:
        ldx #$40    ; we loop this many times (64 times = 16Kb, 16 times = 4k)
loop:
        lda ($fb),y ; read byte from src $fb/$fc
        sta VMEM_A_VAL   ; write byte to dest video ram
        iny         ; do this 256 times...
        bne loop    ; ..for low byte $00 to $FF
        inc $fc     ; when we passed $FF increase high byte...
        dex         ; ... and decrease X by one before restart
        bne loop    ; We repeat this until X becomes Zero

        cli         ; turn off interrupt disable flag

        lda savefb
        sta $fb
        lda savefc
        sta $fc

        rts

savefc:
.BYTE   0
savefb:
.BYTE   0

.export _copy_6000_0000
