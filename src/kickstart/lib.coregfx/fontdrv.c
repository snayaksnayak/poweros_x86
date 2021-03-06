#include "coregfx.h"
#include "rastport.h"
#include "regions.h"
#include "pixmap.h"
#include "font.h"
#include "fontdrv.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

#define SysBase CoreGfxBase->SysBase

#define NUMBER_FONTS 4

extern CGfxCFont font_winFreeSansSerif11x13;	/* new MWFONT_SYSTEM_VAR (was MWFONT_GUI_VAR)*/
extern CGfxCFont font_X6x13;			/* MWFONT_SYSTEM_FIXED (should be ansi)*/
/*extern MWCFONT font_winFreeSystem14x16;*/	/* deprecated MWFONT_SYSTEM_VAR*/
/*extern MWCFONT font_rom8x16;*/		/* deprecated MWFONT_OEM_FIXED*/
/*extern MWCFONT font_rom8x8, font_X5x7;*/	/* unused*/

/* handling routines for MWCOREFONT*/
CGfxFontProcs fontprocs = {
	0,				/* capabilities*/
	TF_ASCII,		/* routines expect ascii*/
	NULL,			/* init*/
	NULL,			/* createfont*/
	gen_getfontinfo,
	gen_gettextsize,
	gen_gettextbits,
	gen_unloadfont,
	gen_drawtext,
	NULL,			/* setfontsize*/
	NULL,			/* setfontrotation*/
	NULL,			/* setfontattr*/
	NULL			/* duplicate*/
};

/* first font is default font*/
CGfxCoreFont gen_fonts[NUMBER_FONTS] = {
	{&fontprocs, 0, 0, 0, 0, FONT_SYSTEM_VAR,   &font_winFreeSansSerif11x13},
	{&fontprocs, 0, 0, 0, 0, FONT_SYSTEM_FIXED, &font_X6x13},
	/* deprecated redirections for the time being*/
	{&fontprocs, 0, 0, 0, 0, "Helvetica",         &font_winFreeSansSerif11x13}, /* redirect*/
	{&fontprocs, 0, 0, 0, 0, "Terminal",          &font_X6x13}	/* redirect*/
};

/*  Sets the fontproc to fontprocs.  */
void gen_setfontproc(CGfxCoreFont *pf)
{
	pf->fontprocs = &fontprocs;
}

/*
 * Generalized low level get font info routine.  This
 * routine works with fixed and proportional fonts.
 */
BOOL gen_getfontinfo(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, pCGfxFontInfo pfontinfo)
{
	pCGfxCFont	pf = ((pCGfxCoreFont)pfont)->cfont;
	int			i;

	pfontinfo->maxwidth = pf->maxwidth;
	pfontinfo->height	= pf->height;
	pfontinfo->baseline = pf->ascent;
	pfontinfo->firstchar= pf->firstchar;
	pfontinfo->lastchar = pf->firstchar + pf->size - 1;
	pfontinfo->fixed 	= pf->width == NULL? TRUE: FALSE;
	for(i=0; i<256; ++i) 
	{
		if(pf->width == NULL) pfontinfo->widths[i] = pf->maxwidth;
		else 
		{
			if(i<pf->firstchar || i >= pf->firstchar+pf->size) pfontinfo->widths[i] = 0;
			else pfontinfo->widths[i] = pf->width[i-pf->firstchar];
		}
	}
	return TRUE;
}

/*
 * Generalized low level routine to calc bounding box for text output.
 * Handles both fixed and proportional fonts.  Passed ASCII or UC16 string.
 */
void gen_gettextsize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, const void *text, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase)
{
	pCGfxCFont		pf = ((pCGfxCoreFont)pfont)->cfont;
	const unsigned char *str = text;
	const unsigned short *istr = text;
	int				width;

	if(pf->width == NULL) width = cc * pf->maxwidth;
	else {
		width = 0;
		while(--cc >= 0) 
		{
			unsigned int	c;

			if (pfont->fontprocs->encoding == TF_UC16) c = *istr++;
			else c = *str++;
			/* if char not in font, map to first character by default*/
			if(c < pf->firstchar || c >= pf->firstchar+pf->size) c = pf->firstchar;
			/*if(c >= pf->firstchar && c < pf->firstchar+pf->size)*/
			width += pf->width[c - pf->firstchar];
		}
	}
	*pwidth = width;
	*pheight = pf->height;
	*pbase = pf->ascent;
}

/*
 * Generalized low level routine to get the bitmap associated
 * with a character.  Handles fixed and proportional fonts.
 */
