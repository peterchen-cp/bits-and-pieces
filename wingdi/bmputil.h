#pragma once

#include <wtypes.h>
#include <stdint.h>
#include "../core/pointer_iterator_typedefs.h"

/** a very spotty collection of helpers for making some selected GDI operations easier
*/
namespace GDIUtil
{

   /** (VC-specific): \c HINSTANCE of the binary (EXE or DLL) this translation unit ends up in. 
       Can be used as "sensible default" when a \c HINSTANCE for loading a resource from is required.
   */
   extern "C" IMAGE_DOS_HEADER __ImageBase;
   const HINSTANCE ThisModule = (HINSTANCE)&__ImageBase;

   /** the typical wrapper for a Windows resource ID, accepts an UINT or an LPCTSTR
       see msdn:MAKEINTRESOURCE
   */
   struct ResID
   {
      LPCTSTR resID = nullptr;
      ResID() = default;
      ResID(LPCTSTR resID_) : resID(resID_) {}
      ResID(UINT resID_) : resID(MAKEINTRESOURCE(resID_)) {}

      operator LPCTSTR() const { return resID; }
   };


   /** Accessing resource data, 
       Combines \c FindResource(Ex), \c SizeofResource, 
       \c LoadResourceData and \c LockResourceData.

       The resource is valid as long as the \c module is loaded. 

       Implementation Note: \c Load/LockResourceData do not require a 
       Release or Unlock. (This was different in ... Win16)
   */
   class CResourceData : public pointer_interator_typedefs<BYTE>
   {
   public:

      CResourceData() = default;
      CResourceData(LPCTSTR type, ResID resID, HMODULE module = ThisModule);
      CResourceData(WORD language, LPCTSTR type, ResID resID, HMODULE module = ThisModule);

      explicit operator bool() const { return m_resData && m_size; }
      DWORD GetError() const { return m_err; }

      void const * ptr() const { return m_resData; }
      size_type size() const { return m_size; }
      const_iterator begin() const { return (BYTE *)m_resData; }
      const_iterator end() const { return begin() + size(); }

   private:
      void Init(HRSRC rsrc, HMODULE module);
      DWORD m_err = 0;
      HRSRC m_resInfo = nullptr;
      HGLOBAL m_hgl = nullptr;
      DWORD m_size = 0;
      void const * m_resData = nullptr;
   };

   IStreamPtr ResourceAsStream(CResourceData const & res);



   HBITMAP CreateRGBADIBSection(SIZE size, uint32_t ** imageBits = nullptr);
   bool BitmapIsRGBA(HBITMAP bmp);
   bool BitmapMakeTransparentInPlace(HBITMAP bmp, COLORREF transparentColor);
   HBITMAP BitmapMakeTransparent(HBITMAP bmp, COLORREF transparentColor);

}
