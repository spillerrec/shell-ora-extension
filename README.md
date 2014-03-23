# shell-ora-extension

Provides thumbnails in Windows file explorer for OpenRaster and similarly structured formats. Windows Vista/7/8/8.1 only.

### Installation

To install you need to manually modify your registry. Add the following to register the component:

```
HKEY_CLASSES_ROOT
 |-- .ora
 |    |-- ShellEx
 |    |    |-- {E357FCCD-A995-4576-B01F-234630154E96}
 |    |    |    |-- Key:REG_SZ  (Default) = {80CF1ACD-0EE8-409A-A22C-EC25BE82C647}
 |
 |-- CLSID
 |    |-- {80CF1ACD-0EE8-409A-A22C-EC25BE82C647}
 |    |    |-- InprocServer32
 |    |    |    |-- Key:REG_SZ  (Default) = "Full path to DLL file"
```

*{E357FCCD-A995-4576-B01F-234630154E96}* is the thumbnailer interface. {80CF1ACD-0EE8-409A-A22C-EC25BE82C647} is the actual ora thumbnailer implementation.

To enable the component, either restart *explorer.exe* or run a program which calls the following function:

```C++
SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0 );
```

### Building

**Dependencies**

- Windows SDK
- Visual Studio 2012
- zlib
- libarchive

**Building**

Ensure that zlib and libarchive are properly linked. Static linking was used for zlib and dynamic for libarchive, as this is what I got working. Paths are probably specific to my computer, as I have no idea on how to properly set up a VS project.

Make sure to select the same platform as your system, if you are running 64-bit Windows, compile a 64-bit DLL. A 32-bit DLL will not work.
