#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <KeyValues.h>
#include <utlbuffer.h>

static char g_szBasedir[MAX_PATH];



bool DesiredApp(int appid)
{
	switch (appid)
	{
	case 220: //hl2
	case 240: //cs:s
	case 360: //hldm:s
	case 380: //ep1
	case 243750: //sdk2013
	case 320: //hl2dm
	case 400: //portal
	case 420: //ep2
	//case 1583720: //ez2
	//case 714070: //ez1
	//case 620: //p2
		return true;
	default:
		return false;
	}
}

void PatchSearchPath(KeyValues* keyv, const char* find, const char* basepath)
{
	char replace[MAX_PATH];
	strncpy(replace, basepath,MAX_PATH);
	strncat(replace, find, MAX_PATH);
	int findlen = strlen(find);
	for (KeyValues* value = keyv->GetFirstValue(); value; value = value->GetNextValue())
	{
		const char* str = value->GetString();
		if (!str)
			continue;
		int len = strlen(str);
		if (len >= findlen && strcmp(str + len - findlen, find) == 0)
		{
			value->SetStringValue(replace);
			return;
		}
	}
}

KeyValues* GetKeyvaluesFromFile(const char* dir, const char* name)
{
	FILE* fp = fopen(dir, "r");
	if (!fp)
	{
		MessageBox(0, dir, name, MB_OK);
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	char* fileRead = (char*)g_pMemAlloc->Alloc(sz+2);
	memset(fileRead, 0, sz+1);
	rewind(fp);
	fread((void*)fileRead, 1, sz, fp);
	fileRead[sz] = '\x00';
	KeyValues* keyv = new KeyValues(name);
	keyv->LoadFromBuffer(name, fileRead);
	fclose(fp);
	g_pMemAlloc->Free(fileRead);
	return keyv;
}

void GetAppManifest(const char* appid, const char* path)
{
	char manifestDir[MAX_PATH];
	strncpy(manifestDir, path, MAX_PATH);
	V_AppendSlash(manifestDir, MAX_PATH);
	strncat(manifestDir, "steamapps", MAX_PATH);
	V_AppendSlash(manifestDir, MAX_PATH);
	strncat(manifestDir, "appmanifest_", MAX_PATH);
	strncat(manifestDir, appid, MAX_PATH);
	strncat(manifestDir, ".acf", MAX_PATH);
	KeyValues* appmanifest = GetKeyvaluesFromFile(manifestDir, "AppState");
	const char* installDir = appmanifest->GetString("installdir");
	char appDir[MAX_PATH];
	strncpy(appDir, path, MAX_PATH);
	V_AppendSlash(appDir, MAX_PATH);
	strncat(appDir, "steamapps", MAX_PATH);
	V_AppendSlash(appDir, MAX_PATH);
	strncat(appDir, "common", MAX_PATH);
	V_AppendSlash(appDir, MAX_PATH);
	strncat(appDir, installDir, MAX_PATH);
	V_AppendSlash(appDir, MAX_PATH);
	
	V_FixSlashes(appDir,'\\');
	V_FixDoubleSlashes(appDir);

	char gameinfoDir[MAX_PATH];
	strncpy(gameinfoDir, g_szBasedir, MAX_PATH);
	V_AppendSlash(gameinfoDir, MAX_PATH);
	strncat(gameinfoDir, "jbmodce", MAX_PATH);
	V_AppendSlash(gameinfoDir, MAX_PATH);
	strncat(gameinfoDir, "gameinfo.txt", MAX_PATH);
	KeyValues* gameinfo = GetKeyvaluesFromFile(gameinfoDir, "GameInfo");
	KeyValues* searchpaths = gameinfo->FindKey("FileSystem", true)->FindKey("SearchPaths", true);
	
	switch (Q_atoi(appid))
	{
	case 220: //hl2
		break;
	case 240: //cs:s
		PatchSearchPath(searchpaths, "cstrike/cstrike_pak.vpk", appDir);
		break;
	case 243750:
		PatchSearchPath(searchpaths, "hl2/hl2_textures.vpk", appDir);
		PatchSearchPath(searchpaths, "hl2/hl2_sound_vo_english.vpk", appDir);
		PatchSearchPath(searchpaths, "hl2/hl2_sound_misc.vpk", appDir);
		PatchSearchPath(searchpaths, "hl2/hl2_misc.vpk", appDir);
		PatchSearchPath(searchpaths, "platform/platform_misc.vpk", appDir);
		PatchSearchPath(searchpaths, "platform", appDir);
		PatchSearchPath(searchpaths, "hl2mp", appDir);
		PatchSearchPath(searchpaths, "hl2mp/hl2mp_pak.vpk", appDir);
		break;
	case 320: //hl2dm
		break;
	case 360: //hldm:s
		PatchSearchPath(searchpaths, "hl1/hl1_pak.vpk", appDir);
		PatchSearchPath(searchpaths, "hl1mp/hl1mp_pak.vpk", appDir);
		break;
	case 380: //ep1
		PatchSearchPath(searchpaths, "episodic/ep1_pak.vpk", appDir);
		break;
	case 400: //portal
		PatchSearchPath(searchpaths, "portal/portal_pak.vpk", appDir);
		break;
	case 420: //ep2
		PatchSearchPath(searchpaths, "ep2/ep2_pak.vpk", appDir);
		break;
	default:
		char stupidwarning[128];
		snprintf(stupidwarning, 128, "YOU FORGOT TO ADD THE GAME (%s) TO GetAppManifest() IN launcher/launcher.cpp", appid);
		MessageBox(NULL, stupidwarning, "YOU BUFFOON", MB_OK | MB_ICONEXCLAMATION);
		break;
	}
	
	appmanifest->deleteThis();
	CUtlBuffer buf(0, 0, 0);
	gameinfo->RecursiveSaveToFile(buf, 0);
	gameinfo->deleteThis();
	
	FILE* fp = fopen(gameinfoDir, "w");
	fwrite(buf.Base(), 1, buf.PeekStringLength(), fp);
	fclose(fp);
	
}




int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char* pPath = getenv("PATH");

	// Use the .EXE name to determine the root directory
	if (!::GetModuleFileName((HINSTANCE)GetModuleHandle(NULL), g_szBasedir, MAX_PATH))
	{
		return 1;
	}
	char* pBuffer = strrchr(g_szBasedir, '\\');
	if (pBuffer)
	{
		*(pBuffer + 1) = '\0';
	}

	char szBuffer[4096];
	_snprintf(szBuffer, sizeof(szBuffer), "PATH=%s\\bin\\;%s", g_szBasedir, pPath);
	szBuffer[sizeof(szBuffer) - 1] = '\0';
	_putenv(szBuffer);
	printf("%x %x %s %x\n", (unsigned int)hInstance, (unsigned int)hPrevInstance, lpCmdLine, nCmdShow);
	
	_snprintf(szBuffer, sizeof(szBuffer),"%s\n", g_szBasedir);
	
	char steamDir[MAX_PATH];
	char steamLibraryFolders[MAX_PATH];
	DWORD BufferSize = 8192;
	RegGetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath", RRF_RT_REG_SZ, NULL, (PVOID)&steamDir, &BufferSize);
	if (!steamDir)
		return 1;
	strncpy(steamLibraryFolders,steamDir,MAX_PATH);
	strncat(steamLibraryFolders, "\\steamapps\\libraryfolders.vdf", MAX_PATH);
	KeyValues* libraryfolders = GetKeyvaluesFromFile(steamLibraryFolders, "libraryfolders");
	char sdk2013dir[MAX_PATH] = "";
	for (KeyValues* folder = libraryfolders->GetFirstTrueSubKey(); folder; folder = folder->GetNextTrueSubKey())
	{
		//Msg("%s %x\n", folder->GetName(), folder->FindKey("apps")->GetFirstValue());
		for (KeyValues* app = folder->FindKey("apps", true)->GetFirstValue(); app; app = app->GetNextValue())
		{
			const char* appid = app->GetName();

			if (DesiredApp(Q_atoi(appid)))
			{
				GetAppManifest(appid, folder->GetString("path"));
			}
			if (Q_atoi(appid) == 563560)
			{
				strncpy(sdk2013dir, folder->GetString("path"), MAX_PATH);
				strncat(sdk2013dir, "\\steamapps\\common\\Alien Swarm Reactive Drop\\reactivedrop.exe", MAX_PATH);
			}
		}
	}
	libraryfolders->deleteThis();
	if (sdk2013dir[0] == '\x00')
	{
		MessageBox(0, "Install Source SDK 2013 Base Multiplayer to play this.", "Launcher error", MB_OK);
	}
	char gameParam[1024];
	strncpy(gameParam, "-windowed -steam -game \"", 1024);
	strncat(gameParam, g_szBasedir, 1024);
	V_AppendSlash(gameParam, 1024);
	strncat(gameParam, "jbmodce\" ", 1024);
	strncat(gameParam, lpCmdLine, 1024);

	ShellExecute(0, "open", sdk2013dir, gameParam, steamDir, SW_SHOWDEFAULT);
	return 0;
}