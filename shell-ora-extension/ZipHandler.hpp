#pragma once

#include <Thumbcache.h>
#include <Propsys.h>
#include <wincodec.h>

#include <string>

class ZipHandler : 
		public IThumbnailProvider
	,	public IInitializeWithStream
	,	public IPropertyStore
	{
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

		//IThumbnailProvider
		virtual HRESULT __stdcall GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha );

		//IInitializeWithFile
		virtual HRESULT __stdcall Initialize( IStream *pstream, DWORD grfMode );
	
		//IPropertyStore
		virtual HRESULT __stdcall Commit();
		virtual HRESULT __stdcall GetAt( DWORD iProp, PROPERTYKEY *pkey );
		virtual HRESULT __stdcall GetCount( DWORD *cProps );
		virtual HRESULT __stdcall GetValue( REFPROPERTYKEY key, PROPVARIANT *pv );
		virtual HRESULT __stdcall SetValue( REFPROPERTYKEY key, REFPROPVARIANT propvar );

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
		IPropertyStoreCache* prop_cache;

		void read_xml( std::string xml );
};

