+------------------------------------------------------------+
|                * Kaptan Yosun's Baseline *                 |
+------------------------------------------------------------+

+-------------------------------------------------+----------+
| General / Extern Information                    | Version  |
+-------------------------------------------------+----------+
| Base                                            | Mainline |
| DirectX                                         | 9Ex      |
| Character Encoding                              | UTF-8    |
+-------------------------------------------------+----------+

+-------------------------------+---------+--------+---------+
| Extern Information            | Server  | Client | Version |
|-------------------------------+---------+--------+---------+
| MySQL                         | YES     | NO     |         |
| Granny                        | NO      | YES    | 2.11.8  |
| SpeedTree                     | NO      | YES    | 1.6.0   |
| LZO                           | NO      | YES    |         |
| MiniLZO                       | YES     | NO     | 2.10    |
| DiscordRPC                    | NO      | YES    |         |
| MilesSoundSystem              | NO      | YES    |         |
| Boost                         | YES     | YES    | 1.90.0  |
| CryptoPP                      | YES     | YES    | 8.9.0   |
| DevIL (Static)                | YES     | YES    | 1.8.0   |
+-------------------------------+---------+--------+---------+

+-----------------------------------+--------+--------+------+
| Removed Systems/Significant Code  | Server | Client | Pack |
+-----------------------------------+--------+--------+------+
| Xtrap                             | YES    | YES    | NO   |
| Hackshield                        | YES    | YES    | NO   |
| nProtect_GameGuard                | NO     | YES    | NO   |
| libserverkey                      | YES    | NO     | NO   |
| PC_Bang            [+ Dump Proto] | YES    | YES    | YES  |
| Lottery            [+ Dump Proto] | YES    | YES    | YES  |
| Teen                              | YES    | NO     | NO   |
| passpod                           | YES    | YES    | YES  |
| matrix_card                       | YES    | YES    | YES  |
| Vcard                             | YES    | NO     | NO   |
| Over9Refine                       | YES    | NO     | NO   |
| Auction                           | YES    | YES    | YES  |
| Monarch + Castle / Siege          | YES    | NO     | NO   |
| BattleArena                       | YES    | NO     | NO   |
| Threeway_War                      | YES    | NO     | NO   |
| SpeedServer                       | YES    | NO     | NO   |
| BlockCountryIp                    | YES    | NO     | NO   |
| Software Tiling                   | NO     | YES    | YES  |
| Auth_Brazil & Auth_Japan          | YES    | NO     | NO   |
| OpenID                            | YES    | YES    | YES  |
| Mobile - SMS                      | YES    | YES    | YES  |
| Virtual Keyboard                  | NO     | NO     | YES  |
| Panama                            | YES    | YES    | NO   |
| CSHybridCrypt                     | YES    | YES    | NO   |
| Sequence                          | YES    | YES    | YES  |
| empire_text_convert               | YES    | YES    | YES  |
| DanceEvent                        | YES    | NO     | NO   |
| AutoBan                           | NO     | YES    | YES  |
| AdminPage                         | YES    | NO     | NO   |
| ProcessCRC / MagicCube            | YES    | YES    | NO   |
| Billing                           | YES    | NO     | YES  |
| Login_sim                         | YES    | NO     | NO   |
| NetDatagram                       | YES    | YES    | YES  |
+-----------------------------------+--------+--------+------+

