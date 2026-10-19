# 《极品飞车：热度》过场动画严重卡顿修复  
Need for Speed Heat — Cutscene Stutter Fix  

Heat CG Fix v0.1.0 · 测试版 / Test Release  
作者 / Authors：弦灯 Chordlight（Codex-GPT-6.1 Sol）& NorthLantern  
首次验证 / First verified：2026.10.20  

只想安装？看第 2、3 节即可；第 6～8、11 节供感兴趣的玩家和维护者阅读。  
Just installing? Read Sections 2–3. Sections 6–8 and 11 cover technical details and maintenance.  

---  
## 1. 补丁说明  
如果开车、实时菜单都流畅，唯独过场动画卡成“PPT”，这个补丁可能有帮助。我们定位并修正了解码路径中的一处高开销操作，本机已通过动画、地图加载和实际游玩测试。目前仅验证一个 EXE 版本和一台电脑，不保证解决所有 CG 卡顿。  

## 1. About This Fix  
If driving and real-time menus are smooth but cutscenes play like a slideshow, this fix may help. We identified and changed an expensive operation in a decoding path; cutscenes, map loading, and gameplay passed local testing. Only one executable build and one PC are verified, so this will not necessarily fix every cause of cutscene stutter.  

## 2. 使用方法  
① 完全退出游戏。  
② Steam 库中右键游戏 → 管理 → 浏览本地文件。  
③ 只把 dinput8.dll 放到 NeedForSpeedHeat.exe 旁边。  
④ 从 Steam 正常启动，无需额外启动器、启动参数或管理员权限。  

已有同名 DLL？先确认用途并备份，再决定是否替换。覆盖会停用原文件的功能；本版不支持代理链，也不能靠改名叠加。卸载时需恢复原文件。  

## 2. Installation  
① Exit the game completely.  
② In your Steam Library, right-click the game → Manage → Browse local files.  
③ Place only dinput8.dll beside NeedForSpeedHeat.exe.  
④ Launch normally through Steam. No extra launcher, launch options, or administrator privileges are required.  

Already have a DLL with that name? Identify and back it up before deciding whether to replace it. Replacement disables its functionality. Proxy chaining and combining DLLs by renaming are unsupported. Restore the original file when uninstalling.  

## 3. 生效确认与卸载  
查看游戏目录的 HeatCGFix.log：APPLIED / ACTIVE 表示已生效；UNSUPPORTED 表示校验、布局或读取检查未通过；TIMEOUT 表示未等到预期指令。出现 ERROR 请保留日志；没有日志不能证明生效，可能未加载或无法写日志。  
卸载：退出游戏，删除本补丁 DLL，必要时恢复此前备份；日志可删除。  
修改仅存在于游戏运行内存中，不改硬盘 EXE、CPU 亲和性、效率模式、注册表或系统设置。  

## 3. Activation and Removal  
Check HeatCGFix.log in the game folder. APPLIED / ACTIVE confirms activation. UNSUPPORTED indicates a validation, layout, or read failure; TIMEOUT means the expected instruction was not observed. Keep the log if ERROR appears. No log does not confirm activation: the DLL may not have loaded or logging may have failed.  
To uninstall, exit the game, remove this DLL, and restore any previous DLL backup if applicable. The log may be deleted.  
Only runtime memory is changed; the on-disk EXE, CPU affinity, Efficiency Mode, registry, and system settings are untouched.  

## 4. 支持版本  
已验证：Steam 启动的 Windows x64 版，EXE 文件版本 1.0.60.7040。以以下 SHA256 和运行时指令检查为准；相同显示版本号不一定是同一 EXE，其他版本会拒绝修改。  

## 4. Supported Build  
Verified: Windows x64, launched through Steam, executable version 1.0.60.7040. Support requires the SHA256 below and matching runtime instructions. An identical displayed version number does not guarantee an identical EXE; other builds are rejected.  

NeedForSpeedHeat.exe SHA256:  
`573f91d6984b5dcfbbeacc7ef92ca2d89b46a76b328d71411747b17cd187106c`  

## 5. 测试环境与结果  
CPU：i5-14600KF（14 核 / 20 线程）  
GPU：RTX 5060（8 GB）　RAM：32 GB  
OS：Windows 11 Pro　Driver：617.42（32.0.16.1742）  
Storage：NVMe SSD　Resolution：2560 × 1440 / 100%  
画质以高 / 极高为主，VSync 开启，普通 Steam 启动。  
单文件自动加载、内存读回、动画、地图加载和实际游玩均通过。上述配置不是最低要求，也没有绑定硬件或盘符；暂无量化 FPS 数据，其他电脑、全部剧情和长时间稳定性待验证。  

