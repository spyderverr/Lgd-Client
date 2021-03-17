#include "Main.h"

#if _MSC_VER >= 1914
	#include <filesystem>
#else
	#include <istream>
#endif

static constexpr WORD default_LoginServerPORT  = 44405;
static constexpr char default_IpAddress[]      = "127.0.0.1";
static constexpr char default_ClientVersion[]  = "1.07.17";
static constexpr char default_ClientSerial[]   = "jw45af7xf4wxj198";
static constexpr char default_ScreenShotPath[] = "ScreenShots\\Screen(%02d_%02d-%02d-%02d)-%04d.jpg";

WORD LoginServerPORT     = 0;
char IpAddress[14]       = { 0 };
char ClientVersion[8]    = { 0 };
char ClientSerial[17]    = { 0 };
char ScreenShotPath[100] = { 0 };

void ReadConfig()
{
	bool configFileExists = false;

#if _MSC_VER >= 1914
	if (std::filesystem::exists(CONFIG_FILEPATH))
	{
		configFileExists = true;
	}
#else
	std::ifstream myReadFile;
	myReadFile.open(CONFIG_FILEPATH);
	if (myReadFile.is_open())
	{
		configFileExists = true;
	}
#endif

	if (!configFileExists)
	{
		char errorMessage[256] = { 0 };

		sprintf(errorMessage,
			"File '%s' not found.\n\n"
			"Create file with default values.",
			CONFIG_FILEPATH);

		MessageBox(NULL, errorMessage, "Client", ERROR);

		// Create a config file with some default values
		WritePrivateProfileString(CONFIG_SECTION_MAIN_INFO, "IpAddress", default_IpAddress, CONFIG_FILEPATH);
		WritePrivateProfileString(CONFIG_SECTION_MAIN_INFO, "IpAddressPort", std::to_string(default_LoginServerPORT).c_str(), CONFIG_FILEPATH);
		WritePrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ClientVersion", default_ClientVersion, CONFIG_FILEPATH);
		WritePrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ClientSerial", default_ClientSerial, CONFIG_FILEPATH);
		WritePrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ScreenShotPath", default_ScreenShotPath, CONFIG_FILEPATH);

		exit(-1);
	}

#if _MSC_VER < 1914
	myReadFile.close();
#endif

	// Read the values from the config file
	LoginServerPORT = GetPrivateProfileInt(CONFIG_SECTION_MAIN_INFO, "IpAddressPort", default_LoginServerPORT, CONFIG_FILEPATH);

	GetPrivateProfileString(CONFIG_SECTION_MAIN_INFO, "IpAddress", default_IpAddress, IpAddress, sizeof(IpAddress), CONFIG_FILEPATH);
	GetPrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ClientVersion", default_ClientVersion, ClientVersion, sizeof(ClientVersion), CONFIG_FILEPATH);
	GetPrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ClientSerial", default_ClientSerial, ClientSerial, sizeof(ClientSerial), CONFIG_FILEPATH);
	GetPrivateProfileString(CONFIG_SECTION_MAIN_INFO, "ScreenShotPath", default_ScreenShotPath, ScreenShotPath, sizeof(ScreenShotPath), CONFIG_FILEPATH);
}

void InitEntryProc()
{
	CreateDirectory("Logs", NULL);
	CreateDirectory("ScreenShots", NULL);

	ReadConfig();
	
	for(int i = 0; i < MAIN_OFFSET_EP_SIZE; ++i)
	{
		SetByte(MAIN_OFFSET_EP + i, MainOffsetEP[i]);
	}

	for ( size_t i = 0; g_MainOffsetByteFix[i].offset != 0; ++i )
	{
		SetByte(g_MainOffsetByteFix[i].offset, g_MainOffsetByteFix[i].value);
	}

	// Note: Previously used to set the Screenshot path
	for ( size_t i = 0; g_MainOffsetDwordFix[i].offset != 0; ++i )
	{
		SetDword(g_MainOffsetDwordFix[i].offset, g_MainOffsetDwordFix[i].value);
	}

	for ( size_t i = 0; g_MainOffsetMemorySet[i].offset != 0; ++i )
	{
		MemorySet(g_MainOffsetMemorySet[i].offset, g_MainOffsetMemorySet[i].value, g_MainOffsetMemorySet[i].count);
	}

	// Note: Previously used to set the IP
	for ( size_t i = 0; g_MainOffsetMemoryCpy[i].offset != 0; ++i )
	{
		MemoryCpy(g_MainOffsetMemoryCpy[i].offset, g_MainOffsetMemoryCpy[i].value, g_MainOffsetMemoryCpy[i].size);
	}

	// Note: Set the Screenshot path here instead of g_MainOffsetDwordFix
	SetDword(0x0050EA4E, (DWORD)ScreenShotPath);

	// Note: Set the IP here instead of g_MainOffsetMemoryCpy
	MemoryCpy(IP_ADDRESS_OFFSET, (void*)IpAddress, sizeof(IpAddress));

	// Note: Moved the ClientVersion from g_MainOffsetByteFix
	SetByte(MAIN_OFFSET_CLIENT_VERSION, (ClientVersion[0] + 1));
	SetByte(MAIN_OFFSET_CLIENT_VERSION + 1, (ClientVersion[2] + 2));
	SetByte(MAIN_OFFSET_CLIENT_VERSION + 2, (ClientVersion[3] + 3));
	SetByte(MAIN_OFFSET_CLIENT_VERSION + 3, (ClientVersion[5] + 4));
	SetByte(MAIN_OFFSET_CLIENT_VERSION + 4, (ClientVersion[6] + 5));

	SetCompleteHook(0xFF, MAIN_OFFSET_DATASEND_HOOK_1, &DataSendHOOK);
	SetCompleteHook(0xFF, MAIN_OFFSET_DATASEND_HOOK_2, &DataSendHOOK);
	SetCompleteHook(0xFF, MAIN_OFFSET_DATASEND_HOOK_3, &DataSendHOOK);
	SetCompleteHook(0xFF, MAIN_OFFSET_PROTOCOL_CORE_HOOK, &ProtocolCoreEx);

	InitCommon();

	InitItem();

	InitPrintPlayer();
#if RECONNECT_ENABLED == 1
	InitReconnect();
#endif

	ApplyAntiCheat();

	ApplyNewEncodeFix();

	g_AntiCheat.InitKey();

	ApplyNewEncodeFix();
}

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			InitEntryProc();
			break;
		case DLL_PROCESS_DETACH:
			CloseSocket();
			sInputOutput.FreeMemory();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}

	return 1;
}