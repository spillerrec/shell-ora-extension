#pragma once

#include <Thumbcache.h>
#include <Propsys.h>
#include <wincodec.h>

#include <string>

class OraHandler : public IThumbnailProvider, public IInitializeWithStream{
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

		//IThumbnailProvider
		virtual HRESULT __stdcall GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha );

		//IInitializeWithFile
		virtual HRESULT __stdcall Initialize( IStream *pstream, DWORD grfMode );
	
	public:
		OraHandler();
		~OraHandler();

	public:
		bool valid;
		std::string xml;
		std::string thumb;

		IWICImagingFactory *pFactory;

	private:
		ULONG ref_count;
};

