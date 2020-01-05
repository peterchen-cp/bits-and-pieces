#include "../pch.h"
#include <comdef.h>
#include "bmputil.h"
#include "../core/finally.h"

namespace GDIUtil
{


   void CResourceData::Init(HRSRC rsrc, HMODULE module)
   {
      m_resInfo = rsrc;
      if (m_resInfo)
      {
         m_size = SizeofResource(module, m_resInfo);
         m_hgl = LoadResource(module, m_resInfo);
         if (m_hgl)
         {
            m_resData = LockResource(m_hgl);
            return;
         }
      }
      m_err = GetLastError();
   }
 
   CResourceData::CResourceData(WORD language, LPCTSTR type, ResID resID, HMODULE module)
   {
      Init(FindResourceEx(module, resID, type, language), module);
   }

   CResourceData::CResourceData(LPCTSTR type, ResID resID, HMODULE module)
   {
      Init(FindResource(module, resID, type), module);
   }


   namespace
   {
      /** calls GlobalAlloc(uFlags, size), and initializes the buffer by copying \c size bytes from \c data
         if data is \c nullptr, the memory is zero-initialized. If \c uFlags contains \c GMEM_MOVEABLE, 
         the memory is locked for initialization. If locking fails, the memory is freed.
      */
      HGLOBAL GlobalAllocAndCopy(UINT uFlags, DWORD size, void const * data)
      {
         if (!data)
            uFlags |= GMEM_ZEROINIT;

         auto result = GlobalAlloc(uFlags, size);
         if (!result)
            return nullptr;

         if (size && data)
         {
            if (uFlags & GMEM_MOVEABLE)
            {
               void * locked = GlobalLock(result);
               if (!locked)
               {
                  GlobalFree(result);
                  return nullptr;
               }
               memcpy(locked, data, size);
               GlobalUnlock(result);
            }
            else
               memcpy((void *)result, data, size);
         }
         return result;
      }


      /** creates a stream on the specified data. 
          \param data      if not null, the data for the stream \c size bytes are copied to the stream's backing data. 
          \param size      size of the stream

          if \c data==nullptr, the stream is created with size \c size, and initialized with null data.
      */
      IStreamPtr CreateStreamOnCopyOf(void const * data, DWORD size)
      {
         auto mem = GlobalAllocAndCopy(GMEM_MOVEABLE, size, data);
         if (!mem)
            return IStreamPtr();

         IStreamPtr result;
         HRESULT hr = CreateStreamOnHGlobal(mem, TRUE, &result);
         if (FAILED(hr))
         {
            SetLastError(hr);
            GlobalFree(mem);
            return nullptr;
         }

         return result;
      }
   }

   /** Creates an IStream for reading resource data.
       The stream holds a copy of the resource data.
   */
   IStreamPtr ResourceAsStream(CResourceData const & res)
   {
      return CreateStreamOnCopyOf(res.ptr(), res.size());
   }

   /** Creates a device independent bitmap (DIB) with common options:

       32 bits/pixel (red, green, blue and alpha channel), no compression and no color palette.

       \param size: size as specified.
         A negative height creates a top-down DIB with the origin in the top left corner,
         A positive height a bottom-up DIB with the origin in the bottom left corner.

       \param imageBits [out, optional] receives a pointer to the underlying data of the bitmap,
       one RGBA quad per pixel.

   */
   HBITMAP CreateRGBADIBSection(SIZE size, uint32_t ** imageBits)
   {
      BITMAPINFO bminfo = {};
      bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bminfo.bmiHeader.biWidth = size.cx;
      bminfo.bmiHeader.biHeight = size.cy;
      bminfo.bmiHeader.biPlanes = 1;
      bminfo.bmiHeader.biBitCount = 32;
      bminfo.bmiHeader.biCompression = BI_RGB;

      void * vBits = nullptr;
      HDC hdcScreen = GetDC(NULL);
      HBITMAP result = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &vBits, NULL, 0);
      ReleaseDC(NULL, hdcScreen);

      if (imageBits)
         *imageBits = static_cast<uint32_t *>(vBits);

      return result;
   }


   /** Checks if  HBITMAP uses 32 bits/pixel */
   bool BitmapIsRGBA(HBITMAP bmp)
   {
      if (!bmp)
         return false;

      BITMAP bminfo = {};
      if (!GetObject(bmp, sizeof(bminfo), &bminfo))
         return false;

      return bminfo.bmBitsPixel == 32;
   }

   /**  Makes \c transparentColor transparent
       All pixels equal to \c transparentColor, are made transparent (alpha = 0) 
       and all other pixels fully opaque (alpha = 255). 

       \c bmp must be an RGBA (32 bit/pixel) DIB section that will be modified in-place.
       To create a transparent copy of other bitmaps, see \ref BitmapMakeTransparent
   */
   bool BitmapMakeTransparentInPlace(HBITMAP bmp, COLORREF transparentColor)
   {
      DIBSECTION dibinfo = {};
      if (!GetObject(bmp, sizeof(dibinfo), &dibinfo))
         return false;

      // must be a 32 bit uncompressed bitmap
      if (dibinfo.dsBmih.biBitCount != 32 ||
         dibinfo.dsBmih.biCompression != BI_RGB)
      {
         SetLastError(ERROR_INVALID_DATA);
         return false;
      }

      uint32_t * data = (uint32_t *)dibinfo.dsBm.bmBits;
      size_t totalPixels = std::abs(dibinfo.dsBmih.biHeight) * dibinfo.dsBmih.biWidth; // height is negative for "top-down" bitmaps
      _ASSERTE(((DWORD_PTR)data & 3) == 0); // expected to be DWORD-aligned.

      for (size_t i = 0; i < totalPixels; ++i)
         if (data[i] == transparentColor)
            data[i] = 0;
         else
            data[i] |= 0xFF000000;
      return true;
   }


   /** creates a transparent copy of bmp
       creates an RGBA copy of \c bmp and applies \ref BitmapMakeTransparentInPlace to it
   */
   HBITMAP BitmapMakeTransparent(HBITMAP bmp, COLORREF transparentColor)
   {
      HBITMAP result = (HBITMAP)CopyImage(bmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
      if (!result)
         return nullptr;

      if (!BitmapMakeTransparentInPlace(result, transparentColor))
      {
         DeleteObject(result);
         return nullptr;
      }
      return result;
   }

} // namespace GDIUtil
