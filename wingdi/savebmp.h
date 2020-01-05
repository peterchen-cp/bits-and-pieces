#pragma once

namespace GDIUtil
{

   bool BitmapSaveToFile(LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
   bool BitmapSaveToFile(LPCTSTR pszFile, HBITMAP hBMP);
}
