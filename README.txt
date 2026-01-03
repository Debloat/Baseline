+------------------------------------------------------------+
|                * Kaptan Yosun's Baseline *                 |
+------------------------------------------------------------+

+-------------------------------------------------+----------+
| General / Extern Information                    | Version  |
+-------------------------------------------------+----------+
| Base                                            | Mainline |
| DirectX                                         | 8        |
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