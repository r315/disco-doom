#include "doomstat.h"
#include "d_event.h"
#include "z_zone.h"
#include "st_classic.h"
#include "v_video.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_main.h"
#include "g_game.h"

#define FG 0
#define BG 4

typedef enum st_state_e {
	ST_HIDDEN = 0,
	ST_REDRAW,
	ST_VISIBLE
}st_state_t;

typedef struct st_widget_s {
	short x, y;
	patch_t *bg;
	int last;
	void(*draw)(struct st_widget_s*, byte);
}st_widget_t;

typedef struct st_wi_icon_s {
	st_widget_t base;
	byte count;
	byte frame;
	patch_t **frames;
}st_wi_icon_t;

typedef struct st_wi_multi_s {
	st_widget_t base;
	byte count;
	st_widget_t *sub;
}st_wi_multi_t;

typedef struct st_wi_num_s {
	st_widget_t base;
	patch_t **font;
	byte w, h;
	int value;
	byte digits;
}st_wi_num_t;


// lump number for PLAYPAL
static int lu_palette;
// 0-9, tall numbers
static patch_t *numtall[12];
// 0-9, short, yellow (,different!) numbers
static patch_t *y_numshort[10];
static patch_t *g_numshort[10];
// main player in game
static player_t *plyr;

//
// Status bar widgets
// | Ready weapon | health  | arms | Face |  Armor  |  Key  |  Ammo  |
// |    ammo      | percent |      |      | percent | boxes | Counts |
static st_wi_multi_t w_backgound;
static st_wi_num_t   w_ready;  // Current ready weapon ammo widget
static st_wi_num_t   w_health; // health widget
static st_wi_multi_t w_arms;   // arms owned widget
static st_wi_icon_t  w_face;   // face widget
static st_wi_num_t   w_armor;  // armor widget
static st_wi_multi_t w_keys;   // keys widget
static st_wi_multi_t w_ammo;   // ammo widget

// 
static st_state_t st_state;
static st_state_t st_last_state;	// Used when exiting automap

static int st_palette;
static int st_randomnumber;
static int st_oldhealth;
static int st_lastattackdown = -1;
static byte st_oldweaponsowned[NUMWEAPONS];
static int st_priority;
//
//
//
static void ST_drawBackground(st_widget_t *sbar, byte refresh)
{
	st_wi_multi_t *patches = (st_wi_multi_t*)sbar;
	// status bar backgound
	V_DrawPatch(sbar->x, sbar->y, BG, sbar->bg);
	for (int i = 0; i < patches->count; i++) {
		// copy patch to ST buffer
		short y = patches->sub[i].y;
		if (y > ST_HEIGHT) {
			y -= ST_Y;
		}
		V_DrawPatch(patches->sub[i].x, y, BG, patches->sub[i].bg);
	}
	// copy ST background to foreground video buffer
	V_CopyRect(ST_X, 0, BG, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y, FG);
}

