#include "OraHandler.hpp"

#include <iostream>
#include <string>
#include <Shlwapi.h>

//TODO: find the type to use for "size"

char* read_string_file( unzFile file, unz_file_info64 info, const char* password=NULL ){
	if( unzOpenCurrentFilePassword( file, password ) != UNZ_OK )
		return NULL;
	
	//Create zero-terminated buffer
	char* buf = new char[ info.uncompressed_size + 1 ];
	buf[ info.uncompressed_size ] = 0;
	
	//Read file
	if( unzReadCurrentFile( file, buf, info.uncompressed_size ) < 0 ){
		//TODO: this function reads in chunks?
		delete[] buf;
		return NULL;
	}
	
	return buf;
}

bool read_binary_file( unzFile file, unz_file_info64 info, char** data, unsigned &size, const char* password=NULL ){
	*data = NULL;
	size = 0;
	
	if( unzOpenCurrentFilePassword( file, password ) != UNZ_OK )
		return false;
	
	//Create zero-terminated buffer
	char* buf = new char[ info.uncompressed_size + 1 ];
	buf[ info.uncompressed_size ] = 0;
	
	//Read file
	if( unzReadCurrentFile( file, buf, info.uncompressed_size ) < 0 ){
		//TODO: this function reads in chunks?
		delete[] buf;
		return false;
	}
	
	*data = buf;
	size = info.uncompressed_size;
	return true;
}

bool read_mzip( unzFile file, char* *xml, char* *thumbnail, unsigned &size ){
	*xml = NULL;
	*thumbnail = NULL;
	size = 0;
	
	unz_global_info64 gi;
	int err = unzGetGlobalInfo64( file, &gi );
	if( err != UNZ_OK ){
		std::cout << "ERROR in unzGetGlobalInfo64, code: " << err << "\n";
		return false;
	}
	
	bool mime_validated = false;
	
	//Iterate over each file
	for( uLong i=0; i<gi.number_entry; ){
		char filename_inzip[256];
      unz_file_info64 file_info;
		
		//Read info
		int err = unzGetCurrentFileInfo64( file, &file_info, filename_inzip, sizeof(filename_inzip), NULL,0,NULL,0 );
		if( err != UNZ_OK ){
			std::cout << "ERROR in unzGetCurrentFileInfo64, code " << err << "\n";
			break;
		}
		
		//Check for mime-type
		if( !mime_validated && std::string("mimetype") == filename_inzip ){
			//Validate compression and placement
			if( i!=0 || file_info.compression_method!=0 )
				return false;
			
			char* mime = read_string_file( file, file_info );
			if( !mime )
				return false;
			if( std::string("image/openraster") == mime )
				mime_validated = true;
			delete[] mime;
			
			if( !mime_validated )
				return false;
		}
		
		//Check for thumbnail
		if( !*thumbnail && std::string(filename_inzip).find("Thumbnails/thumbnail.") == 0 ){
			if( !read_binary_file( file, file_info, thumbnail, size ) ){
				std::cout << "Reading thumbnail failed\n";
				return false;
			}
			//TODO: what about file format?
		}
		
		//Check for meta
		if( !*xml && std::string("meta.xml") == filename_inzip )
			*xml = read_string_file( file, file_info );
		
		//Go to next file, but don't go past the end
		if( ++i<gi.number_entry && unzGoToNextFile( file ) != UNZ_OK ){
			std::cout << "ERROR in unzGoToNextFile" << "\n";
			return false;
		}
	}
	
	return mime_validated;
}


OraHandler::OraHandler()
	:	file( NULL )
	,	valid( false ), xml( NULL ), thumb( NULL )
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
	if( file ){
		unzClose( file );

		if( xml )
			delete[] xml;
		if( thumb )
			delete[] thumb;
	}
}



HRESULT __stdcall OraHandler::QueryInterface( const IID& iid, void** ppv ){
	if( iid == IID_IUnknown || iid == IID_IThumbnailProvider ){
		*ppv = static_cast<IThumbnailProvider*>(this);
	}
	else if( iid == IID_IInitializeWithFile ){
		*ppv = static_cast<IInitializeWithFile*>(this);
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

	IStream *stream = SHCreateMemStream( (BYTE*)thumb, size );
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

#include <iostream>

#define BUFFER_SIZE 1000
HRESULT __stdcall OraHandler::Initialize( LPCWSTR pszFilePath, DWORD grfMode ){
	size_t   i;
char *strChar = new char[BUFFER_SIZE];
wcstombs_s(&i, strChar, (size_t)BUFFER_SIZE, pszFilePath, (size_t)BUFFER_SIZE);

	std::cout << strChar << "\n";
	file = unzOpen64( strChar );
	if( file )
		valid = read_mzip( file, &xml, &thumb, size );
	valid = true;
	if( !file )
		return S_FALSE;
	if( !valid )
		return S_FALSE;
	std::cout << "Loading OK\n";
	return S_OK;
}
