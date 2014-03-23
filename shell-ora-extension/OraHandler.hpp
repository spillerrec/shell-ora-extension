#pragma once

#include <Thumbcache.h>
#include <Propsys.h>
#include <wincodec.h>
#include "unzip.h"

class OraHandler : public IThumbnailProvider, public IInitializeWithFile{
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

		//IThumbnailProvider
		virtual HRESULT __stdcall GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha );

		//IInitializeWithFile
		virtual HRESULT __stdcall Initialize( LPCWSTR pszFilePath, DWORD grfMode );
	
	public:
		OraHandler();
		~OraHandler();

	public:
		unzFile file;
		bool valid;
		char* xml;
		char* thumb;
		unsigned size;

		IWICImagingFactory *pFactory;

	private:
		ULONG ref_count;
};

