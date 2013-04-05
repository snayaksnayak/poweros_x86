#include "coregfx.h"
#include "rastport.h"
#include "regions.h"
#include "pixmap.h"
#include "font.h"
#include "fontdrv.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

void dbcs_gettextbits(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int ch, UINT32 flags, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase)
{
	/* gettextbits function*/
	void (*func)(struct CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int ch, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
	if (ch >= 0x0100) {	/* character was DBCS-encoded*/
		switch (flags & TF_DBCSMASK) {
		default:
			*pwidth = *pheight = *pbase = 0;
			return;
		}
	} else /* not DBCS, use standard corefont routine*/
		func = gen_gettextbits;
	/* get text bits*/
	func(CoreGfxBase, pfont, ch, retmap, pwidth, pheight, pbase);
}

/*
 * Calc text size using specially-compiled-in DBCS fonts for each
 * non-ASCII character, else use normally selected font.
 */
void dbcs_gettextsize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, const unsigned short *str, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase)
{
	pCGfxCFont	pf = ((pCGfxCoreFont)pfont)->cfont;
	unsigned int	c;
	int		width = 0;

	flags &= TF_DBCSMASK;
	while(--cc >= 0) {
		c = *str++;
		if (c >= 0x0100) {	/* character was DBCS-encoded*/
			switch (flags) {
			case TF_DBCS_BIG5:
			case TF_DBCS_EUCCN:
			case TF_DBCS_JIS:
			/*case MWTF_DBCS_EUCJP:*/
				width += 12;
				continue;
			case TF_DBCS_EUCKR:
				width += 12;
				continue;
			}
		} else
			if(c >= pf->firstchar && c < pf->firstchar+pf->size) width += pf->width? pf->width[c - pf->firstchar]: pf->maxwidth;
	}
	*pwidth = width;
	*pheight = pf->height;
	*pbase = pf->ascent;
}
