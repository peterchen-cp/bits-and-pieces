#pragma once

#include <stdint.h>

/** a very spotty collection of helpers for making some selected GDI operations easier
*/
namespace GDIUtil
{

   HBITMAP CreateRGBADIBSection(SIZE size, uint32_t ** imageBits = nullptr);
   bool BitmapIsRGBA(HBITMAP bmp);
   bool BitmapMakeTransparentInPlace(HBITMAP bmp, COLORREF transparentColor);
   HBITMAP BitmapMakeTransparent(HBITMAP bmp, COLORREF transparentColor);

}
