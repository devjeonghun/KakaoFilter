#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <atlstr.h>
#pragma comment(lib, "ws2_32.lib")
void Start();

DWORD dwKakao = 0;

DWORD Kakao_return_1 = 0;
BYTE jmp[6] = { 0xe9,0x00,0x00,0x00,0x00,0xc3 };
char *format1 = "Message - > %s\n";
int check = 0;
wchar_t strUnicode[512] = {0,};
char	buf[512] = { 0, };
char temp[512] = { 0, };
char *bufptr;
char *bufptr2;
char	*strUTF8;
int nLen;
bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask) return 0;
	return (*szMask) == NULL;

}
DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD i = 0; i<dwLen; i++)
		if (bCompare((BYTE*)(dwAddress + i), bMask, szMask)) return (DWORD)(dwAddress + i);
	return 0;
}

DWORD Hook(LPVOID lpFunction)
{
	DWORD dwAddr = Kakao_return_1 - 7;
	DWORD dwCalc = ((DWORD)lpFunction - dwAddr - 5);

	memcpy(&jmp[1], &dwCalc, 4);
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, jmp, 6, 0);
	return dwAddr;
}

char * replace(char *s, const char *olds, const char *news) {
	char *result, *sr;
	size_t i, count = 0;
	size_t oldlen = strlen(olds); if (oldlen < 1) return s;
	size_t newlen = strlen(news);


	if (newlen != oldlen) {
		for (i = 0; s[i] != '\0';) {
			if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
			else i++;
		}
	}
	else i = strlen(s);


	result = temp;
	if (result == NULL) return NULL;


	sr = result;
	while (*s) {
		if (memcmp(s, olds, oldlen) == 0) {
			memcpy(sr, news, newlen);
			sr += newlen;
			s += oldlen;
		}
		else *sr++ = *s++;
	}
	*sr = '\0';

	return result;
}

char * Filter(char *buf)
{
	
	buf=replace(buf, "³ª»Û", "ÂøÇÏÁö¸øÇÑ");
	buf=replace(buf, "¸ÁÇÒ", "ÈïÇÒ");

	return buf;
}

void __declspec(naked) Kakao_hook_1()
{
	__asm
	{
		lea eax, [esp + 0x1C]
		push eax
			mov ecx, ebx
			pushad

			mov strUTF8,esi
	}
	bufptr2 = strUTF8;

	nLen=MultiByteToWideChar(CP_UTF8, 0, strUTF8, strlen(strUTF8), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, strlen(strUTF8), strUnicode, nLen);
	nLen = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, buf, nLen, NULL, NULL);
	//buf[nLen] = 0;
	ZeroMemory(strUnicode, 512);
	

	
	bufptr=Filter(buf);
	
	nLen = MultiByteToWideChar(CP_ACP, 0, bufptr, strlen(bufptr), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, bufptr, strlen(bufptr), strUnicode, nLen);
	nLen = WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, strUnicode, lstrlenW(strUnicode), strUTF8, nLen, NULL, NULL);

	

	
	nLen = strlen(strUTF8);
	strUTF8[nLen] = 0;

	memcpy(bufptr2 - 8, &nLen, 1);
	memcpy(bufptr2, strUTF8, nLen);

	ZeroMemory(buf,512);
	ZeroMemory(temp, 512);
	ZeroMemory(strUnicode, 512);
	__asm{
		popad
		jmp[Kakao_return_1]
	}
}





void Start()
{
	DWORD dwSize = 0x180000;
	DWORD dwAddress = 0;
	do
	{
		dwKakao = (DWORD)GetModuleHandleA("KakaoTalk.exe");
		Sleep(10);
	} while (!dwKakao);
	Sleep(100);
	printf("Kakao Chat Filter!!\nCompany: Saint Security", dwKakao);

	dwAddress = FindPattern(dwKakao, dwSize, (PBYTE)"\x8d\x44\x24\x1C\x50\x8B\xCB\xE8", "xxxxxxxx");
	
	//printf("Address: %x\n", dwAddress);
	Kakao_return_1 = dwAddress + 7;
	Hook(Kakao_hook_1);
	
}
BOOL APIENTRY DllMain(HMODULE hModul, DWORD ul_reason_for_ca, LPVOID lpReserve)
{
	switch (ul_reason_for_ca)
	{
	case DLL_PROCESS_ATTACH:
		AllocConsole();
		freopen("CON", "w", stdout);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start, NULL, NULL, NULL);

	case DLL_THREAD_ATTACH:

	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}