## 5. Test Environment and Results  
Shared hardware and software specifications are listed above. Settings were mainly High / Ultra, with VSync enabled and a normal Steam launch.  
Automatic DLL loading, instruction readback, cutscenes, map loading, and gameplay passed. These are test specifications, not minimum requirements; the fix is not tied to specific hardware or drive letters. No quantified FPS gains are provided. Other PCs, all cutscenes, and long-session stability remain unverified.  

## 6. 排查过程  
我们先尝试了画面、驱动和音频设置，但没有解决问题，随后对比卡顿动画与流畅主菜单的 CPU 采样。动画中有 108,210 / 365,878 个游戏样本（约 29.6%）集中在同一指令地址，菜单中同一位置为 0。继续分析运行代码，发现该地址紧随一条内存 XCHG。  
采样存在偏移，比例不等于精确耗时或 FPS 提升。例程结构类似布尔位解码器，但尚未确认具体视频格式。  

## 6. Investigation  
Graphics, driver, and audio-setting changes did not help. Comparing CPU profiles of a stuttering cutscene and smooth menu revealed 108,210 / 365,878 game samples (~29.6%) at one address during the cutscene, versus zero there in the menu. Runtime disassembly placed that address immediately after a memory XCHG.  
Sampling can skid; these percentages are neither exact execution times nor FPS gains. The routine resembles a boolean bit decoder, but the exact video codec is unconfirmed.  

## 7. 修改原理  
该 XCHG 操作的两字节内存跨越 64 字节缓存行，并带有隐式原子锁定语义，是高开销的强嫌疑点。分析显示，读回 AX 的旧值在例程后续使用前会被覆盖，因此我们改用保留写入的 MOV，只改一个操作码字节：87 → 89。测试玩家随后确认动画流畅、加载和驾驶正常。  
这项修改取消了原子性及内存顺序保证，其他线程的共享访问尚未穷尽分析；有效的本机实测不代表所有环境都安全，也未通过硬件计数器完全证明底层因果。  

## 7. What Changes  
The XCHG accesses two bytes across a 64-byte cache-line boundary and has implicit atomic locking semantics, making it a strong suspect for excessive overhead. Its returned AX value is overwritten before use within the routine. We replace it with a store-preserving MOV, changing one opcode byte: 87 → 89. The player then confirmed smooth cutscenes, loading, and driving.  
This removes atomicity and ordering guarantees. Concurrent accesses have not been exhaustively analyzed; local success does not establish universal safety. Hardware counters were not used to prove the full causal chain.  

## 8. 单文件如何工作？  
dinput8.dll 将 DirectInput 的六个导出接口转交给 Windows 原版 DLL，后台线程校验 EXE 和目标指令，等待初始化后修改内存并读回确认，最多等待五分钟。已测试输入接口创建释放、名称及序号导出、系统转发和不支持版本的拒绝修改。没有网络通信、遥测或自动更新。  

## 8. How the DLL Works  
The proxy forwards all six DirectInput exports to the original Windows DLL. A background worker validates the EXE and target instruction, waits up to five minutes for initialization, then patches memory and verifies readback. Tests covered interface creation/release, named and ordinal exports, system forwarding, and unsupported-build refusal. There is no networking, telemetry, or automatic updating.  

## 9. 限制与反馈  
仅支持第 4 节的 EXE；其他原因造成的卡顿可能无效，不支持直接叠加同名代理。若黑屏、崩溃或输入异常，请退出、移除补丁并恢复原 DLL，再对比正常启动的表现。  
欢迎反馈成功或失败：电脑配置、系统及驱动、游戏来源及 EXE 版本或 SHA256、具体场景、动画与游玩效果、其他 Mod，以及 HeatCGFix.log。公开前请检查个人信息。  

## 9. Limitations and Feedback  
Only the executable in Section 4 is supported. Other causes of stutter may not improve, and stacking same-name proxies is unsupported. For black screens, crashes, or input issues, exit, remove this fix, restore any previous DLL, and compare a normal launch.  
Success and failure reports are welcome: include hardware, OS/driver, game distribution and EXE version or SHA256, affected scene, cutscene/gameplay results, other mods, and HeatCGFix.log. Review personal information before posting.  