+-----------------------------------+--------+--------+------+
| Removed Small/Insignificant Code  | Server | Client | Pack |
+-----------------------------------+--------+--------+------+
| All locale code                   | YES    | YES    | YES  |
| Hotbackup                         | YES    | NO     | NO   |
| ChinaEventServer                  | YES    | NO     | NO   |
| distribution_test_server          | YES    | NO     | NO   |
| AlphaFog                          | NO     | YES    | NO   |
| Netmarble                         | YES    | NO     | NO   |
| HammerOfTor                       | YES    | NO     | NO   |
| Roulette                          | YES    | NO     | NO   |
| VIETNAM_CONVERT_WEAPON_VNUM       | NO     | YES    | NO   |
| DBString / GreetMessage           | YES    | NO     | NO   |
| ip_ban                            | YES    | NO     | NO   |
| GRAPHICS_CAPS_CAN_NOT_DRAW_LINE   | NO     | YES    | NO   |
| GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW | NO     | YES    | NO   |
| GRAPHICS_CAPS_HALF_SIZE_IMAGE     | NO     | YES    | NO   |
| ...CAN_NOT_TEXTURE_ADDRESS_BORDER | NO     | YES    | NO   |
| IsLowTextureMemory                | NO     | YES    | NO   |
| IsHighTextureMemory               | NO     | YES    | NO   |
| IsFastTNL                         | NO     | YES    | NO   |
| SupportDXT                        | NO     | YES    | NO   |
| PutImage16                        | NO     | YES    | NO   |
| CREATE_FORMAT                     | NO     | YES    | NO   |
| uTexBias                          | NO     | YES    | NO   |
| IsTLVertexClipping                | NO     | YES    | NO   |
| IsHAL                             | NO     | YES    | NO   |
| InitializeQuestItemTable          | YES    | NO     | NO   |
| EFFECT_LEVELUP_xx_FOR_GERMANY     | YES    | YES    | YES  |
| FullBackImage                     | NO     | NO     | YES  |
| OLD_rtrim                         | YES    | NO     | NO   |
| do_mount                          | YES    | NO     | NO   |
| malloc_allocator                  | YES    | NO     | NO   |
| noncopyable                       | YES    | NO     | NO   |
| Texcache                          | NO     | YES    | YES  |
| GrpVertexShader / GrpPixelShader  | NO     | YES    | NO   |
| GrpShadowTexture                  | NO     | YES    | NO   |
| FileMonitor_FreeBSD               | YES    | NO     | NO   |
| GrpVertexBufferDynamic            | NO     | YES    | NO   |
| GrpVertexBufferStatic             | NO     | YES    | NO   |
| CPostIt                           | NO     | YES    | NO   |
| CheckLatestFiles                  | NO     | YES    | NO   |
| EterPackCursor                    | NO     | YES    | NO   |
| OLD_SetNPC                        | NO     | NO     | YES  |
| __FISHING_MAIN__                  | YES    | NO     | NO   |
| Profiler                          | NO     | YES    | NO   |
| gost & gost_old                   | YES    | NO     | NO   |
| des.c & des.h                     | YES    | NO     | NO   |
| ProcessScanner                    | NO     | YES    | NO   |
| SendNPCPosition & RecvNPCList     | YES    | YES    | NO   |
+-----------------------------------+--------+--------+------+

+-----------------------------------+--------+--------+------+
| New Code                          | Server | Client | Pack |
+-----------------------------------+--------+--------+------+
| [REVERSED] ABORT_TRACEBACK_UPDATE | NO     | YES    | NO   | More detailed Python traceback with line printing.
| [REVERSED] CLIENT_LOCALE_STRING   | YES    | YES    | YES  | This system is the basis of Multilanguage.
| [REVERSED] ATLAS_MARK_INFO        | YES    | YES    | YES  | Pull NPC positions from Client instead of ServerFiles.
| YOSUN_MULTILANGUAGE [MINIMAL]     | YES    | NO     | NO   | Minimal implementation of Multilanguage.
|                                   |        |        |      |
+-----------------------------------+--------+--------+------+

+------------------------------------------------------------+
|                * Commands to set up MySQL *                |
+------------------------------------------------------------+
CREATE DATABASE account;
CREATE DATABASE log;
CREATE DATABASE common;
CREATE DATABASE player;
GRANT ALL PRIVILEGES ON *.* TO 'kaptan'@'localhost' WITH GRANT OPTION;

+------------------------------------------------------------+
|                  * Commands for GitHub *                   |
+------------------------------------------------------------+
~Remove cached files in case gitignore doesn't ignore them:
git rm -r --cached *

~Generate a Diff file of the latest commit:
git show --pretty=format:%b > burayaisimyaz.diff


TODO:
"InvalidateRect() - Failed to LockRect" while attacking metins
maybe when there is a quest item to be found from mob
could be lacking d3dusage_dynamic