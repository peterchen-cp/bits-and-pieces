#pragma once

#include <comdef.h>
#include <wincodec.h>

// interface declarations for Windows Imaging Components

_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(IWICBitmapSource));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));

namespace GDIUtil
{
   IWICBitmapSourcePtr WICLoadBitmapFromStream(IStream * imageStream);
   HBITMAP WICCreateHBITMAP(IWICBitmapSource * ipBitmap);
}
