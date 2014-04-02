#pragma once
#include "ZipHandler.hpp"

class OraHandler : public ZipHandler {
protected:
	virtual std::string mimetype() const { return "image/openraster"; }

	virtual bool isThumbnail( std::string filename ) const 
		{ return filename == "Thumbnail/thumbnail.png"; }

	virtual bool isMeta( std::string filename ) const 
		{ return filename == "stack.xml"; }
	
	void readMeta( std::string xml );
};