static void ST_drawNumber(st_widget_t *wi, byte refresh)
{
	st_wi_num_t *num = (st_wi_num_t*)wi;
	// Draw only when changed
	if (wi->last == num->value && !refresh) {
		return;
	}

	int val = num->value;
	int numdigits = num->digits;
	int x = wi->x;
	int y = wi->y;
	int digit_w = num->font[0]->width;
	int digit_h = num->font[0]->height;

	V_CopyRect(x - ST_X, y - ST_Y, BG, numdigits * digit_w, digit_h, x, y, FG);

	if (val < 0)
		val = -val;
	
	// if non-number
	if (val != 1994) 
	{		
		// Start from right to left
		x += (numdigits - 1) * digit_w;

		do {
			V_DrawPatch(x, y, FG, num->font[val%10]);
			numdigits--;
			x -= digit_w;
			val /= 10;
		} while (val);	
	
		// If number is can be negative, caller must ensure 
		// enought digits
		if (val < 0 && numdigits)
		{
			V_DrawPatch(x, y, FG, num->font[11]); // minus sign			
		}
	}

	wi->last = num->value;
}
//
//
//
static void ST_drawFace(st_widget_t *wi, byte refresh)
{
		patch_t *p = w_face.frames[w_face.base.last];

	int x = w_face.base.x - p->leftoffset;
	int y = w_face.base.y - p->topoffset;
	int w = p->width;
	int h = p->height;

	V_CopyRect(x, y - ST_Y, BG, w, h, x, y, FG);
	p = w_face.frames[w_face.frame];
	V_DrawPatch(w_face.base.x, w_face.base.y, FG, p);
	w_face.base.last = w_face.frame;

}
//
//
//
static void ST_drawKey(st_widget_t *wi, byte refresh)
{
	if ((refresh && wi->last & ST_KEYFLAGON) || wi->last == ST_KEYFLAGDRAW)
	{	
		V_CopyRect(wi->x - ST_X, wi->y - ST_Y, BG, wi->bg->width, wi->bg->height, wi->x, wi->y, FG);
		V_DrawPatch(wi->x, wi->y, FG, wi->bg);
		wi->last = ST_KEYFLAGON;
	}
}
//
//
//
static void ST_drawWidgets(byte refresh)
{
	st_wi_num_t *aux_num;
	st_widget_t *aux_wi;	
	int i;

	ST_drawNumber((st_widget_t*)&w_ready, refresh);
	ST_drawNumber((st_widget_t*)&w_health, refresh);
	ST_drawNumber((st_widget_t*)&w_armor, refresh);

	aux_num = (st_wi_num_t *)w_arms.sub;
	for (i = 0; i < w_arms.count; i++, aux_num++)
	{
		ST_drawNumber((st_widget_t*)aux_num, refresh);
	}
	
	aux_wi = (st_widget_t*)w_keys.sub;
	for (i = 0; i < w_keys.count; i++, aux_wi++)
	{
		ST_drawKey(aux_wi, refresh);
	}

	aux_num = (st_wi_num_t *)w_ammo.sub;
	for (i = 0; i < w_ammo.count; i++, aux_num++)
	{
		ST_drawNumber((st_widget_t*)aux_num, refresh);
		ST_drawNumber((st_widget_t*)(aux_num + ST_AMMO_SLOTS), refresh);
	}

	ST_drawFace((st_widget_t*)&w_face, refresh);
}

