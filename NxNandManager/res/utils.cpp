#include "utils.h"
using namespace std;

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wString, 4096); //Fix issue #1
	return wString;
}

LPWSTR convertCharArrayToLPWSTR(const char* charArray)
{
	int nSize = MultiByteToWideChar(CP_UTF8, 0, charArray, -1, NULL, 0); //Fix issue #1
	LPWSTR wString = new WCHAR[nSize];
	MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wString, 4096);
	return wString;
}

u64 GetFilePointerEx (HANDLE hFile) {
	LARGE_INTEGER liOfs={0};
	LARGE_INTEGER liNew={0};
	SetFilePointerEx(hFile, liOfs, &liNew, FILE_CURRENT);
	return liNew.QuadPart;
}

unsigned long sGetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) return std::string(); //No error message has been recorded

	for (ErrorLabel el : ErrorLabelArr)
	{
		if (el.error == errorMessageID)
			return std::string(el.label);
	}

	LPSTR messageBuffer = NULL;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}


constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
						   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(unsigned char *data, int len)
{
	std::string s(len * 2, ' ');
	for (int i = 0; i < len; ++i)
	{
		s[2 * i]	 = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	for (auto & c: s) c = toupper(c);

	return s;
}

void hexStrPrint(unsigned char *data, int len)
{
    len = (len / 32)*32;
    std::string s;
    for (int i=0; i<len ; i+=32)
        s.append(hexStr(&data[i], 32)).append("\n");

    printf("%s\n", s.c_str());
}

void flipBytes(u8* buf, size_t len)
{
    u8* tmp_buf = (u8*)malloc(len);
    for (size_t i(0); i < len; i++)
        memcpy(&tmp_buf[i], &buf[len-1-i], 1);
    memcpy(buf, tmp_buf, len);
    free(tmp_buf);
}

BOOL AskYesNoQuestion(const char* question, void* p_arg1, void* p_arg2)
{
	BOOL bContinue = TRUE;
	string s;
	while (bContinue)
	{
        if (NULL != p_arg1)
        {
            if (NULL != p_arg2)
                printf(question, p_arg1, p_arg2);
            else
                printf(question, p_arg1);
        }
        else
            printf(question);
		printf(" (Y/N) : ");
		cin >> ws;
		getline(cin, s);
		if (s.empty())
		{
			continue;
		}
		switch (toupper(s[0]))
		{
		case 'Y':
			return TRUE;
			break;
		case 'N':
			return FALSE;
			break;
		}
	}
	return FALSE;
}

std::string GetReadableSize(u64 size)
{
	char buf[100];
	if (size / (1024 * 1024 * 1024) > 0)
	{
        sprintf_s(buf, sizeof(buf), "%.2f GB", (double)size / (1024 * 1024 * 1024));
	}
	else if (size / (1024 * 1024) > 0)
	{
        sprintf_s(buf, sizeof(buf), "%.2f MB", (double)size / (1024 * 1024));
	}
	else if (size / 1024 > 0)
	{
        sprintf_s(buf, sizeof(buf), "%.2f KB", (double)size / 1024);
	}
	else
	{
		sprintf_s(buf, sizeof(buf), "%I64d byte%s", size, size>1 ? "s" : "");
	}
	return std::string(buf);
}

std::string GetReadableElapsedTime(std::chrono::duration<double> elapsed_seconds)
{
	char buf[64];
	u32 seconds = (u32)elapsed_seconds.count();
	int minutes = seconds / 60;
	if (minutes > 0) seconds = seconds % 60;
	int hours = minutes / 60;
	if (hours > 0) minutes = minutes % 60;    
	if ((int)elapsed_seconds.count() > 1)
	{
		sprintf_s(buf, 64, "%02d:%02d:%02d", hours, minutes, seconds);
	} else {
		sprintf_s(buf, 64, "%.2fs", elapsed_seconds.count());
	}
	return std::string(buf);
}

void throwException(int rc, const char* errorStr)
{
	if (NULL != errorStr) printf("%s\n", errorStr);
	else {
		for (int i=0; i < (int)array_countof(ErrorLabelArr); i++)
		{
			if(ErrorLabelArr[i].error == rc)
			{
				printf("ERROR: %s\n", ErrorLabelArr[i].label);
                SetThreadExecutionState(ES_CONTINUOUS);
                exit(rc);
			}
		}
        std::string lstErrStr = GetLastErrorAsString();
        if(lstErrStr.length()) printf("ERROR : %s\n", lstErrStr.c_str());
	}
	SetThreadExecutionState(ES_CONTINUOUS);
	exit(rc);
}

void throwException(const char* errorStr, void* p_arg1, void* p_arg2)
{
	SetThreadExecutionState(ES_CONTINUOUS);
	if (NULL != errorStr) 
	{
		if (NULL != p_arg1)
		{
			if (NULL != p_arg2) printf(errorStr, p_arg1, p_arg2);
			else printf(errorStr, p_arg1);
			printf("\n");
		}
		else
			printf("%s\n", errorStr);
	}
	exit(EXIT_FAILURE);
}

char * flipAndCodeBytes(const char * str,  int pos, int flip, char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf[0] = '\0';
	if (pos <= 0)
		return buf;

	if (!j)
	{
		char p = 0;

		// First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char)(c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char)(c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && !isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}
	}

	if (!j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if (!isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if (!j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		// Flip adjacent characters
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}

	// Trim any beginning and end space
	i = j = -1;
	for (k = 0; buf[k] != '\0'; ++k)
	{
		if (!isspace(buf[k]))
		{
			if (i < 0)
				i = k;
			j = k;
		}
	}

	if ((i >= 0) && (j >= 0))
	{
		for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
			buf[k - i] = buf[k];
		buf[k - i] = '\0';
	}

	return buf;
}

std::string ExePath()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	wstring ws(buffer);
	return string(ws.begin(), ws.end());
}

std::wstring ExePathW()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    wstring ws(buffer);
    return ws;
}

HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;
	GetModuleHandleEx(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCTSTR)GetCurrentModule,
				&hModule);

	return hModule;
}

