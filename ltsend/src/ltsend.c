#include <inttypes.h>
#include <stdbool.h>

#include <intrin.h>

#pragma warning(push, 0)
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma warning(pop)

#pragma comment(lib, "Ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wincrypt.h>
#include <strsafe.h>

#include <WinDNS.h>
#pragma comment(lib, "Dnsapi.lib")

//////////////////////////////////////////////////////////////////////////

#define NO_C_RUNTIME 1

#ifdef NO_C_RUNTIME
#pragma function(memset)
extern void *memset(void *pDst, int data, size_t size)
{
  size_t i = 0;
  uint8_t *const pDst8 = (uint8_t *const)pDst;
  const __m128i data128 = _mm_set1_epi8((char)data);

  if (size >= sizeof(__m128i))
    for (; i < size - (sizeof(__m128i) - 1); i += sizeof(__m128i))
      _mm_storeu_si128((__m128i *)(pDst8 + i), data128);

  for (; i < size; i++)
    pDst8[i] = (uint8_t)data;

  return pDst;
}

#pragma function(memcpy)
extern void *memcpy(void *pDst, const void *pSrc, size_t size)
{
  size_t i = 0;
  const uint8_t *const pSrc8 = (const uint8_t *)pSrc;
  uint8_t *const pDst8 = (uint8_t *)pDst;

  if (size >= sizeof(__m128i))
    for (; i < size - (sizeof(__m128i) - 1); i += sizeof(__m128i))
      _mm_storeu_si128((__m128i *)(pDst8 + i), _mm_loadu_si128((const __m128i *)(pSrc8 + i)));

  for (; i < size; i++)
    pDst8[i] = pSrc8[i];

  return pDst;
}

#pragma function(strlen)
extern size_t strlen(const char *text)
{
  if (text == NULL)
    return 0;

  size_t length = 0;

  while (text[length] != '\0')
    length++;

  return length;
}

wchar_t *wstrchr(wchar_t *text, const wchar_t t)
{
  if (text == NULL)
    return NULL;

  size_t i = 0;

  while (true)
  {
    if (text[i] == t)
      return text + i;

    if (text[i] == L'\0')
      return NULL;

    i++;
  }
}

char *strchr(char const *text, int t)
{
  if (text == NULL)
    return NULL;

  size_t i = 0;

  while (true)
  {
    if (text[i] == (char)t)
      return (char *)(text + i);

    if (text[i] == L'\0')
      return NULL;

    i++;
  }
}
#endif

//////////////////////////////////////////////////////////////////////////

#define STRINGIFY(value) #value
#define STRINGIFY_VALUE(value) STRINGIFY(value)

static bool _OpenLogFile();
__declspec(noreturn) static void _LogErrorAndQuit(const char *text, const size_t errorCode);
static void _Log(const char *text);

//////////////////////////////////////////////////////////////////////////

bool CpuExtensions_sseSupported = false;
bool CpuExtensions_sse2Supported = false;
bool CpuExtensions_sse3Supported = false;
bool CpuExtensions_ssse3Supported = false;
bool CpuExtensions_sse41Supported = false;
bool CpuExtensions_sse42Supported = false;
bool CpuExtensions_avxSupported = false;
bool CpuExtensions_avx2Supported = false;
bool CpuExtensions_fma3Supported = false;
bool CpuExtensions_aesNiSupported = false;

static bool CpuExtensions_simdFeaturesDetected = false;

void CpuExtensions_Detect()
{
  if (CpuExtensions_simdFeaturesDetected)
    return;

  int32_t info[4];
  __cpuid(info, 0);
  const int32_t idCount = info[0];

  if (idCount >= 0x1)
  {
    int32_t cpuInfo[4];
    __cpuid(cpuInfo, 1);

    const bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    const bool cpuAVXSuport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
      const uint64_t xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
      CpuExtensions_avxSupported = (xcrFeatureMask & 0x6) == 0x6;
    }

    CpuExtensions_sseSupported = (cpuInfo[3] & (1 << 25)) != 0;
    CpuExtensions_sse2Supported = (cpuInfo[3] & (1 << 26)) != 0;
    CpuExtensions_sse3Supported = (cpuInfo[2] & (1 << 0)) != 0;

    CpuExtensions_ssse3Supported = (cpuInfo[2] & (1 << 9)) != 0;
    CpuExtensions_sse41Supported = (cpuInfo[2] & (1 << 19)) != 0;
    CpuExtensions_sse42Supported = (cpuInfo[2] & (1 << 20)) != 0;
    CpuExtensions_fma3Supported = (cpuInfo[2] & (1 << 12)) != 0;
    CpuExtensions_aesNiSupported = (cpuInfo[2] & (1 << 25)) != 0;
  }

  if (idCount >= 0x7)
  {
    int32_t cpuInfo[4];
    __cpuid(cpuInfo, 7);

    CpuExtensions_avx2Supported = (cpuInfo[1] & (1 << 5)) != 0;
  }

  CpuExtensions_simdFeaturesDetected = true;
}

