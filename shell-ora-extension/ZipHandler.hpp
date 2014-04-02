#pragma once

#include <Thumbcache.h>
#include <Propsys.h>
#include <wincodec.h>

#include <string>

struct archive;

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
		ZipHandler();
		virtual ~ZipHandler();

	public:
		bool valid;
		std::string xml;
		std::string thumb;

		IWICImagingFactory *pFactory;

	private:
		ULONG ref_count;
		bool read_zip( archive* file );

	protected:
		IPropertyStoreCache* prop_cache;

		virtual std::string mimetype() const = 0;
		virtual bool isThumbnail( std::string filename ) const = 0;
		virtual bool isMeta( std::string filename ) const = 0;
		virtual void readMeta( std::string xml ){}
};

