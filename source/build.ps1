param([Parameter(Mandatory=$true)][string]$Zig)
$ErrorActionPreference='Stop'
$out=Join-Path $PSScriptRoot '..\release'
New-Item -ItemType Directory -Path $out -Force | Out-Null
& $Zig cc -target x86_64-windows-gnu -shared -O2 -Wall -Wextra -Werror -Wno-cast-function-type-mismatch -s (Join-Path $PSScriptRoot 'heat_cg_fix.c') (Join-Path $PSScriptRoot 'dinput8.def') -lbcrypt -o (Join-Path $out 'dinput8.dll')
if($LASTEXITCODE -ne 0){throw 'DLL compilation failed'}
& $Zig cc -target x86_64-windows-gnu -O2 -Wall -Wextra -Werror -Wno-cast-function-type-mismatch -municode (Join-Path $PSScriptRoot 'proxy_smoke_test.c') -o (Join-Path $out 'proxy_smoke_test.exe')
if($LASTEXITCODE -ne 0){throw 'Smoke test compilation failed'}
Get-FileHash -LiteralPath (Join-Path $out 'dinput8.dll') -Algorithm SHA256

