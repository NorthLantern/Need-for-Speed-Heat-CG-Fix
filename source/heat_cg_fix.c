/* Heat CG Fix - experimental. Authors: community contributor & Chordlight (OpenAI Codex).
 * No game code distributed. Only the SHA256-allowlisted executable is modified in RAM.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

static HMODULE self_module, real_input;
static INIT_ONCE input_once = INIT_ONCE_STATIC_INIT;
static FARPROC input_exports[6];
static const char *export_names[6] = {"DirectInput8Create", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer", "GetdfDIJoystick"};

static BOOL CALLBACK init_input(PINIT_ONCE once, PVOID arg, PVOID *context) {
    WCHAR path[MAX_PATH];
    (void)once; (void)arg; (void)context;
    UINT n = GetSystemDirectoryW(path, MAX_PATH);
    if (!n || n + 13 >= MAX_PATH) return FALSE;
    lstrcatW(path, L"\\dinput8.dll");
    real_input = LoadLibraryExW(path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!real_input) return FALSE;
    for (int i = 0; i < 6; ++i) {
        input_exports[i] = GetProcAddress(real_input, export_names[i]);
        if (!input_exports[i]) return FALSE;
    }
    return TRUE;
}
static FARPROC resolve_input(int index) {
    if (!InitOnceExecuteOnce(&input_once, init_input, NULL, NULL)) return NULL;
    return input_exports[index];
}
HRESULT WINAPI DirectInput8Create(HINSTANCE a, DWORD b, const GUID *c, void **d, void *e) {
    typedef HRESULT (WINAPI *Fn)(HINSTANCE,DWORD,const GUID*,void**,void*);
    Fn fn = (Fn)resolve_input(0); return fn ? fn(a,b,c,d,e) : E_FAIL;
}
HRESULT WINAPI DllCanUnloadNow(void) {
    typedef HRESULT (WINAPI *Fn)(void); Fn fn=(Fn)resolve_input(1); return fn ? fn() : S_FALSE;
}
HRESULT WINAPI DllGetClassObject(const GUID *a, const GUID *b, void **c) {
    typedef HRESULT (WINAPI *Fn)(const GUID*,const GUID*,void**);
    Fn fn=(Fn)resolve_input(2); return fn ? fn(a,b,c) : E_FAIL;
}
HRESULT WINAPI DllRegisterServer(void) {
    typedef HRESULT (WINAPI *Fn)(void); Fn fn=(Fn)resolve_input(3); return fn ? fn() : E_FAIL;
}
HRESULT WINAPI DllUnregisterServer(void) {
    typedef HRESULT (WINAPI *Fn)(void); Fn fn=(Fn)resolve_input(4); return fn ? fn() : E_FAIL;
}
const void * WINAPI GetdfDIJoystick(void) {
    typedef const void *(WINAPI *Fn)(void); Fn fn=(Fn)resolve_input(5); return fn ? fn() : NULL;
}

static void log_line(const char *message) {
    WCHAR path[MAX_PATH];
    DWORD n=GetModuleFileNameW(self_module,path,MAX_PATH);
    if (!n || n>=MAX_PATH) return;
    WCHAR *last=wcsrchr(path,L'\\'); if (!last) return;
    if ((size_t)(last-path)+16>=MAX_PATH) return;
    lstrcpyW(last+1,L"HeatCGFix.log");
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (file==INVALID_HANDLE_VALUE) return;
    DWORD written; WriteFile(file,message,(DWORD)strlen(message),&written,NULL);
    WriteFile(file,"\r\n",2,&written,NULL); CloseHandle(file);
}
static BOOL allowed_executable(const WCHAR *path) {
    static const BYTE expected[32]={0x57,0x3f,0x91,0xd6,0x98,0x4b,0x5d,0xcf,0xbb,0xea,0xcc,0x7e,0xf9,0x2c,0xa2,0xd8,0x9b,0x46,0xa7,0x6b,0x32,0x8d,0x71,0x41,0x17,0x47,0xb1,0x7c,0xd1,0x87,0x10,0x6c};
    BCRYPT_ALG_HANDLE alg=NULL; BCRYPT_HASH_HANDLE hash=NULL;
    HANDLE file=INVALID_HANDLE_VALUE; BYTE *buffer=NULL, digest[32]; BOOL ok=FALSE;
    if (BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,NULL,0)<0) goto done;
    if (BCryptCreateHash(alg,&hash,NULL,0,NULL,0,0)<0) goto done;
    file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (file==INVALID_HANDLE_VALUE) goto done;
    buffer=HeapAlloc(GetProcessHeap(),0,1024*1024); if (!buffer) goto done;
    DWORD count;
    for (;;) {
        if (!ReadFile(file,buffer,1024*1024,&count,NULL)) goto done;
        if (!count) break;
        if (BCryptHashData(hash,buffer,count,0)<0) goto done;
    }
    if (BCryptFinishHash(hash,digest,sizeof(digest),0)<0) goto done;
    ok=memcmp(digest,expected,sizeof(digest))==0;
done:
    if (buffer) HeapFree(GetProcessHeap(),0,buffer);
    if (file!=INVALID_HANDLE_VALUE) CloseHandle(file);
    if (hash) BCryptDestroyHash(hash);
    if (alg) BCryptCloseAlgorithmProvider(alg,0);
    return ok;
}
static DWORD WINAPI patch_worker(void *unused) {
    (void)unused;
    WCHAR path[MAX_PATH]; DWORD n=GetModuleFileNameW(NULL,path,MAX_PATH);
    if (!n || n>=MAX_PATH) return 0;
    const WCHAR *name=wcsrchr(path,L'\\'); name=name ? name+1 : path;
    if (lstrcmpiW(name,L"NeedForSpeedHeat.exe")) return 0;
    /* Pin this DLL: unloading while the worker is active is unsupported. */
    HMODULE pinned;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)&patch_worker,&pinned)) return 0;
    log_line("Heat CG Fix 0.1.0 experimental - Chordlight (OpenAI Codex). Starting.");
    if (!allowed_executable(path)) {log_line("UNSUPPORTED: executable SHA256 mismatch or read error. No patch applied.");return 0;}
    BYTE *base=(BYTE*)GetModuleHandleW(NULL);
    IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS64 *nt=(IMAGE_NT_HEADERS64*)(base+dos->e_lfanew);
    const SIZE_T rva=0x0e6d749b;
    if (dos->e_magic!=IMAGE_DOS_SIGNATURE || nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64 || nt->OptionalHeader.SizeOfImage<rva+7) {
        log_line("UNSUPPORTED: image layout. No patch applied.");return 0;
    }
    BYTE *site=base+rva;
    const BYTE original[7]={0x66,0x87,0x05,0x5d,0x99,0x29,0x02};
    const BYTE replacement[7]={0x66,0x89,0x05,0x5d,0x99,0x29,0x02};
    ULONGLONG deadline=GetTickCount64()+300000;
    while (GetTickCount64()<deadline) {
        BYTE current[7]; SIZE_T count=0;
        if (ReadProcessMemory(GetCurrentProcess(),site,current,7,&count) && count==7) {
            if (!memcmp(current,replacement,7)) {log_line("ACTIVE: instruction was already patched.");return 0;}
            if (!memcmp(current,original,7)) {
                DWORD old,ignored;
                if (!VirtualProtect(site+1,1,PAGE_EXECUTE_READWRITE,&old)) {log_line("ERROR: VirtualProtect failed. No patch applied.");return 0;}
                /* One aligned byte store; same instruction length. AX's old value is dead.
                 * Retains the global store but intentionally removes atomic ordering.
                 * Concurrent-observer compatibility is not fully established. */
                *(volatile BYTE*)(site+1)=0x89;
                BOOL flushed=FlushInstructionCache(GetCurrentProcess(),site,7);
                BOOL verified=ReadProcessMemory(GetCurrentProcess(),site,current,7,&count) && count==7 && !memcmp(current,replacement,7);
                if (!flushed || !verified) {
                    *(volatile BYTE*)(site+1)=0x87;
                    FlushInstructionCache(GetCurrentProcess(),site,7);
                }
                BOOL restored=VirtualProtect(site+1,1,old,&ignored);
                log_line(flushed && verified ? "APPLIED: opcode 87 -> 89; full instruction readback verified. Disk EXE unchanged." : "ERROR: patch verification failed; original opcode restored.");
                if (!restored) log_line("ERROR: page protection restore failed. Restart game before continuing.");
                return 0;
            }
        }
        Sleep(100);
    }
    log_line("TIMEOUT: expected instruction was not observed. No patch applied.");
    return 0;
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,void *reserved) {
    (void)reserved;
    if (reason==DLL_PROCESS_ATTACH) {
        self_module=instance; DisableThreadLibraryCalls(instance);
        HANDLE thread=CreateThread(NULL,0,patch_worker,NULL,0,NULL);
        if (thread) CloseHandle(thread);
    }
    return TRUE;
}
