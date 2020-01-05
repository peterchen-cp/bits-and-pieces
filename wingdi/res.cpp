#include "../pch.h"
#include "res.h"

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
}