bool file_exists(const wchar_t * fileName)
{
#if defined(__MINGW32__) || defined(__MINGW64__) || defined(__MSYS__)
	char buffer[_MAX_PATH];
	std::wcstombs(buffer, fileName, _MAX_PATH);
	std::ifstream infile(buffer);
#else
	std::ifstream infile(fileName);
#endif
	return infile.good();
}

const std::string WHITESPACE = " \n\r\t\f\v";
const std::wstring WHITESPACEW = L" \n\r\t\f\v";


std::string ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::wstring rtrimW(const std::wstring& s)
{
    size_t end = s.find_last_not_of(WHITESPACEW);
    return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
}

std::string trim(const std::string& s)
{
	return rtrim(ltrim(s));
}

int digit_to_int(char d)
{
	char str[2];

	str[0] = d;
	str[1] = '\0';
	return (int)strtol(str, NULL, 10);
}

bool is_file(const char* path) {
	struct stat buf;
	stat(path, &buf);
	return S_ISREG(buf.st_mode);
}

bool is_dir(const char* path) {
	struct stat buf;
	stat(path, &buf);
	return S_ISDIR(buf.st_mode);
}


int parseKeySetFile(const char *keyset_file, KeySet* keyset)
{

    int num_keys = 0;
    ifstream readFile(keyset_file);
    string readout;
    std::string delimiter = ":";
    std::string value = "";
    keyset->other_keys.clear();
    if (readFile.is_open())
    {
        memset(keyset->crypt0, 0, 33);
        memset(keyset->tweak0, 0, 33);
        memset(keyset->crypt1, 0, 33);
        memset(keyset->tweak1, 0, 33);
        memset(keyset->crypt2, 0, 33);
        memset(keyset->tweak2, 0, 33);
        memset(keyset->crypt3, 0, 33);
        memset(keyset->tweak3, 0, 33);

        while (getline(readFile, readout)) {
            value.clear();
            if (readout.find("BIS KEY 0 (crypt)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->crypt0, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 0 (tweak)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->tweak0, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 1 (crypt)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->crypt1, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 1 (tweak)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->tweak1, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 2 (crypt)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->crypt2, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 2 (tweak)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->tweak2, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 3 (crypt)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->crypt3, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("BIS KEY 3 (tweak)") != std::string::npos) {
                value = trim(readout.substr(readout.find(delimiter) + 2, readout.length() + 1));
                strcpy_s(keyset->tweak3, value.substr(0, 32).c_str());
                num_keys++;
            }
            else if (readout.find("bis_key_00") != std::string::npos) {
                value = trim(readout.substr(readout.find("=") + 2, readout.length() + 1));
                strcpy_s(keyset->crypt0, value.substr(0, 32).c_str());
                strcpy_s(keyset->tweak0, value.substr(32, 32).c_str());
                num_keys += 2;
            }
            else if (readout.find("bis_key_01") != std::string::npos) {
                value = trim(readout.substr(readout.find("=") + 2, readout.length() + 1));
                strcpy_s(keyset->crypt1, value.substr(0, 32).c_str());
                strcpy_s(keyset->tweak1, value.substr(32, 32).c_str());
                num_keys += 2;
            }
            else if (readout.find("bis_key_02") != std::string::npos) {
                value = trim(readout.substr(readout.find("=") + 2, readout.length() + 1));
                strcpy_s(keyset->crypt2, value.substr(0, 32).c_str());
                strcpy_s(keyset->tweak2, value.substr(32, 32).c_str());
                num_keys += 2;
            }
            else if (readout.find("bis_key_03") != std::string::npos) {
                value = trim(readout.substr(readout.find("=") + 2, readout.length() + 1));
                strcpy_s(keyset->crypt3, value.substr(0, 32).c_str());
                strcpy_s(keyset->tweak3, value.substr(32, 32).c_str());
                num_keys += 2;
            }
            else if (readout.find("=") != std::string::npos) {
                GenericKey key;
                key.name = trim(readout.substr(0, readout.find("=")));
                key.key = trim(readout.substr(readout.find("=") + 2, readout.length() + 1));
                keyset->other_keys.emplace_back(key);
            }
        }
    }
    else {
        return 0;
    }

    // toupper keys
    for (int i = 0; i < strlen(keyset->crypt0); i++) keyset->crypt0[i] = toupper(keyset->crypt0[i]);
    for (int i = 0; i < strlen(keyset->crypt1); i++) keyset->crypt1[i] = toupper(keyset->crypt1[i]);
    for (int i = 0; i < strlen(keyset->crypt2); i++) keyset->crypt2[i] = toupper(keyset->crypt2[i]);
    for (int i = 0; i < strlen(keyset->crypt3); i++) keyset->crypt3[i] = toupper(keyset->crypt3[i]);
    for (int i = 0; i < strlen(keyset->tweak0); i++) keyset->tweak0[i] = toupper(keyset->tweak0[i]);
    for (int i = 0; i < strlen(keyset->tweak1); i++) keyset->tweak1[i] = toupper(keyset->tweak1[i]);
    for (int i = 0; i < strlen(keyset->tweak2); i++) keyset->tweak2[i] = toupper(keyset->tweak2[i]);
    for (int i = 0; i < strlen(keyset->tweak3); i++) keyset->tweak3[i] = toupper(keyset->tweak3[i]);

    readFile.close();
    return num_keys;
}


std::string GetGenericKey(KeySet* keyset, std::string name)
{
    if (!keyset)
        return "";

    for (auto k : keyset->other_keys) if (k.name == name)
        return k.key;

    return "";
}

bool HasGenericKey(KeySet* keyset, std::string name)
{
    if (!keyset)
        return false;

    for (auto k : keyset->other_keys) if (k.name == name)
        return true;

    return false;
}


void app_printf (const char *format, ...)
{
    va_list args;
    va_start( args, format );
    vprintf(format, args);
    va_end( args );
    fflush(stdout);
}
void app_wprintf (const wchar_t *format, ...)
{
    va_list args;
    va_start( args, format );
    vwprintf(format, args);
    va_end( args );
    fflush(stdout);
}

void dbg_printf (const char *format, ...)
{
	if(!isdebug)
		return;

	va_list args;
	va_start( args, format );
	vprintf(format, args);
	va_end( args );    

#if defined(ENABLE_GUI)
    if (isGUI)
    {
        char line[MAX_PATH];
        va_start( args, format );
        vsprintf(line, format, args);
        va_end( args );
        writeDebugLine(std::string(line));
        fflush(stdout);
    }
#endif
}

void dbg_wprintf (const wchar_t *format, ...)
{
	if(!isdebug)
		return;

	va_list args;

#if defined(ENABLE_GUI)
    if (isGUI)
    {
        wchar_t line[MAX_PATH];
        va_start( args, format );
        vswprintf(line, MAX_PATH, format, args);
        va_end( args );
        std::wstring wsline(line);
        std::string sline(wsline.begin(), wsline.end());
        writeDebugLine(std::string(wsline.begin(), wsline.end()));
        printf("%s", sline.c_str());
        fflush(stdout);
    }
    else
#endif
    {
        wchar_t line[MAX_PATH];
        va_start( args, format );
        vwprintf(format, args);
        va_end( args );
    }
}

DWORD crc32Hash(const void *data, DWORD size)
{
  if(crc32Intalized == false)
  {
    register DWORD crc;
    for(register DWORD i = 0; i < 256; i++)
    {
      crc = i;
      for(register DWORD j = 8; j > 0; j--)
      {
        if(crc & 0x1)crc = (crc >> 1) ^ 0xEDB88320L;
        else crc >>= 1;
      }
      crc32table[i] = crc;
    }
 
    crc32Intalized = true;
  }
 
  register DWORD cc = 0xFFFFFFFF;
  for(register DWORD i = 0; i < size; i++)cc = (cc >> 8) ^ crc32table[(((LPBYTE)data)[i] ^ cc) & 0xFF];
  return ~cc;
}

u32 u32_val(u8* buf) { return *(u32*)buf; }

int hex_to_int(char c){
        int first = c / 16 - 3;
        int second = c % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char c, char d)
{
        int high = hex_to_int(c) * 16;
        int low = hex_to_int(d);
        return high+low;
}

std::string hexStr_to_ascii(const char* hexStr)
{
    int length = strlen(hexStr);
    char buf = 0, buf2 = 0;
    std::string destStr = "";
    for(int i = 0; i < length; i++)
    {
        if(i % 2 != 0)
        {
            sprintf(&buf2, "%c", hex_to_ascii(buf, hexStr[i]));
            destStr.append(std::string(1, buf2));
        }
        else buf = hexStr[i];
    }
    return destStr;
}

unsigned random(unsigned n) {
/* n must be < RAND_MAX
 * returns a random number in range [0, n) */
  const int min_reject=RAND_MAX-RAND_MAX%n;
  int r;
  unsigned i=0;
  while ((r=rand()) >= min_reject && i<128) ++i;
  return r%n;
}
DWORD randomDWORD() {
  return DWORD(random(256))     | DWORD(random(256)<<8)
        |DWORD(random(256)<<16) | DWORD(random(256)<<24);
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(nullptr != fnIsWow64Process)
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    return bIsWow64;
}

std::vector<std::string> explode(std::string const & s, char delim)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(std::move(token));
    }

    return result;
}

static int ishex(char c) {
    if ('a' <= c && c <= 'f') return 1;
    if ('A' <= c && c <= 'F') return 1;
    if ('0' <= c && c <= '9') return 1;
    return 0;
}

static char hextoi(char c) {
    if ('a' <= c && c <= 'f') return c - 'a' + 0xA;
    if ('A' <= c && c <= 'F') return c - 'A' + 0xA;
    if ('0' <= c && c <= '9') return c - '0';
    return 0;
}

bool parse_hex_key(unsigned char *key, const char *hex, unsigned int len) {
    if (strlen(hex) != 2 * len) {
        return false;
    }

    for (unsigned int i = 0; i < 2 * len; i++) {
        if (!ishex(hex[i])) {
            return false;
        }
    }

    memset(key, 0, len);

    for (unsigned int i = 0; i < 2 * len; i++) {
        char val = hextoi(hex[i]);
        if ((i & 1) == 0) {
            val <<= 4;
        }
        key[i >> 1] |= val;
    }
}