inline uint64_t _CurrentTimeNs()
{
  FILETIME ft;
  GetSystemTimePreciseAsFileTime(&ft);

  return ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

inline uint64_t _ReadRand()
{
  __declspec(align(16)) static uint64_t last[2];
  __declspec(align(16)) static uint64_t last2[2];
  static bool firstCall = true;

  if (firstCall)
  {
    firstCall = false;

    last[0] = _CurrentTimeNs();
    last[1] = __rdtsc();
    last2[0] = ~__rdtsc();
    last2[1] = ~_CurrentTimeNs();
  }

  if (!CpuExtensions_simdFeaturesDetected) // let's hope that's faster than the function call...
    CpuExtensions_Detect();

  // If we can use AES-NI, use aes decryption.
  if (CpuExtensions_aesNiSupported)
  {
    const __m128i a = _mm_load_si128((const __m128i *)(last));
    const __m128i b = _mm_load_si128((const __m128i *)(last2));

    const __m128i r = _mm_aesdec_si128(a, b);

    _mm_store_si128((__m128i *)(last), b);
    _mm_store_si128((__m128i *)(last2), r);

    return last[1] ^ last[0];
  }
  else
  {
    // This is simply PCG, which is about 25% slower, (we're talking ~5 ns/call, so it's certainly not slow) but works on all CPUs as a fallback option.

    const uint64_t oldstate_hi = last[0];
    const uint64_t oldstate_lo = oldstate_hi * 6364136223846793005ULL + (last[1] | 1);
    last[0] = oldstate_hi * 6364136223846793005ULL + (last[1] | 1);

    const uint32_t xorshifted_hi = (uint32_t)(((oldstate_hi >> 18) ^ oldstate_hi) >> 27);
    const uint32_t rot_hi = (uint32_t)(oldstate_hi >> 59);

    const uint32_t xorshifted_lo = (uint32_t)(((oldstate_lo >> 18) ^ oldstate_lo) >> 27);
    const uint32_t rot_lo = (uint32_t)(oldstate_lo >> 59);

    const uint32_t hi = (xorshifted_hi >> rot_hi) | (xorshifted_hi << (uint32_t)((-(int32_t)rot_hi) & 31));
    const uint32_t lo = (xorshifted_lo >> rot_lo) | (xorshifted_lo << (uint32_t)((-(int32_t)rot_lo) & 31));

    return ((uint64_t)hi << 32) | lo;
  }
}

inline void _BuildInitialSample(uint8_t *pBytes, const size_t count)
{
  HCRYPTPROV cryptoProvider = 0;

  if (!CryptAcquireContext(&cryptoProvider, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) || !CryptGenRandom(cryptoProvider, (DWORD)count, pBytes))
  {
    size_t i = 0;

    for (; i < count - 7; i += 8)
      *(uint64_t *)(pBytes + i) = _ReadRand();

    for (; i < count; i++)
      pBytes[i] = (uint8_t)_ReadRand();
  }

  if (cryptoProvider != 0)
    CryptReleaseContext(cryptoProvider, 0);
}

inline void _Increment(uint8_t *pBytes, const size_t count)
{
  size_t i = count;

  do
  {
    i--;
    pBytes[i]++;

  } while (i > 8 && pBytes[i] == 0);
}

extern void sha512_compress(const uint8_t block[128], uint64_t state[8]);

inline void _SHA512(const uint8_t *pData, size_t count, uint64_t hash[8])
{
  hash[0] = UINT64_C(0x6A09E667F3BCC908);
  hash[1] = UINT64_C(0xBB67AE8584CAA73B);
  hash[2] = UINT64_C(0x3C6EF372FE94F82B);
  hash[3] = UINT64_C(0xA54FF53A5F1D36F1);
  hash[4] = UINT64_C(0x510E527FADE682D1);
  hash[5] = UINT64_C(0x9B05688C2B3E6C1F);
  hash[6] = UINT64_C(0x1F83D9ABFB41BD6B);
  hash[7] = UINT64_C(0x5BE0CD19137E2179);

  size_t i = 0;

  for (i = 0; count - i >= 128; i += 128)
    sha512_compress(pData + i, hash);

  uint8_t block[128] = { 0 };
  size_t remaining = count - i;

  if (remaining > 0)
    memcpy(block, pData + i, remaining);

  block[remaining] = 0x80;
  remaining++;

  if (128 - remaining < 16)
  {
    sha512_compress(block, hash);
    memset(block, 0, sizeof(block));
  }

  block[128 - 1] = (uint8_t)((count & 0x1FU) << 3);
  count >>= 5;

  for (size_t j = 1; j < 16; j++, count >>= 8)
    block[128 - 1 - j] = (uint8_t)(count & 0xFFU);

  sha512_compress(block, hash);
}

void _ProofOfWork(OUT uint8_t *pSolution, const size_t solutionSize, const uint64_t startPattern, const uint32_t hashPattern)
{
  _BuildInitialSample(pSolution, solutionSize);

  *(uint64_t *)(pSolution) = startPattern;

  uint64_t hash[8];
  const uint32_t hashPattern6 = hashPattern & 0xFFFFFF;

  while (true)
  {
    _SHA512(pSolution, solutionSize, hash);

    if ((hash[1] & 0xFFFFFF) == hashPattern6)
      break;

    _Increment(pSolution, solutionSize);
  }
}

//////////////////////////////////////////////////////////////////////////

#define FAIL(reason) _LogErrorAndQuit("FAILURE: " reason " (" STRINGIFY_VALUE(__LINE__) ")\n\n", __LINE__)
#define RETURN_ERROR(reason) do { _Log("ERROR: " reason " (" STRINGIFY_VALUE(__LINE__) ")\n"); return false; } while (0)
#define LOG(reason) _Log("LOG: " reason "\n")
#define LOG_VALUE(value) do { _Log(value); _Log("\n"); } while (0)
#define LOG_VALUEW(value) do { _LogW(value); _Log("\n"); } while (0)

#ifdef NO_C_RUNTIME
#pragma comment(linker, "/entry:EntryPoint")
#else
DWORD CALLBACK EntryPoint();

int32_t WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return EntryPoint();
}
#endif

