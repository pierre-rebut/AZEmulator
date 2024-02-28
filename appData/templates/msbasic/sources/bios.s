.setcpu "65C02"
.segment "BIOS_DATA"

_y:
	.byte $00
_x:
	.byte $00
_vpos:
	.word $0000

_vmul:
	.word $0000

_vshift:
    .byte $00

_blink:
    .byte $00

_timer:
    .byte $00

.segment "BIOS_IO"

_kbr:
	.res	16,$00

_video:
	.res	4000,$00

.segment "BIOS_CODE"

LOAD:
SAVE:
    rts

MONRDKEY:
CHRIN:
	lda _kbr
	beq @chrin_cursor
	jsr CHROUT
@chrin_cursor:
    inc _timer
    bne @chrin_end
    pha
    lda _blink
    beq @chrin_cursor_show
    lda #00
    dec _blink
    jmp @chrin_cursor_draw
@chrin_cursor_show:
    inc _blink
    lda #219
@chrin_cursor_draw:
    jsr PUTCHAR
    pla
@chrin_end:
	rts

MONCOUT:
CHROUT:
	pha

	cmp #$20
	beq @chrout_space

	cmp #$0A
	beq @chrout_newline

	cmp #$08
	beq @chrout_backspace

	cmp #$21
	bmi @chrout_end

	cmp #$80
	bpl @chrout_end

	jmp @chrout_putchar
	
@chrout_space:
    lda #00
    jsr PUTCHAR
	jsr @space
	jmp @chrout_end

@chrout_newline:
    lda #00
    jsr PUTCHAR
	jsr @newline
	jmp @chrout_end

@chrout_backspace:
	lda _x
	bne @chrout_backspace_end
	lda #$50
	sta _x
	lda _y
	bne @chrout_backspace_update
	lda #$19
	sta _y
@chrout_backspace_update:
	dec _y
	jsr @newline_update_vpos
@chrout_backspace_end:
	dec _x
	lda #$00
	jsr PUTCHAR
	jmp @chrout_end
	
@chrout_putchar:	
	jsr PUTCHAR
	jsr @space
	
@chrout_end:
	pla
	rts

@space:
	inc _x
	lda _x
	clc
	cmp #$50
	bne @space_end
	jsr @newline

@space_end:
	rts


@newline:
	stz _x
	inc _y
	lda _vshift
	beq @newline_noshift
	inc _vshift

@newline_noshift:
	lda _y
	clc
	cmp #$19
	bne @newline_update_vpos
	stz _y
	lda _vshift
	bne @newline_update_vpos
	inc _vshift

@newline_update_vpos:
	phy
	phx

	ldx #$A0
	lda _y
	jsr MUL8

	clc
	adc #<(_video)
	tay
	txa
	adc #>(_video)
	tax
	tya
	sta _vpos
	stx _vpos+1

    jsr @clear_line

    plx
	ply
	rts


@clear_line:
    lda #$50

@clear_line_loop:
    tax
    lda #$00
    jsr PUTCHAR
    inc _x
    txa
    dec
    bne @clear_line_loop
    stz _x
    rts

PUTCHAR:
	phy

	pha
	lda _x
	clc
	rol a
	tay
	pla

	sta (<_vpos),y
	lda #$0F
	iny
	sta (<_vpos),y

	ply
	rts

MUL8:
	stz _vmul
	stz _vmul+1

	cmp #$00
	jmp @mul8_end

@mul8_loop:
	pha
	
	txa
	clc
	adc _vmul
	sta _vmul
	lda #$00
	adc _vmul+1
	sta _vmul+1
	
	pla
	dec a

@mul8_end:
	bne @mul8_loop

	lda _vmul
	ldx _vmul+1
	rts

BIOS_RESET:
	stz _y
	stz _x

	lda #<(_video)
	ldx #>(_video)
	sta _vpos
	stx _vpos+1

    jmp COLD_START

.segment "RESETVEC"
	.word $0F00
	.word BIOS_RESET
	.word $0000
