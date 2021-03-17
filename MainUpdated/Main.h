#pragma once

constexpr char* CONFIG_FILEPATH = ".\\config-dev.ini";

constexpr char* CONFIG_SECTION_MAIN_INFO = "MainInfo";

extern WORD LoginServerPORT;
extern char IpAddress[14];
extern char ClientVersion[8];
extern char ClientSerial[17];

extern char ScreenShotPath[100];

extern void ReadConfig();
