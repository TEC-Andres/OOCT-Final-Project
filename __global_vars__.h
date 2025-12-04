#ifndef __GLOBAL_VARS__H
#define __GLOBAL_VARS__H

#include <windows.h>

#define HEX(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

static const COLORREF bg = HEX(0x2e, 0x30, 0x2f);
static const COLORREF bg_title = HEX(0x0f, 0x24, 0x24);


#endif // __GLOBAL_VARS__H