# Validation record — 0.1.0 experimental

Date: 2026-10-20, Asia/Shanghai.

## Automated / inspected

- Compiled x64 C DLL with Zig 0.15.2, optimization enabled, `-Wall -Wextra -Werror`. The function-pointer-cast diagnostic is disabled specifically for the Win32 GetProcAddress API convention.
- DLL size: 22,528 bytes.
- DLL SHA256: `98b2d00cd8805d6723189fa85f34ad51b5102bea36289dcf3b5411277ca476ab`.
- Confirmed all six DirectInput exports with expected ordinals 1–6.
- Loaded the DLL in a separate smoke-test process; confirmed forwarded joystick descriptor equals the system DLL's result.
- Created and released a DirectInput8W interface through the proxy and the Windows DLL; both returned S_OK.
- Ran a test executable named NeedForSpeedHeat.exe with a different SHA256; observed explicit UNSUPPORTED refusal, with no patch applied.
- Installed the DLL beside the real game executable, then launched through `Steam -applaunch 1222680`, without the previous patch launcher.
- Inspected game modules: both the game-directory proxy and the original Windows system dinput8.dll were loaded.
- DLL log reported APPLIED. An independent process-memory read confirmed the full seven-byte replacement instruction, `6689055D992902`.
- Game process was responding after normal Steam launch.
- Disk game-executable SHA256 remained the supported original hash.

## Player observation

Before building the DLL, the player confirmed the identical one-byte runtime change fixed the cutscene stuttering and passed map loading and driving tests.

The player subsequently confirmed that the single-file DLL build also passed cutscene playback, entry into gameplay, and normal play after the ordinary Steam launch. Automatic loading, instruction readback, and player-observed functionality are therefore confirmed on the initial test PC. Long-duration sessions and other PCs have not been tested. Update this record with subsequent reports rather than assuming broader coverage.

