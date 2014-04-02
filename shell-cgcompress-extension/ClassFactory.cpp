#include "ClassFactory.h"
#include "CgCompressHandler.hpp"

#include <new>

ULONG ClassFactory::dll_ref = 0;

HRESULT ClassFactory::QueryInterface( const IID& iid, void** ppv ){
	if( iid == IID_IUnknown || iid == IID_IClassFactory ){
		*ppv = this;
		this->AddRef();
		return S_OK;
	}
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

ULONG ClassFactory::AddRef(){
	return InterlockedIncrement( &ref_count );
}

ULONG ClassFactory::Release(){
	if( InterlockedDecrement( &ref_count ) == 0 )
		delete this;
	return ref_count;
}


HRESULT ClassFactory::LockServer( BOOL flock ){
	if( flock )
		InterlockedIncrement( &dll_ref );
	else
		InterlockedDecrement( &dll_ref );

	return S_OK;
}

HRESULT ClassFactory::CreateInstance( IUnknown *outer, REFIID riid, void **ppv ){
	*ppv = NULL;
	
	if( outer )
		return CLASS_E_NOAGGREGATION; //Aggregation not supported
	else{
		CgCompressHandler* handler = new (std::nothrow) CgCompressHandler();
		if( !handler )
			return E_OUTOFMEMORY;

		handler->AddRef();
		HRESULT hr = handler->QueryInterface( riid, ppv );
		handler->Release();
		return hr;
	}
}