## 10. 源码、署名与联系  
source/ 包含完整源码、构建脚本和接口测试源码；docs/ 包含技术说明、验证记录和维护指南。采用 MIT 许可证，转载请保留 LICENSE、来源和测试版说明。  
NorthLantern 提供问题与环境并完成实测，与弦灯 Chordlight 共同排查、修复和封装。本项目不代表 EA、Intel、NVIDIA 或 OpenAI 官方发布或背书。  
最初只是想下班后玩会儿游戏。希望这个小补丁能让大家少折腾，多玩一会儿。  

## 10. Source, Credits, and Contact  
source/ contains the complete source, build script, and interface test source; docs/ contains technical notes, validation records, and the maintenance guide. Licensed under MIT; retain LICENSE, attribution, and the test-release notice when sharing.  
NorthLantern reported and reproduced the issue and tested gameplay, working with Chordlight on investigation, repair, and packaging. This project is not officially published or endorsed by EA, Intel, NVIDIA, or OpenAI.  
It started with wanting to relax after work. We hope this small fix means less troubleshooting and more time playing.  

联系 / Contact：2930073938@qq.com / northlantern03@gmail.com  
文件校验 / File checksums：SHA256SUMS.txt  
dinput8.dll SHA256:  
`98b2d00cd8805d6723189fa85f34ad51b5102bea36289dcf3b5411277ca476ab`  

## 11. 给后续玩家 Agent 的快速接手指南  
版本更新后不要只改哈希、关闭校验或照搬旧地址。完整中英报告见 docs/Agent维护指南.txt；核心流程如下：  
① 检查日志、EXE SHA256、加载模块及代理冲突，区分加载失败、版本不支持和问题变化。  
② 移除补丁建立基线，短时采集卡顿 CG 与流畅场景的 CPU 及调用栈对照，确认新的异常热点。  
③ 在初始化后的运行代码中重新反汇编，复核控制流、指令边界、RIP 相对目标和访问宽度；磁盘受保护代码不等于运行代码。  
④ 证明读回值无用，保留原写入，并评估共享内存的原子性和并发风险。旧版用的是 MOV [mem], AX，不是丢掉写入的 MOV AX, [mem]。  
⑤ 先做有身份及完整字节校验、写后读回和回退的最小内存实验。动画、地图、驾驶与输入通过后，再为新版本单独加入支持条目。  
⑥ 重新构建，测试系统转发、拒绝不支持版本、普通 Steam 启动及卸载回退，提交源码差异、证据、新哈希和已知限制。不成立就回退继续排查，不要把所有 CG 卡顿归为同一原因。  

## 11. Quick Handoff for Players' Agents  
After an update, do not merely change the hash, disable checks, or reuse old addresses. The complete bilingual report is in docs/Agent维护指南.txt. Core workflow:  
① Check logs, executable SHA256, loaded modules, and proxy conflicts. Distinguish loading failure, unsupported build, and a changed issue.  
② Remove the fix for a clean baseline. Compare short CPU/stack captures of a stuttering cutscene and a smooth scene to identify the new hotspot.  
③ Disassemble initialized runtime code; verify control flow, instruction boundaries, RIP-relative targets, and access width. Protected on-disk code is not equivalent to executed code.  
④ Establish that the returned value is unused, preserve the store, and assess atomicity and concurrency risks. The old replacement is MOV [mem], AX—not the store-dropping MOV AX, [mem].  
⑤ First test a minimal memory change with identity/full-byte guards, readback, and rollback. Only after cutscenes, loading, driving, and input pass should a separate supported-build entry be added.  
⑥ Rebuild and test system forwarding, unsupported-build refusal, normal Steam loading, and removal/rollback. Deliver source changes, evidence, new hashes, and limitations. If the hypothesis fails, revert and investigate other causes.  

旧版线索 / Old-build clues — 仅供定位，不可直接套用 / hints only:  
Routine RVA: 0x0e6d7470–0x0e6d74ed  
XCHG RVA: 0x0e6d749b → Sampled IP RVA: 0x0e6d74a2  
Original:    66 87 05 5D 99 29 02  
Replacement: 66 89 05 5D 99 29 02  
Build (Zig 0.15.2): pwsh -File source/build.ps1 -Zig C:/path/to/zig.exe  

