#pragma once

#include <unknwn.h>

class ClassFactory : public IClassFactory {
	public:
		//IUnknown
		virtual HRESULT __stdcall QueryInterface( const IID& iid, void** ppv ); 
		virtual ULONG __stdcall AddRef();
		virtual ULONG __stdcall Release();

		//IClassFactory
		virtual HRESULT __stdcall LockServer( BOOL flock );
		virtual HRESULT __stdcall CreateInstance( IUnknown *outer, REFIID riid, void **ppv );

	public:
		static ULONG dll_ref;
		ClassFactory() : ref_count( 0 ) { InterlockedIncrement( &dll_ref ); }
		~ClassFactory(){ InterlockedDecrement( &dll_ref ); }

	private:
		ULONG ref_count;
};