//
//
//
static int ST_calcPainOffset(void)
{
	int health;
	static int lastcalc;
	static int oldhealth = -1;

	health = plyr->health > 100 ? 100 : plyr->health;

	if (health != oldhealth)
	{
		lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
		oldhealth = health;
	}
	return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
static void ST_updateFaceWidget(void)
{
	angle_t badguyangle;
	angle_t diffang;
	byte doevilgrin;
	int i;

	if (st_priority < 10)
	{
		// dead
		if (!plyr->health)
		{
			st_priority = 9;
			w_face.frame = ST_DEADFACE;
			w_face.count = 1;
		}
	}

	if (st_priority < 9)
	{
		if (plyr->bonuscount)
		{
			// picking up bonus
			doevilgrin = false;

			for (i = 0; i < NUMWEAPONS; i++)
			{
				if (st_oldweaponsowned[i] != plyr->weaponowned[i])
				{
					doevilgrin = true;
					st_oldweaponsowned[i] = plyr->weaponowned[i];
				}
			}
			if (doevilgrin)
			{
				// evil grin if just picked up weapon
				st_priority = 8;
				w_face.frame = ST_calcPainOffset() + ST_EVILGRINOFFSET;
				w_face.count = ST_EVILGRINCOUNT;
			}
		}
	}

	if (st_priority < 8)
	{
		if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
		{
			// being attacked
			st_priority = 7;

			if (st_oldhealth - plyr->health > ST_MUCHPAIN)
			{
				w_face.frame = ST_calcPainOffset() + ST_OUCHOFFSET;
				w_face.count = ST_TURNCOUNT;
			}
			else
			{
				badguyangle = R_PointToAngle2(plyr->mo->x,
					plyr->mo->y,
					plyr->attacker->x,
					plyr->attacker->y);

				if (badguyangle > plyr->mo->angle)
				{
					// whether right or left
					diffang = badguyangle - plyr->mo->angle;
					i = diffang > ANG180;
				}
				else
				{
					// whether left or right
					diffang = plyr->mo->angle - badguyangle;
					i = diffang <= ANG180;
				} // confusing, aint it?

				w_face.frame = ST_calcPainOffset();
				w_face.count = ST_TURNCOUNT;

				if (diffang < ANG45)
				{
					// head-on
					w_face.frame += ST_RAMPAGEOFFSET;
				}
				else if (i)
				{
					// turn face right
					w_face.frame += ST_TURNOFFSET;
				}
				else
				{
					// turn face left
					w_face.frame += ST_TURNOFFSET + 1;
				}
			}
		}
	}

	if (st_priority < 7)
	{
		// getting hurt because of your own damn stupidity
		if (plyr->damagecount)
		{
			if (plyr->health - st_oldhealth > ST_MUCHPAIN)
			{
				st_priority = 7;
				w_face.count = ST_TURNCOUNT;
				w_face.frame = ST_calcPainOffset() + ST_OUCHOFFSET;
			}
			else
			{
				st_priority = 6;
				w_face.count = ST_TURNCOUNT;
				w_face.frame = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
			}
		}
	}

	if (st_priority < 6)
	{
		// rapid firing
		if (plyr->attackdown)
		{
			if (st_lastattackdown == -1)
				st_lastattackdown = ST_RAMPAGEDELAY;
			else if (!--st_lastattackdown)
			{
				st_priority = 5;
				w_face.frame = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
				w_face.count = 1;
				st_lastattackdown = 1;
			}
		}
		else
			st_lastattackdown = -1;
	}

	if (st_priority < 5)
	{
		// invulnerability
		if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
		{
			st_priority = 4;

			w_face.frame = ST_GODFACE;
			w_face.count = 1;
		}
	}

	// look left or look right if the facecount has timed out
	if (!w_face.count)
	{
		w_face.frame = ST_calcPainOffset() + (st_randomnumber % 3);
		w_face.count = ST_STRAIGHTFACECOUNT;
		st_priority = 0;
	}

	w_face.count--;
}
//
// ST_updateWidgets
//
static void ST_updateWidgets(void)
{
	int i;
	st_wi_num_t *aux_num;
	st_widget_t *aux_wi;
	

	if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
		w_ready.value = 1994; // means "n/a"
	else
		w_ready.value = plyr->ammo[weaponinfo[plyr->readyweapon].ammo];

	w_health.value = plyr->health;
	w_armor.value = plyr->armorpoints;

	aux_num = (st_wi_num_t*)w_arms.sub;
	for (i = 0; i < w_arms.count; i++, aux_num++) {
		if (plyr->weaponowned[i + 1] > 0) {			// skip malee weapon
			aux_num->font = y_numshort;
			if (aux_num->value < 0) aux_num->value = -aux_num->value;
		}
		else {
			aux_num->font = g_numshort;
			if (aux_num->value > 0) aux_num->value = -aux_num->value;
		}

	}

	aux_wi = (st_widget_t*)w_keys.sub;
	for (i = 0; i < w_keys.count; i++, aux_wi++)
	{
		if (plyr->cards[i]) {
			aux_wi->last |= ST_KEYFLAGDRAW;
		}
		else {
			aux_wi->last = ST_KEYFLAGSOFF;
		}
	}

	// Oh well order is reversed between index 2 and 3 
	((st_wi_num_t*)w_ammo.sub)[0].value = plyr->ammo[0];
	((st_wi_num_t*)w_ammo.sub)[1].value = plyr->ammo[1];
	((st_wi_num_t*)w_ammo.sub)[2].value = plyr->ammo[3];
	((st_wi_num_t*)w_ammo.sub)[3].value = plyr->ammo[2];
	((st_wi_num_t*)w_ammo.sub)[4].value = plyr->maxammo[0];
	((st_wi_num_t*)w_ammo.sub)[5].value = plyr->maxammo[1];
	((st_wi_num_t*)w_ammo.sub)[6].value = plyr->maxammo[3];
	((st_wi_num_t*)w_ammo.sub)[7].value = plyr->maxammo[2];

	// refresh everything if this is him coming back to life
	ST_updateFaceWidget();
}


//
// ST_loadGraphics
//
static void ST_loadGraphics(void)
{
	int i;
	int j;
	char namebuf[9];
	st_wi_num_t *aux_num;
	st_widget_t *aux_wi;

	// status bar background bits
	w_backgound.base.x = 0;
	w_backgound.base.y = 0;
	w_backgound.base.bg = (patch_t *)W_CacheLumpName("STBAR", PU_STATIC);
	w_backgound.base.draw = ST_drawBackground;
	w_backgound.count = ST_BG_COUNT;
	w_backgound.sub = (st_widget_t*)Z_Malloc(w_backgound.count * sizeof(st_widget_t), PU_STATIC, NULL);
	// load health '%' sign, as it never change, load it with backgound
	w_backgound.sub[0].bg = (patch_t *)W_CacheLumpName("STTPRCNT", PU_STATIC);
	w_backgound.sub[0].x = ST_HEALTHX + (ST_HEALTHWIDTH * w_backgound.sub[0].bg->width);
	w_backgound.sub[0].y = ST_HEALTHY;
	// same for armor
	w_backgound.sub[1].bg = w_backgound.sub[0].bg;
	w_backgound.sub[1].x = ST_ARMORX + (ST_ARMORWIDTH * w_backgound.sub[0].bg->width);
	w_backgound.sub[1].y = ST_ARMORY;
	// arms bg
	w_backgound.sub[2].bg = (patch_t *)W_CacheLumpName("STARMS", PU_STATIC);
	w_backgound.sub[2].x = ST_ARMSBGX;
	w_backgound.sub[2].y = ST_ARMSBGY;

	// Load the numbers, tall and short
	for (i = 0; i < 10; i++)
	{
		sprintf(namebuf, "STTNUM%d", i);
		numtall[i] = (patch_t *)W_CacheLumpName(namebuf, PU_STATIC);

		sprintf(namebuf, "STYSNUM%d", i);
		y_numshort[i] = (patch_t *)W_CacheLumpName(namebuf, PU_STATIC);

		sprintf(namebuf, "STGNUM%d", i);
		g_numshort[i] = (patch_t *)W_CacheLumpName(namebuf, PU_STATIC);
	}
	// Load minus sign.
	numtall[11] = (patch_t *)W_CacheLumpName("STTMINUS", PU_STATIC);

	// Create ammo widget
	w_ready.base.x = ST_AMMOX;
	w_ready.base.y = ST_AMMOY;
	w_ready.base.last = -1;
	w_ready.base.draw = ST_drawNumber;
	w_ready.digits = ST_AMMOWIDTH;
	w_ready.font = numtall;
	// Create health widget
	w_health.base.x = ST_HEALTHX;
	w_health.base.y = ST_HEALTHY;
	w_health.base.last = -1;
	w_health.base.draw = ST_drawNumber;
	w_health.digits = ST_HEALTHWIDTH;
	w_health.font = numtall;	
	// Create armor widget
	w_armor.base.x = ST_ARMORX;
	w_armor.base.y = ST_ARMORY;
	w_armor.base.last = -1;
	w_armor.base.draw = ST_drawNumber;
	w_armor.digits = ST_ARMORWIDTH;
	w_armor.font = numtall;	
	// Create arms widget
	w_arms.base.x = ST_ARMSBGX;
	w_arms.base.y = ST_ARMSBGY;
	w_arms.count = ST_ARMS_COUNT;
	w_arms.sub = (st_widget_t*)Z_Malloc(w_arms.count * sizeof(st_wi_num_t), PU_STATIC, NULL);
	aux_num = (st_wi_num_t*)w_arms.sub;
	for (i = 0; i < w_arms.count; i++, aux_num++) {
		aux_num->base.x = ST_ARMSX + (i % 3) * ST_ARMSXSPACE;
		aux_num->base.y = ST_ARMSY + (i / 3) * ST_ARMSYSPACE;
		aux_num->base.last = -1;
		aux_num->base.draw = ST_drawNumber;
		aux_num->base.bg = NULL;
		aux_num->digits = 1;
		aux_num->value = i + 2;
	}	
	// Create keys widget
	w_keys.base.x = ST_KEY0X;
	w_keys.base.y = ST_KEY0Y;
	w_keys.count = ST_KEY_COUNT;
	w_keys.sub = (st_widget_t*)Z_Malloc(w_keys.count * sizeof(st_widget_t), PU_STATIC, NULL);
	aux_wi = (st_widget_t*)w_keys.sub;
	for (i = 0; i < w_keys.count; i++, aux_wi++) {
		aux_wi->x = ST_KEY0X;
		aux_wi->y = ST_KEY0Y + (i % w_keys.count) * ST_KEYYSPACE;
		sprintf(namebuf, "STKEYS%d", i);
		aux_wi->bg = (patch_t *)W_CacheLumpName(namebuf, PU_STATIC);
		aux_wi->last = ST_KEYFLAGSOFF;
		aux_wi->draw = ST_drawKey;
	}	
	// Create ammo widget
	w_ammo.base.x = ST_AMMO0X;
	w_ammo.base.y = ST_AMMO0Y;
	w_ammo.count = ST_AMMO_SLOTS;
	w_ammo.sub = (st_widget_t*)Z_Malloc(w_ammo.count * 2 * sizeof(st_wi_num_t), PU_STATIC, NULL);
	aux_num = (st_wi_num_t*)w_ammo.sub;
	for (i = 0; i < w_ammo.count; i++, aux_num++) {
		// Ammo counter
		aux_num->base.x = ST_AMMO0X;
		aux_num->base.y = ST_AMMO0Y + (i % ST_AMMO_SLOTS) * ST_AMMOYSPACE;
		aux_num->base.last = -1;
		aux_num->base.draw = ST_drawNumber;
		aux_num->font = y_numshort;
		aux_num->digits = ST_AMMO0WIDTH;
		// Ammo capacity
		(aux_num + ST_AMMO_SLOTS)->base.x = ST_MAXAMMO0X + ST_AMMOXSPACE;
		(aux_num + ST_AMMO_SLOTS)->base.y = ST_MAXAMMO0Y + (i % ST_AMMO_SLOTS) * ST_AMMOYSPACE;
		(aux_num + ST_AMMO_SLOTS)->base.last = -1;
		(aux_num + ST_AMMO_SLOTS)->base.draw = ST_drawNumber;
		(aux_num + ST_AMMO_SLOTS)->font = y_numshort;
		(aux_num + ST_AMMO_SLOTS)->digits = ST_AMMO0WIDTH;
	}

	// Create face widget
	w_face.base.x = ST_FACESX;
	w_face.base.y = ST_FACESY;
	w_face.frame = 0;
	w_face.base.last = 0;
	w_face.count = (ST_NUMPAINFACES * (ST_NUMSTRAIGHTFACES + 5)) + 2;
	w_face.frames = (patch_t**)Z_Malloc(w_face.count * sizeof(patch_t*), PU_STATIC, NULL);
	for (i = 0; i < ST_NUMPAINFACES; i++)
	{
		for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
		{
			sprintf(namebuf, "STFST%d%d", i, j); // three normal faces per pain level
			w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
		}
		sprintf(namebuf, "STFTR%d0", i); // turn right
		w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
		sprintf(namebuf, "STFTL%d0", i); // turn left
		w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
		sprintf(namebuf, "STFOUCH%d", i); // ouch!
		w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
		sprintf(namebuf, "STFEVL%d", i); // evil grin ;)
		w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
		sprintf(namebuf, "STFKILL%d", i); // pissed off
		w_face.frames[w_face.frame++] = W_CacheLumpName(namebuf, PU_STATIC);
	}
	w_face.frames[w_face.frame++] = W_CacheLumpName("STFGOD0", PU_STATIC);  // GOD
	w_face.frames[w_face.frame++] = W_CacheLumpName("STFDEAD0", PU_STATIC); // DEAD
	w_face.frame = 0;
}

//
// ST_doPaletteStuff
//
static void ST_doPaletteStuff(void)
{
	int palette;
	byte *pal;
	int cnt;
	int bzc;

	cnt = plyr->damagecount;

	if (plyr->powers[pw_strength])
	{
		// slowly fade the berzerk out
		bzc = 12 - (plyr->powers[pw_strength] >> 6);

		if (bzc > cnt)
			cnt = bzc;
	}

	if (cnt)
	{
		palette = (cnt + 7) >> 3;

		if (palette >= NUMREDPALS)
			palette = NUMREDPALS - 1;

		palette += STARTREDPALS;
	}
	else if (plyr->bonuscount)
	{
		palette = (plyr->bonuscount + 7) >> 3;

		if (palette >= NUMBONUSPALS)
			palette = NUMBONUSPALS - 1;

		palette += STARTBONUSPALS;
	}
	else if (plyr->powers[pw_ironfeet] > 4 * 32 || plyr->powers[pw_ironfeet] & 8)
		palette = RADIATIONPAL;
	else
		palette = 0;

	if (palette != st_palette)
	{
		st_palette = palette;
		pal = (byte *)W_CacheLumpNum(lu_palette, PU_CACHE) + palette * 768;
		I_SetPalette(pal);
	}
}

//
//
//
static void ST_ReallocBG(byte new_size)
{
	st_widget_t *tmp_wi;

	tmp_wi = (st_widget_t*)Z_Malloc(new_size * sizeof(st_widget_t), PU_STATIC, NULL);
	if (new_size > w_backgound.count) {
		memcpy(tmp_wi, w_backgound.sub, w_backgound.count * sizeof(st_widget_t));
	}
	else {
		memcpy(tmp_wi, w_backgound.sub, new_size * sizeof(st_widget_t));
	}

	w_backgound.count = new_size;
	Z_Free(w_backgound.sub);
	w_backgound.sub = tmp_wi;
}

//
// ST_Responder
//
boolean ST_Responder(event_t *ev)
{
	// Filter automap on/off.
	if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
	{
		switch (ev->data1)
		{
		case AM_MSGENTERED:			
			if (st_state == ST_HIDDEN) {
				st_last_state = st_state;
				st_state = ST_REDRAW;
			}
			break;

		case AM_MSGEXITED:
			if (st_last_state != st_state) {
				st_state = st_last_state;
			}
			break;
		}
	}
	// if a user keypress...
	else if (ev->type == ev_keydown)
	{
		
	}
	return false;
}

//
// ST_Ticker
//
void ST_Ticker(void)
{
	st_randomnumber = M_Random();
	ST_updateWidgets();
	st_oldhealth = plyr->health;
}

//
// ST_Visible
//
void ST_Visible(boolean visible) 
{
	st_state = visible ? ST_REDRAW : ST_HIDDEN;
}

//
// ST_Drawer
//
void ST_Drawer(void)
{
	// Do red-/gold-shifts from damage/items
	ST_doPaletteStuff();

	switch (st_state) 
	{
		case ST_HIDDEN:
		default:
			break;

		case ST_REDRAW:
			// draw status bar background to off-screen buff
			((st_widget_t*)&w_backgound)->draw((st_widget_t*)&w_backgound, true);
			// and refresh all widgets
			ST_drawWidgets(true);
			st_state = ST_VISIBLE;
			break;

		case ST_VISIBLE:
			ST_drawWidgets(false);
			break;
	}
}

//
// ST_Start
//
void ST_Start(player_t *pl)
{
	int i;
	char namebuf[9];

	if (plyr != NULL) {
		// ST_Stop
		I_SetPalette(W_CacheLumpNum(lu_palette, PU_CACHE));
	}

	// Init Data
	plyr = pl;

	st_palette = -1;
	st_priority = 0;
	st_lastattackdown = -1;
	st_oldhealth = 100;
	st_state = (st_state == ST_HIDDEN) ? ST_HIDDEN : ST_REDRAW;

	for (i = 0; i < NUMWEAPONS; i++)
		st_oldweaponsowned[i] = plyr->weaponowned[i];	

	if (netgame)
	{
		if (w_backgound.count == ST_BG_COUNT) {
			ST_ReallocBG(ST_BG_COUNT + 1);
		}
		// player bg color
		sprintf(namebuf, "STFB%d", consoleplayer);
		w_backgound.sub[ST_BG_COUNT].bg = (patch_t *)W_CacheLumpName(namebuf, PU_STATIC);
		w_backgound.sub[ST_BG_COUNT].x = ST_FX;
		w_backgound.sub[ST_BG_COUNT].y = ST_FY;
	}
	else {
		if (w_backgound.count != ST_BG_COUNT) {
			ST_ReallocBG(ST_BG_COUNT);
		}
	}

	w_face.frame = 0;
}

//
// ST_Init
//
void ST_Init(void)
{
	lu_palette = W_GetNumForName("PLAYPAL");
	ST_loadGraphics();
	screens[BG] = (byte *)Z_Malloc(ST_WIDTH * ST_HEIGHT, PU_STATIC, NULL);
}