bool SendFile(const char *serverLocator, const uint8_t *pFile, const size_t length)
{
  if (serverLocator == NULL || pFile == NULL)
    return false;

  char serverIp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")];

  {
    bool isIp = true;

    size_t i = 0;

    while (serverLocator[i] != '\0')
    {
      if (!((serverLocator[i] >= '0' && serverLocator[i] <= '9') || serverLocator[i] == '.' || serverLocator[i] == ':') || i >= sizeof(serverIp))
      {
        isIp = false;
        break;
      }

      i++;
    }

    if (isIp)
    {
      memcpy(serverIp, serverLocator, i);
    }
    else
    {
      DNS_RECORDA *pDnsRecords = NULL;

      DNS_STATUS result = DnsQuery_A(serverLocator, DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pDnsRecords, NULL);

      DnsRecordListFree(pDnsRecords, DnsFreeRecordList);
    }
  }
}

bool HandleFile(HANDLE file)
{
  if (file == NULL || file == INVALID_HANDLE_VALUE)
    RETURN_ERROR("Invalid file handle.");

  LARGE_INTEGER size;

  if (!GetFileSizeEx(file, &size))
    RETURN_ERROR("Failed to retrieve file size from handle.");

  if (size.QuadPart > 1024 * 128)
    RETURN_ERROR("File size exceeds 128 kb.");
}

