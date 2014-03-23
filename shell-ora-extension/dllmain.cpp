// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "ClassFactory.h"
#include <new>

// {80CF1ACD-0EE8-409A-A22C-EC25BE82C647}
static const GUID CLSID_MZipHandler = 
{ 0x80cf1acd, 0xee8, 0x409a, { 0xa2, 0x2c, 0xec, 0x25, 0xbe, 0x82, 0xc6, 0x47 } };



BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved ){
	switch( ul_reason_for_call ){
		case DLL_PROCESS_ATTACH:
				DisableThreadLibraryCalls( hModule );
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

HRESULT __stdcall DllGetClassObject( REFCLSID objGuid, REFIID riid, void **ppv ){
	*ppv = NULL;
	
	if( IsEqualCLSID( objGuid, CLSID_MZipHandler ) ){
		ClassFactory *factory = new (std::nothrow) ClassFactory();
		if( !factory )
			return E_OUTOFMEMORY;

		factory->AddRef();
		HRESULT hr = factory->QueryInterface( riid, ppv );
		factory->Release();
		return hr;
	}
	else
		return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT __stdcall DllCanUnloadNow(){
	return (ClassFactory::dll_ref>0) ? S_FALSE : S_OK;
}

