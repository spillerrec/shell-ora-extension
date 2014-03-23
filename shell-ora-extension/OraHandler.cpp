#include "OraHandler.hpp"

#include <iostream>
#include <string>
#include <Shlwapi.h>

#include <archive.h>
#include <archive_entry.h>

using namespace std;

//TODO: find the type to use for "size"

struct ReadingData{
	IStream& stream;
	unsigned buff_size; //TODO: how large should this be?
	char* buff;
	
	ReadingData( IStream& stream, int buff_size = 1024*4 ) : stream( stream ), buff_size(buff_size) {
		buff = new char[buff_size];
	}
	~ReadingData(){ delete[] buff; }
};
__LA_SSIZE_T stream_read( archive*, void* data, const void** buff ){
	ULONG bytes_read;
	ReadingData& stream = *(ReadingData*)data;
	stream.stream.Read( stream.buff, stream.buff_size, &bytes_read );
	*buff = stream.buff;
	return bytes_read;
}

int stream_close( archive*, void *data ){
	//((ReadingData*)data)->stream.Commit( STGC_DEFAULT );
	return ARCHIVE_OK;
}

static string read_data( archive* a ){
	string data;

	const char *buff;
	size_t size;
	__LA_INT64_T offset;

	while( true ){
		switch( archive_read_data_block( a, (const void**)&buff, &size, &offset ) ){
			case ARCHIVE_OK: data.append( buff, size ); break;
			case ARCHIVE_EOF: return data;
			default: 
				cout << archive_error_string(a);
				return data;
		}
	}
}

static std::string next_file( archive* a ){
	archive_entry *entry;
	switch( archive_read_next_header( a, &entry ) ){
		case ARCHIVE_EOF: return "";
		case ARCHIVE_OK:
			return archive_entry_pathname( entry );
			
		default:
			cout << archive_error_string(a);
			return "";
	}
}

bool read_zip( archive* file, string &xml, string &thumbnail ){
	string mime = next_file( file );
	if( mime != "mimetype" )
		return false;
	string mimetype = read_data( file );
	//NOTE: we could check contents, but whatever
	
	//Iterate over each file
	string filename;
	while( !(filename = next_file( file )).empty() ){
		//Check for thumbnail
		if( thumbnail.empty() && (
				filename.find( "Thumbnails/thumbnail." ) == 0
			||	filename.find( "thumb" ) == 0
			) ){
			thumbnail = read_data( file );
			//TODO: what about file format?
		}
		
		//Check for meta
		if( xml.empty() && "meta.xml" == filename )
			xml = read_data( file );
	}
	
	return true;
}


OraHandler::OraHandler()
	:	valid( false )
	,	pFactory( NULL )
	,	ref_count( 0 )
{
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory
		,	NULL
		,	CLSCTX_INPROC_SERVER
		,	IID_PPV_ARGS( &pFactory )
		);
}


OraHandler::~OraHandler(){

}



HRESULT __stdcall OraHandler::QueryInterface( const IID& iid, void** ppv ){
	if( iid == IID_IUnknown || iid == IID_IThumbnailProvider ){
		*ppv = static_cast<IThumbnailProvider*>(this);
	}
	else if( iid == IID_IInitializeWithStream ){
		*ppv = static_cast<IInitializeWithStream*>(this);
	}
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG __stdcall OraHandler::AddRef(){
	return InterlockedIncrement( &ref_count );
}

ULONG __stdcall OraHandler::Release(){
	if( InterlockedDecrement( &ref_count ) == 0 )
		delete this;
	return ref_count;
}


#include <algorithm>
HRESULT __stdcall OraHandler::GetThumbnail( UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha ){
	if( !valid || !pFactory )
		return S_FALSE;
	*phbmp = NULL;
	//return S_FALSE;

	IStream *stream = SHCreateMemStream( (BYTE*)thumb.c_str(), thumb.size() );
	if( !stream )
		return S_FALSE;

	IWICBitmapDecoder *ppIDecoder = NULL;
	HRESULT decoder_err = pFactory->CreateDecoderFromStream(
			stream
		,	NULL
		,	WICDecodeMetadataCacheOnDemand
		,	&ppIDecoder
		);

	IWICBitmapFrameDecode *pFrame = NULL;
	if( SUCCEEDED( decoder_err ) ){
		if( SUCCEEDED( ppIDecoder->GetFrame( 0, &pFrame ) ) ){
			UINT width, height;
			if( !SUCCEEDED( pFrame->GetSize( &width, &height ) ) ) return S_FALSE; //TODO: error
			double scale = (double)cx / (double)max( width, height );
			width = UINT(width * scale + 0.5);
			height = UINT(height * scale + 0.5);
			
			IWICBitmapScaler *pIScaler = NULL;
			if( SUCCEEDED( pFactory->CreateBitmapScaler( &pIScaler ) ) ){
				HRESULT scale_err = pIScaler->Initialize(
						pFrame
					,	width, height
					,	WICBitmapInterpolationModeFant
					);

				if( SUCCEEDED( scale_err ) ){
					IWICFormatConverter *pIFormatConverter = NULL;
					pFactory->CreateFormatConverter( &pIFormatConverter ); //TODO: err
					if( !SUCCEEDED( pIFormatConverter->Initialize( //TODO: err
							pIScaler
						,	GUID_WICPixelFormat32bppPBGRA
						,	WICBitmapDitherTypeNone
						,	NULL
						,	0.f
						,	WICBitmapPaletteTypeCustom
						) ) ) return 25;
					
					
					HDC hdc = ::GetDC(NULL);
					void *bits = 0;

					BITMAPINFO bi;
					bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
					bi.bmiHeader.biWidth = width;
					bi.bmiHeader.biHeight = -static_cast<LONG>(height);
					bi.bmiHeader.biPlanes = 1;
					bi.bmiHeader.biBitCount = 32;
					bi.bmiHeader.biCompression = BI_RGB;

					HBITMAP hBmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);

					WICRect rect = {0, 0, width, height};
					HRESULT hr = pIFormatConverter->CopyPixels( &rect, width * 4, width * height * 4, (BYTE*)bits ); // RGBA
					if( !SUCCEEDED( hr ) ) return S_FALSE;

					*phbmp = hBmp;

					ReleaseDC(NULL, hdc);
				}
				else
					return S_FALSE;
			}
			else
				return S_FALSE;
		}
		else
			return S_FALSE;
	}
	else
		return S_FALSE;

	//TODO: cleanup stream?

	return S_OK;
}


HRESULT __stdcall OraHandler::Initialize( IStream *pstream, DWORD grfMode ){
	HRESULT sucess = S_FALSE;

	if( !pstream )
		return S_FALSE;

	archive* file = archive_read_new();
	archive_read_support_compression_all( file );
	archive_read_support_format_all( file );
	
	ReadingData data( *pstream );
	if( !archive_read_open( file, &data, nullptr, stream_read, stream_close ) ){
		valid = read_zip( file, xml, thumb );
		sucess = S_OK;
	}

	archive_read_close( file );
	archive_read_free( file );

	return sucess;
}
