#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <unknwn.h>
#include <stdio.h>
#include <stdint.h>

typedef HRESULT (WINAPI *CreateInput)(HINSTANCE,DWORD,const GUID*,void**,void*);
typedef const void *(WINAPI *JoystickFormat)(void);
int wmain(int argc, WCHAR **argv) {
    if (argc!=2) {fwprintf(stderr,L"Usage: proxy_smoke_test path-to-dinput8.dll\n");return 1;}
    HMODULE proxy=LoadLibraryW(argv[1]);
    if (!proxy) {printf("LoadLibrary failed: %lu\n",GetLastError());return 2;}
    const char *names[]={"DirectInput8Create","DllCanUnloadNow","DllGetClassObject","DllRegisterServer","DllUnregisterServer","GetdfDIJoystick"};
    for (int i=0;i<6;++i) {
        FARPROC byname=GetProcAddress(proxy,names[i]);
        FARPROC ordinal=GetProcAddress(proxy,(LPCSTR)(uintptr_t)(i+1));
        if (!byname || byname!=ordinal) {printf("Export mismatch: %s\n",names[i]);return 3;}
    }
    WCHAR path[MAX_PATH]; GetSystemDirectoryW(path,MAX_PATH); lstrcatW(path,L"\\dinput8.dll");
    HMODULE system=LoadLibraryW(path); if (!system) return 4;
    JoystickFormat a=(JoystickFormat)GetProcAddress(proxy,"GetdfDIJoystick");
    JoystickFormat b=(JoystickFormat)GetProcAddress(system,"GetdfDIJoystick");
    if (!a() || a()!=b()) {puts("Joystick descriptor was not forwarded correctly");return 5;}
    static const GUID iid={0xbf798031,0x483a,0x4da2,{0xaa,0x99,0x5d,0x64,0xed,0x36,0x97,0x00}};
    CreateInput pa=(CreateInput)GetProcAddress(proxy,"DirectInput8Create");
    CreateInput pb=(CreateInput)GetProcAddress(system,"DirectInput8Create");
    IUnknown *object_a=NULL,*object_b=NULL;
    HRESULT ha=pa(GetModuleHandleW(NULL),0x0800,&iid,(void**)&object_a,NULL);
    HRESULT hb=pb(GetModuleHandleW(NULL),0x0800,&iid,(void**)&object_b,NULL);
    printf("DirectInput8Create proxy=%08lx system=%08lx\n",(unsigned long)ha,(unsigned long)hb);
    if (ha!=hb || FAILED(ha) || !object_a || !object_b) return 6;
    IUnknown_Release(object_a); IUnknown_Release(object_b);
    Sleep(300);
    FreeLibrary(proxy); FreeLibrary(system);
    puts("PASS: all six name/ordinal exports, system forwarding, DirectInput creation/release.");
    return 0;
}
