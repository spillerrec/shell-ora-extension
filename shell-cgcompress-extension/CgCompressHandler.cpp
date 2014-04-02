#include "CgCompressHandler.hpp"

//For properties
#include <propkey.h>
#include <Propvarutil.h>

//Xml parsing
#include "pugixml/pugixml.hpp"

using namespace std;

void CgCompressHandler::readMeta( string xml ) {
	pugi::xml_document doc;
	doc.load_buffer( xml.c_str( ), xml.size( ) );
	pugi::xml_node image = doc.child( "image" );

	//Read width and height
	PROPVARIANT prop_width = { 0 };
	PROPVARIANT prop_height = { 0 };
	prop_height.vt = prop_width.vt = VT_UI4;
	prop_width.uiVal = image.attribute( "w" ).as_int( 0 );
	prop_height.uiVal = image.attribute( "h" ).as_int( 0 );
	prop_cache->SetValue( PKEY_Image_HorizontalSize, prop_width );
	prop_cache->SetValue( PKEY_Image_VerticalSize, prop_height );

	//width and height in string format
	PROPVARIANT prop_dims = { 0 };
	wstring dims_str = to_wstring( prop_width.uiVal ) + L"x" + to_wstring( prop_height.uiVal );
	InitPropVariantFromString( dims_str.c_str( ), &prop_dims );
	prop_cache->SetValue( PKEY_Image_Dimensions, prop_dims );
	//PropVariantClear( &prop_dims );

	//Read resolution
	PROPVARIANT prop_xres = { 0 };
	PROPVARIANT prop_yres = { 0 };
	prop_xres.vt = prop_yres.vt = VT_R8;
	prop_xres.dblVal = image.attribute( "xres" ).as_double( 72 );
	prop_yres.dblVal = image.attribute( "yres" ).as_double( 72 );
	prop_cache->SetValue( PKEY_Image_HorizontalResolution, prop_xres );
	prop_cache->SetValue( PKEY_Image_VerticalResolution, prop_yres );
}