void gen_gettextbits(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int ch, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase)
{
	pCGfxCFont		pf = ((pCGfxCoreFont)pfont)->cfont;
	int 			width;
	const UINT16 	*bits;

	/* if char not in font, map to first character by default*/
	if(ch < pf->firstchar || ch >= pf->firstchar+pf->size) ch = pf->firstchar;

	ch -= pf->firstchar;

	/* get font bitmap depending on fixed pitch or not*/
	/* automatically detect if offset is 16 or 32 bit */
	if( pf->offset ) 
	{
		if( ((UINT32 *)pf->offset)[0] >= 0x00010000 )
			bits = pf->bits + ((unsigned short *)pf->offset)[ch];
		else
			bits = pf->bits + ((UINT32 *)pf->offset)[ch];
	} else
		bits = pf->bits + (pf->height * ch);
		
 	width = pf->width ? pf->width[ch] : pf->maxwidth;
//	count = MWIMAGE_WORDS(width) * pf->height; 

	*retmap = bits;

	/* return width depending on fixed pitch or not*/
	*pwidth = width; 
	*pheight = pf->height;
	*pbase = pf->ascent; 
}

void gen_unloadfont(CoreGfxBase *CoreGfxBase, pCGfxFont pfont)
{
	/* builtins can't be unloaded*/
}

void gen_drawtext(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, CRastPort *rp, INT32 x, INT32 y, const void *text, int cc, UINT32 flags)
{
	const unsigned char *str = text;
	const unsigned short *istr = text;
	INT32		width;			/* width of text area */
	INT32	 	height;			/* height of text area */
	INT32		base;			/* baseline of text*/
	INT32		startx, starty;
	const UINT16 *bitmap;		/* bitmap for characters */
	BOOL		bgstate = rp->crp_useBg;
	int		clip;
	BLITFUNC convblit;
	CGfxBlitParms parms;
	PixMap *psd = rp->crp_PixMap;
	
	/* fill in unchanging convblit parms*/
	parms.op = ROP_COPY;					/* copy to dst, 1=fg (0=bg if usebg)*/
	parms.data_format = IF_MONOWORDMSB;	/* data is 1bpp words, msb first*/
	parms.fg_colorval = rp->crp_ForegroundRGB;
	parms.bg_colorval = rp->crp_BackgroundRGB;
	parms.fg_pixelval = rp->crp_Foreground;	/* for palette mask convblit*/
	parms.bg_pixelval = rp->crp_Background;
	parms.usebg = rp->crp_useBg;
	parms.srcx = 0;
	parms.srcy = 0;
	parms.dst_pitch = psd->pitch;			/* usually set in GdConversionBlit*/
	parms.data_out = psd->addr;
	parms.srcpsd = NULL;
	convblit = FindConvBlit(rp, IF_MONOWORDMSB, ROP_COPY);

	if (flags & TF_DBCSMASK) dbcs_gettextsize(CoreGfxBase, pfont, istr, cc, flags, &width, &height, &base);
	else pfont->fontprocs->FGetTextSize(CoreGfxBase, pfont, str, cc, flags, &width, &height, &base);
	
	if (flags & TF_BASELINE) y -= base;
	else if (flags & TF_BOTTOM) y -= (height - 1);
	startx = x;
	starty = y + base;

	/* pre-clip entire text area for speed*/
	switch (clip = ClipArea(rp, x, y, x + width - 1, y + height - 1)) 
	{
	case CLIP_VISIBLE:
		/* fast clear background once for all characters if drawing point by point*/
		if (!convblit && rp->crp_useBg) 
		{
			psd->_FillRect(rp, x, y, x + width - 1, y + height - 1, rp->crp_Background);
			SetUseBackground(rp, FALSE);
		}
		break;

	case CLIP_INVISIBLE:
		return;
	}

	/*
	 * Get the bitmap for each character individually, and then display
	 * them possibly using clipping for each one.
	 */
	while (--cc >= 0 && x < psd->xvirtres) 
	{
		/*
	 	 * If the string was marked as DBCS, then we've forced the conversion
	 	 * to UC16 in GdText.  Here we special-case the non-ASCII values and
	 	 * get the bitmaps from the specially-compiled-in font.  Otherwise,
	 	 * we draw them using the normal pfont->fontprocs->GetTextBits.
	 	 */
		if (flags & TF_DBCSMASK) dbcs_gettextbits(CoreGfxBase, pfont, *istr++, flags, &bitmap, &width, &height, &base);
		else {
			int ch;

			if (pfont->fontprocs->encoding == TF_UC16) ch = *istr++;
			else ch = *str++;
			pfont->fontprocs->FGetTextBits(CoreGfxBase, pfont, ch, &bitmap, &width, &height, &base);
		}

		/* use fast blit for text draw, fallback draw point-by-point*/
		if (convblit) {
			parms.dstx		= x;
			parms.dsty 		= y;
			parms.height	= height;
			parms.width		= width;
			parms.src_pitch = ((width + 15) >> 4) << 1;	/* pad to WORD boundary*/
			parms.data		= (char *)bitmap;
			/* skip clipping checks if fully visible*/
			if (clip == CLIP_VISIBLE) convblit(psd, &parms);
			else
				ConversionBlit(rp, &parms);
		} else
			BitmapByPoint(rp, x, y, width, height, bitmap, clip);
		x += width;
	}

	if (pfont->fontattr & TF_UNDERLINE) 
	{
		DPrintF("Underline\n");
		Line(rp, startx, starty, x, starty, FALSE);
	}

	/* restore background draw state*/
	SetUseBackground(rp, bgstate);
	FixCursor(psd);
}