DWORD CALLBACK EntryPoint()
{
  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc == 0 || pArgv == NULL)
    FAIL("Failed to retrieve Args.");

  if (argc != 3)
    FAIL("Invalid Argument Count.");

  // Wait for Process to quit.
  {
    DWORD processId = 0;

    // Decode processId.
    {
      size_t i = 0;

      while (pArgv[1][i] != L'\0')
      {
        const wchar_t c = pArgv[1][i];
        uint32_t value = 0;

        if (c >= L'0' && c <= L'9')
          value = c - '0';
        else if (c >= 'A' && c <= 'F')
          value = c - 'A';
        else if (c >= 'a' && c <= 'f')
          value = c - 'a';
        else
          FAIL("Invalid Process Id.");

        processId = processId << 4 | value;
        i++;
      }
    }

    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processId);

    if (process != NULL)
    {
      WaitForSingleObject(process, INFINITE);
      
      // To ensure that writing the log file is finalized.
      Sleep(200);
    }
  }

  HANDLE mutex = CreateMutexExW()

  // Iterate Directory of Log Files.
  {
    WIN32_FIND_DATAW fileData;

    HANDLE handle = FindFirstFileExW(pArgv[2], FindExInfoBasic, &fileData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

    if (handle == NULL || handle == INVALID_HANDLE_VALUE)
      FAIL("Failed to Iterate Directory.");

    do
    {
      HANDLE file = CreateFileW(fileData.cFileName, GENERIC_READ, FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);

      if (file == NULL || file == INVALID_HANDLE_VALUE)
      {
        LOG("Failed to open file:");
        LOG_VALUEW(fileData.cFileName);
      }
      else
      {
        LOG("File:");
        LOG_VALUEW(fileData.cFileName);
      }

      if (HandleFile(file))
        DeleteFileW(fileData.cFileName);
      else
        LOG("Failed to handle file.");

    } while (FindNextFileW(handle, &fileData));
  }

  LOG("== END ==");

  return 0;
}

//////////////////////////////////////////////////////////////////////////

HANDLE _LogFile = NULL;

static bool _OpenLogFile()
{
  if (_LogFile != NULL)
    return _LogFile != INVALID_HANDLE_VALUE;

  char path[MAX_PATH] = "";

  // Prepare directory.
  {
    const char *defaultFolderPath = "%AppData%\\lt_logs\\_ltsend.log";
    const char *folderPath = defaultFolderPath;

    if (0 == ExpandEnvironmentStringsA(folderPath, path, ARRAYSIZE(path)))
      return false;

    size_t length = 0;

    if (FAILED(StringCchLengthA(path, ARRAYSIZE(path), &length)))
      return false;

    if (length >= ARRAYSIZE(path))
      return false;
  }

  // Create Directory if missing.
  {
    char folder[ARRAYSIZE(path)];
    char *end = NULL;
    memset(folder, 0, sizeof(folder));

    end = strchr(path, '\\');

    while (end != NULL)
    {
      StringCchCopyA(folder, end - path + 1, path);

      if (!CreateDirectoryA(folder, NULL))
      {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
          // this should result in an error later (when we're trying to create the file).
        }
      }

      end++;
      end = strchr(end, '\\');
    }
  }

  _LogFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

  if (_LogFile == INVALID_HANDLE_VALUE)
    return false;

  if (GetLastError() == ERROR_ALREADY_EXISTS)
    SetFilePointer(_LogFile, 0, 0, FILE_END);

  SYSTEMTIME time;
  GetSystemTime(&time);

#ifndef GIT_BUILD
#define GIT_BUILD 0
#endif

#ifndef GIT_REF
#define GIT_REF "<DEV_BUILD>"
#endif

  char buffer[] = "\n== LOG START ==\nVersion " STRINGIFY_VALUE(GIT_BUILD) " (SHA: " GIT_REF " / " __DATE__ " " __TIME__ ")\n";

  WriteFile(_LogFile, buffer, (DWORD)sizeof(buffer) - 1, NULL, NULL);

  return true;
}

__declspec(noreturn) static void _LogErrorAndQuit(const char *text, const size_t errorCode)
{
  _Log(text);

  ExitProcess((UINT)errorCode);
}

static void _Log(const char *text)
{
  if (text && _OpenLogFile())
    WriteFile(_LogFile, text, (DWORD)strlen(text), NULL, NULL);
}

static void _LogW(const wchar_t *text)
{
  if (text && _OpenLogFile())
  {
    char utf8[1024 * 8];
    const int32_t count = WideCharToMultiByte(CP_UTF8, 0, text, lstrlenW(text) + 1, utf8, sizeof(utf8), NULL, false);

    if (count > 0)
      WriteFile(_LogFile, utf8, count, NULL, NULL);
  }
}
