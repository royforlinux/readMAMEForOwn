/*****************************************************************************
 *
 * includes/wswan.h
 *
 ****************************************************************************/

#ifndef WSWAN_H_
#define WSWAN_H_

#define WSWAN_TYPE_MONO 0
#define WSWAN_TYPE_COLOR 1

#define WSWAN_X_PIXELS  (28*8)
#define WSWAN_Y_PIXELS  (18*8)

#define INTERNAL_EEPROM_SIZE    1024

#include "emu.h"
#include "cpu/v30mz/v30mz.h"
#include "audio/wswan_snd.h"
#include "machine/nvram.h"
#include "bus/wswan/slot.h"
#include "bus/wswan/rom.h"


struct SoundDMA
{
	UINT32  source;     /* Source address */
	UINT16  size;       /* Size */
	UINT8   enable;     /* Enabled */
};

struct VDP
{
	UINT8 layer_bg_enable;          /* Background layer on/off */
	UINT8 layer_fg_enable;          /* Foreground layer on/off */
	UINT8 sprites_enable;           /* Sprites on/off */
	UINT8 window_sprites_enable;        /* Sprite window on/off */
	UINT8 window_fg_mode;           /* 0:inside/outside, 1:??, 2:inside, 3:outside */
	UINT8 current_line;         /* Current scanline : 0-158 (159?) */
	UINT8 line_compare;         /* Line to trigger line interrupt on */
	UINT32 sprite_table_address;        /* Address of the sprite table */
	UINT8 sprite_table_buffer[512];
	UINT8 sprite_first;         /* First sprite to draw */
	UINT8 sprite_count;         /* Number of sprites to draw */
	UINT16 layer_bg_address;        /* Address of the background screen map */
	UINT16 layer_fg_address;        /* Address of the foreground screen map */
	UINT8 window_fg_left;           /* Left coordinate of foreground window */
	UINT8 window_fg_top;            /* Top coordinate of foreground window */
	UINT8 window_fg_right;          /* Right coordinate of foreground window */
	UINT8 window_fg_bottom;         /* Bottom coordinate of foreground window */
	UINT8 window_sprites_left;      /* Left coordinate of sprites window */
	UINT8 window_sprites_top;       /* Top coordinate of sprites window */
	UINT8 window_sprites_right;     /* Right coordinate of sprites window */
	UINT8 window_sprites_bottom;        /* Bottom coordinate of sprites window */
	UINT8 layer_bg_scroll_x;        /* Background layer X scroll */
	UINT8 layer_bg_scroll_y;        /* Background layer Y scroll */
	UINT8 layer_fg_scroll_x;        /* Foreground layer X scroll */
	UINT8 layer_fg_scroll_y;        /* Foreground layer Y scroll */
	UINT8 lcd_enable;           /* LCD on/off */
	UINT8 icons;                /* FIXME: What do we do with these? Maybe artwork? */
	UINT8 color_mode;           /* monochrome/color mode */
	UINT8 colors_16;            /* 4/16 colors mode */
	UINT8 tile_packed;          /* layered/packed tile mode switch */
	UINT8 timer_hblank_enable;      /* Horizontal blank interrupt on/off */
	UINT8 timer_hblank_mode;        /* Horizontal blank timer mode */
	UINT16 timer_hblank_reload;     /* Horizontal blank timer reload value */
	UINT16 timer_hblank_count;      /* Horizontal blank timer counter value */
	UINT8 timer_vblank_enable;      /* Vertical blank interrupt on/off */
	UINT8 timer_vblank_mode;        /* Vertical blank timer mode */
	UINT16 timer_vblank_reload;     /* Vertical blank timer reload value */
	UINT16 timer_vblank_count;      /* Vertical blank timer counter value */
	UINT8 *vram;                /* pointer to start of ram/vram (set by machine_reset) */
	UINT8 *palette_vram;            /* pointer to start of palette area in ram/vram (set by machine_reset), WSC only */
	int main_palette[8];
	emu_timer *timer;
};


class wswan_state : public driver_device
{
public:
	wswan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_cursx(*this, "CURSX"),
		m_cursy(*this, "CURSY"),
		m_buttons(*this, "BUTTONS") { }

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;
	DECLARE_READ8_MEMBER(bios_r);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);

	VDP m_vdp;
	UINT8 m_ws_portram[256];
	UINT8 m_internal_eeprom[INTERNAL_EEPROM_SIZE];
	UINT8 m_system_type;
	SoundDMA m_sound_dma;
	UINT8 *m_ws_ram;
	UINT8 *m_ws_bios_bank;
	UINT8 m_bios_disabled;
	int m_pal[16][16];
	bitmap_ind16 m_bitmap;
	UINT8 m_rotate;

	void wswan_clear_irq_line(int irq);
	void common_start();
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(wswan);
	DECLARE_MACHINE_START(wscolor);
	DECLARE_PALETTE_INIT(wscolor);
	TIMER_CALLBACK_MEMBER(wswan_scanline_interrupt);

protected:
	/* Interrupt flags */
	static const UINT8 WSWAN_IFLAG_STX    = 0x01;
	static const UINT8 WSWAN_IFLAG_KEY    = 0x02;
	static const UINT8 WSWAN_IFLAG_RTC    = 0x04;
	static const UINT8 WSWAN_IFLAG_SRX    = 0x08;
	static const UINT8 WSWAN_IFLAG_LCMP   = 0x10;
	static const UINT8 WSWAN_IFLAG_VBLTMR = 0x20;
	static const UINT8 WSWAN_IFLAG_VBL    = 0x40;
	static const UINT8 WSWAN_IFLAG_HBLTMR = 0x80;

	/* Interrupts */
	static const UINT8 WSWAN_INT_STX    = 0;
	static const UINT8 WSWAN_INT_KEY    = 1;
	static const UINT8 WSWAN_INT_RTC    = 2;
	static const UINT8 WSWAN_INT_SRX    = 3;
	static const UINT8 WSWAN_INT_LCMP   = 4;
	static const UINT8 WSWAN_INT_VBLTMR = 5;
	static const UINT8 WSWAN_INT_VBL    = 6;
	static const UINT8 WSWAN_INT_HBLTMR = 7;

	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;

	void wswan_register_save();
	void wswan_postload();
	void wswan_handle_irqs();
	void wswan_set_irq_line(int irq);
	void wswan_setup_palettes();
	void wswan_draw_background();
	void wswan_draw_foreground_0();
	void wswan_draw_foreground_2();
	void wswan_draw_foreground_3();
	void wswan_handle_sprites( int mask );
	void wswan_refresh_scanline( );
};


#endif /* WSWAN_H_ */
