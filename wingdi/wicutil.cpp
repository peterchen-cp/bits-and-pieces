#include "../pch.h"
#include "wicutil.h"
#include "bmputil.h"

namespace GDIUtil
{

   /** Loads a PNG image from the specified stream (using Windows Imaging Component).
   */
   IWICBitmapSourcePtr WICLoadBitmapFromStream(IStream * imageStream)
   {
      // load WIC's PNG decoder
      IWICBitmapDecoderPtr decoder;
      HRESULT hr = S_OK;

      do
      {
         hr = decoder.CreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER);
         if (FAILED(hr)) break;

         // load the PNG
         hr = decoder->Initialize(imageStream, WICDecodeMetadataCacheOnLoad);
         if (FAILED(hr)) break;

         // check for the presence of the first frame in the bitmap
         UINT nFrameCount = 0;
         hr = decoder->GetFrameCount(&nFrameCount);
         if (FAILED(hr)) break;

         if (nFrameCount != 1)
         {
            SetLastError(ERROR_INVALID_DATA);
            return nullptr;
         }

         // load the first frame (i.e., the image)
         IWICBitmapFrameDecodePtr frame;
         hr = decoder->GetFrame(0, &frame);
         if (FAILED(hr)) break;

         // convert the image to 32bpp BGRA format with pre-multiplied alpha
         //   (it may not be stored in that format natively in the PNG resource,
         //   but we need this format to create the DIB to use on-screen)
         IWICBitmapSourcePtr bitmap;
         hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &bitmap);
         if (FAILED(hr)) break;
         return bitmap;
      } while (0);

      SetLastError(hr);
      return nullptr;
   }

   /** creates a HBITMAP from an IWICBitmapSource */
   HBITMAP WICCreateHBITMAP(IWICBitmapSource * ipBitmap)
   {
      HRESULT hr = S_OK;
      HBITMAP result = 0;

      do
      {
         UINT width = 0;
         UINT height = 0;
         hr = ipBitmap->GetSize(&width, &height);
         if (!FAILED(hr) && (!width || !height))
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
         if (FAILED(hr)) break;

         // create a DIB section that can hold the image
         uint32_t * pvImageBits = nullptr;
         result = CreateRGBADIBSection({ static_cast<LONG>(width), -static_cast<LONG>(height) }, &pvImageBits);
         if (!result)
            return nullptr;

         DWORD * dbgdwdata = (DWORD *)pvImageBits;

         // extract the image into the HBITMAP
         const UINT cbStride = width * 4;
         const UINT cbImage = cbStride * height;
         hr = ipBitmap->CopyPixels(NULL, cbStride, cbImage, reinterpret_cast<BYTE *>(pvImageBits));
         if (FAILED(hr))
         {
            DeleteObject(result);
            break;
         }
      } while (0);

      if (FAILED(hr))
      {
         SetLastError(hr);
         return nullptr;
      }
      return result;
   }

} // namespace GDIUtil
