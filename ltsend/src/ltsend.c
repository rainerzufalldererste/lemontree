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
#include <intrin.h>

#pragma warning(push, 0)
#include <winternl.h>
#pragma warning(pop)

#include <WinDNS.h>
#pragma comment(lib, "Dnsapi.lib")

#include "monocypher.h"
#include "monocypher.c"

//////////////////////////////////////////////////////////////////////////

enum
{
  lt_version = 0x10000001,
  lt_t_start = 0,
};

//#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////

#define NO_C_RUNTIME 1

#ifdef NO_C_RUNTIME
// This is stupid, but sadly msvc seems to sometimes generate calls to `__chkstk` even without the c runtime.
void __chkstk()
{

}

#pragma function(memset)
extern void *memset(void *pDst, int data, size_t size)
{
  size_t i = 0;
  uint8_t *const pDst8 = (uint8_t *const)pDst;
  const __m128i data128 = _mm_set1_epi8((char)data);

  if (size >= sizeof(__m128i))
    for (; i + sizeof(__m128i) >= size; i += sizeof(__m128i))
      _mm_storeu_si128((__m128i *)(pDst8 + i), data128);

  for (; i < size; i++)
    pDst8[i] = (uint8_t)data;

  return pDst;
}

#pragma function(memcpy)
extern void *memcpy(void *pDst, const void *pSrc, size_t size)
{
  size_t i = 0;
  const uint8_t *const pSrc8 = (const uint8_t *const)pSrc;
  uint8_t * const pDst8 = (uint8_t *const)pDst;

  if (size >= sizeof(__m128i))
    for (; i + sizeof(__m128i) >= size; i += sizeof(__m128i))
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

extern size_t strnlen(const char *text, const size_t maxCapacity)
{
  if (text == NULL || maxCapacity == 0)
    return 0;

  size_t length = 0;

  while (text[length] != '\0')
  {
    length++;

    if (length >= maxCapacity)
      return 0;
  }

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

HANDLE _GetHeap()
{
  static HANDLE heap = NULL;

  if (heap == NULL)
  {
    heap = GetProcessHeap();

    if (heap == NULL)
      heap = HeapCreate(0, 0, 0);
  }

  return heap;
}

void *malloc(const size_t size)
{
  HANDLE heap = _GetHeap();

  if (heap == NULL)
    return NULL;

  return HeapAlloc(heap, 0, size);
}

void *realloc(void *pData, const size_t size)
{
  HANDLE heap = _GetHeap();

  if (heap == NULL)
    return NULL;

  return HeapReAlloc(heap, 0, pData, size);
}

void free(void *pData)
{
  HANDLE heap = _GetHeap();

  if (heap == NULL)
    return;

  HeapFree(heap, 0, pData);
}
#endif

//////////////////////////////////////////////////////////////////////////

#define STRINGIFY(value) #value
#define STRINGIFY_VALUE(value) STRINGIFY(value)

static bool _OpenLogFile();
__declspec(noreturn) static void _LogErrorAndQuit(const char *text, const size_t errorCode);
static void _Log(const char *text);
static void _LogW(const wchar_t *text);
void _SetupSignalHandler();

//////////////////////////////////////////////////////////////////////////

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

    CpuExtensions_aesNiSupported = (cpuInfo[2] & (1 << 25)) != 0;
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

void ReadRandomBytes(uint8_t *pBytes, const size_t count)
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
  size_t i = count / sizeof(uint64_t);
  uint64_t *pData = (uint64_t *)pBytes;

  do
  {
    i--;
    pData[i]++;

  } while (i > 1 && pData[i] == 0);
}

void SolveChallenge(OUT uint8_t *pSolution, const size_t solutionSize, const uint64_t startPattern, const uint32_t hashPattern)
{
  ReadRandomBytes(pSolution, solutionSize);

  *(uint64_t *)pSolution = startPattern;

  uint8_t hash[64];
  const uint32_t bitPattern = 0xFFFF3F;
  const uint32_t hashPattern6 = hashPattern & bitPattern;

  while (true)
  {
    // Hash `sample` to `hash`.
    {
      crypto_blake2b_ctx ctx;

      crypto_blake2b_init(&ctx);
      crypto_blake2b_update(&ctx, pSolution, solutionSize);
      crypto_blake2b_final(&ctx, hash);
    }

    // Check if the solution is valid.
    if ((*(uint32_t *)(hash + 8) & bitPattern) == hashPattern6)
      break;

    // Increment the solution data.
    _Increment(pSolution, solutionSize);
  }
}

//////////////////////////////////////////////////////////////////////////

#define FAIL(reason) _LogErrorAndQuit("FAILURE: " reason " (" STRINGIFY_VALUE(__LINE__) ")\n\n", __LINE__)
#define RETURN_ERROR(reason) do { _Log("ERROR: " reason " (" STRINGIFY_VALUE(__LINE__) ")\n"); return false; } while (0)
#define LOG(reason) _Log("LOG: " reason "\n")
#define LOG_VALUE(value) do { _Log(value); _Log("\n"); } while (0)
#define LOG_VALUEW(value) do { _LogW(value); _Log("\n"); } while (0)
#define LOG_DESCRIPTION_VALUE(description, value) do { _Log("LOG: " description); _Log(value); _Log("\n"); } while (0)
#define LOG_DESCRIPTION_VALUEW(description, value) do { _Log("LOG: " description); _LogW(value); _Log("\n"); } while (0)

#ifdef NO_C_RUNTIME
#pragma comment(linker, "/entry:EntryPoint")
#else
DWORD CALLBACK EntryPoint();

int main(void)
{
  return EntryPoint();
}

int32_t WinMain(HINSTANCE _0, HINSTANCE _1, LPSTR _2, int _3)
{
  (void)_0;
  (void)_1;
  (void)_2;
  (void)_3;

  return EntryPoint();
}
#endif

bool ResolveHostNameToIP(const char *serverLocator, OUT char ip[40])
{
  // Resolve Hostname / Host-IP to `ip`.
  bool isIp = true;

  size_t serverLocatorLength = 0;

  while (serverLocator[serverLocatorLength] != '\0')
  {
    if (!((serverLocator[serverLocatorLength] >= '0' && serverLocator[serverLocatorLength] <= '9') || serverLocator[serverLocatorLength] == '.' || serverLocator[serverLocatorLength] == ':') || serverLocatorLength + 1 >= sizeof(ip))
    {
      isIp = false;
      break;
    }

    serverLocatorLength++;
  }

  if (isIp)
  {
    memcpy(ip, serverLocator, serverLocatorLength + 1);
  }
  else
  {
    LOG_DESCRIPTION_VALUE("Resolving Hostname: ", serverLocator);

    DNS_RECORD *pDnsRecords = NULL;

    DNS_STATUS result = DnsQuery_UTF8(serverLocator, DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pDnsRecords, NULL);

    if (result == 0 && pDnsRecords != NULL && pDnsRecords[0].wType == DNS_TYPE_A)
    {
      size_t position = 0;

      // Stringify IPv4.
      for (size_t i = 0; i < 4; i++)
      {
        uint8_t value = ((uint8_t *)&pDnsRecords[0].Data.A)[i];
        size_t pos = 0;
        char num[3];

        do
        {
          num[pos++] = '0' + value % 10;
          value /= 10;
        } while (value != 0);

        do
        {
          ip[position++] = num[--pos];
        } while (pos != 0);

        if (i < 3)
          ip[position] = '.';
        else
          ip[position] = '\0';

        position++;
      }

      DnsRecordListFree(pDnsRecords, DnsFreeRecordList);
    }
    else
    {
      if (pDnsRecords != NULL)
      {
        DnsRecordListFree(pDnsRecords, DnsFreeRecordList);
        pDnsRecords = NULL;
      }

      result = DnsQuery_UTF8(serverLocator, DNS_TYPE_AAAA, DNS_QUERY_STANDARD, NULL, &pDnsRecords, NULL);

      if (result != 0 || pDnsRecords == NULL || pDnsRecords[0].wType != DNS_TYPE_AAAA)
      {
        DnsRecordListFree(pDnsRecords, DnsFreeRecordList);

        RETURN_ERROR("Failed to retrieve an IP for hostname.");
      }

      // Stringify IPv6.
      {
        const char lut[] = "0123456789abcdef";
        size_t position = 0;

        for (size_t i = 0; i < 8; i++)
        {
          uint16_t value = ((uint16_t)pDnsRecords[0].Data.AAAA.Ip6Address.IP6Byte[i * 2] << 8) | pDnsRecords[0].Data.AAAA.Ip6Address.IP6Byte[i * 2 + 1];
          size_t pos = 0;
          char num[4];

          do
          {
            num[pos++] = lut[value & 0xF];
            value >>= 4;
          } while (value != 0);

          do
          {
            ip[position++] = num[--pos];
          } while (pos != 0);

          if (i < 7)
            ip[position] = ':';
          else
            ip[position] = '\0';

          position++;
        }
      }

      DnsRecordListFree(pDnsRecords, DnsFreeRecordList);
    }

    LOG_DESCRIPTION_VALUE("Resoled Hostname To: ", ip);
  }

  return true;
}

static size_t _WinSock_References = 0;
static WSADATA _WinSock_Info;

bool WinSock_AddReference()
{
  if (_WinSock_References > 0)
    return true;

  const int32_t result = WSAStartup(MAKEWORD(2, 2), &_WinSock_Info);

  if (result != 0)
    RETURN_ERROR("Failed to initialize WinSock.");

  _WinSock_References++;

  return true;
}

void WinSock_RemoveReference()
{
  if (_WinSock_References == 0)
    return;

  _WinSock_References--;

  if (_WinSock_References == 0)
    WSACleanup(); // yes, this can theoretically fail in multithreaded scenarios yada yada.
}

void DestroySocket(SOCKET *pSocket)
{
  if (*pSocket != INVALID_SOCKET)
  {
    shutdown(*pSocket, SD_SEND);
    closesocket(*pSocket);

    *pSocket = INVALID_SOCKET;
  }
}

bool Connect(const char *serverIp, OUT SOCKET *pSocket)
{
  if (!WinSock_AddReference())
    RETURN_ERROR("Failed to initialize WinSock.");

  struct addrinfo *pResult = NULL;
  struct addrinfo hints = { 0 };

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  const char port[] = "11793";

  int32_t error = getaddrinfo(serverIp, port, &hints, &pResult);

  if (error != 0)
    RETURN_ERROR("Failed to get address info for server ip.");

  SOCKET socketHandle = socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);

  if (socketHandle == INVALID_SOCKET)
  {
    error = WSAGetLastError();
    freeaddrinfo(pResult);
    RETURN_ERROR("Failed to create socket.");
  }

  const DWORD timeout = 1000 * 15;

  // It's not too horrible if these fail.
  error = setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
  error = setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));

  error = connect(socketHandle, pResult->ai_addr, (int32_t)pResult->ai_addrlen);

  if (error == SOCKET_ERROR)
  {
    error = WSAGetLastError();
    freeaddrinfo(pResult);
    DestroySocket(&socketHandle);
    RETURN_ERROR("Failed to connect to endpoint.");
  }

  *pSocket = socketHandle;

  return true;
}

bool Send(IN SOCKET *pSocket, const uint8_t *pData, const size_t length)
{
  if (pSocket == NULL || *pSocket == INVALID_SOCKET || pData == NULL)
    RETURN_ERROR("Invalid Argument.");

  if (length > INT32_MAX)
    RETURN_ERROR("Invalid Size.");

  const int32_t bytesSent = send(*pSocket, (const char *)pData, (int32_t)length, 0);

  if (bytesSent == SOCKET_ERROR || bytesSent < 0)
  {
    const int32_t error = WSAGetLastError();
    (void)error;

    RETURN_ERROR("Failed to send data.");
  }

  if ((size_t)bytesSent != length)
    RETURN_ERROR("Failed to transmit all bytes.");

  return true;
}

bool Receive(IN SOCKET *pSocket, OUT uint8_t *pData, const size_t capacity, OUT size_t *pBytesReceived)
{
  if (pSocket == NULL || *pSocket == INVALID_SOCKET || pData == NULL || pBytesReceived == NULL)
    RETURN_ERROR("Invalid Argument.");

  const int32_t bytesReceived = recv(*pSocket, (char *)pData, (int32_t)min((size_t)INT32_MAX, capacity), 0);

  if (bytesReceived < 0 || bytesReceived == SOCKET_ERROR)
  {
    const int32_t error = WSAGetLastError();
    (void)error;

    *pBytesReceived = 0;
    RETURN_ERROR("Failed to receive data.");
  }
  else
  {
    *pBytesReceived = (size_t)bytesReceived; // if 0 that's the end of the stream.
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////

bool TransferToServer(const char *serverLocator, const char productName[0x100], uint8_t *pData, const size_t dataSize)
{
  if (serverLocator == NULL || productName == NULL || pData == NULL)
    RETURN_ERROR("Argument Null.");

  if (serverLocator[0] == '\0')
    RETURN_ERROR("Invalid Server Locator.");
 
  char serverIp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")];
 
  if (!ResolveHostNameToIP(serverLocator, serverIp))
    RETURN_ERROR("Failed to resolve hostname to ip.");
 
  SOCKET socketHandle = INVALID_SOCKET;

  if (!Connect(serverIp, &socketHandle))
    RETURN_ERROR("Failed to establish connection.");

  // Handshake Step 1: Send Product Name.
  if (!Send(&socketHandle, (const uint8_t *)productName, strnlen(productName, 0x100)))
  {
    DestroySocket(&socketHandle);
    RETURN_ERROR("Failed to establish handshake.");
  }

  uint8_t challenge[8 + 3];

  // Handshake Step 2: Get Challenge.
  {
    size_t bytesReceived = 0;
  
    if (!Receive(&socketHandle, challenge, sizeof(challenge), &bytesReceived) || bytesReceived != sizeof(challenge))
    {
      DestroySocket(&socketHandle);
      RETURN_ERROR("Failed to receive challenge data.");
    }
  }

  uint8_t solution[1024];

  // Solve the Challenge.
  {
    SolveChallenge(solution, sizeof(solution), *(uint64_t *)challenge, ((uint32_t)challenge[8] << 16) | ((uint32_t)challenge[9] << 8) | challenge[10]);
  }

  // Handshake Step 3: Send the Solution.
  if (!Send(&socketHandle, solution + 8, sizeof(solution) - 8)) // first 8 bytes should be the challenge part 1 anyways.
  {
    DestroySocket(&socketHandle);
    RETURN_ERROR("Failed to send solution.");
  }

  uint8_t mac[16];

  // Handshake Step 4, 5: Send Client Public Key (and Encrypt Data), then: Send MAC.
  {
    const uint8_t serverPublicKey[32] =
    {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // your server public key goes here.
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    uint8_t sharedSecret[32];
    uint8_t privateKey[32];
    uint8_t publicKey[32];
    
    // Generate Public Key & Shared Secret.
    {
      ReadRandomBytes(privateKey, sizeof(privateKey));

      crypto_x25519_public_key(publicKey, privateKey); // generate public key.
      crypto_x25519(sharedSecret, privateKey, serverPublicKey); // generate shared secret.

      ZeroMemory(privateKey, sizeof(privateKey)); // we no longer need this.
      MemoryBarrier();

      if (!Send(&socketHandle, publicKey, sizeof(publicKey)))
      {
        ZeroMemory(sharedSecret, sizeof(sharedSecret));
        DestroySocket(&socketHandle);
        RETURN_ERROR("Failed to negotiate secure connection.");
      }
    }

    uint8_t sessionKeys[64];
    
    // Hash keys & shared secret to `sessionKeys`.
    {
      crypto_blake2b_ctx ctx;

      crypto_blake2b_init(&ctx);
      crypto_blake2b_update(&ctx, sharedSecret, 32);
      crypto_blake2b_update(&ctx, publicKey, 32);
      crypto_blake2b_update(&ctx, serverPublicKey, 32);
      crypto_blake2b_final(&ctx, sessionKeys);

      ZeroMemory(sharedSecret, sizeof(sharedSecret));
      MemoryBarrier();
    }

    crypto_lock(mac, pData, sessionKeys, sessionKeys + 32, pData, dataSize); // yes, we're inplace encrypting `pData`. that is perfectly fine.

    ZeroMemory(sessionKeys, sizeof(sessionKeys));
    MemoryBarrier();
  }

  bool retried = false;
retry:

  // Send the MAC of the encrypted file contents.
  if (!Send(&socketHandle, mac, sizeof(mac)))
  {
    DestroySocket(&socketHandle);
    RETURN_ERROR("Failed to send file authentication.");
  }

  // Send the (now encrypted) actual file contents.
  if (!Send(&socketHandle, pData, dataSize))
  {
    DestroySocket(&socketHandle);
    RETURN_ERROR("Failed to transmit telemetry data.");
  }

  // Wait for Server OK (before we're potentially deleting the file already and the server didn't actually receive it correctly).
  {
    uint8_t ok[2];
    size_t bytesReceived = 0;

    if (!Receive(&socketHandle, ok, sizeof(ok), &bytesReceived) || bytesReceived != sizeof(ok))
    {
      DestroySocket(&socketHandle);
      RETURN_ERROR("Failed to receive server acknowledgement.");
    }

    if (!retried && ok[0] == 'R' || ok[1] == 'E')
    {
      retried = true;
      LOG("Server requested retry. Retrying transmission.");
      goto retry;
    }
    else if (ok[0] != 'O' || ok[1] != 'K')
    {
      DestroySocket(&socketHandle);
      RETURN_ERROR("Receive invalid server acknowledgement.");
    }
  }

  DestroySocket(&socketHandle);

  return true;
}

bool ParseFileInfo(const uint8_t *pContents, const size_t bytesRead, OUT char productName[0x100], OUT char hostname[0x100], OUT bool *pIsDebugBuild)
{
  size_t position = 0;

  if (bytesRead < position + sizeof(uint8_t) || pContents[position] != lt_t_start) // `lt_t_start`
    RETURN_ERROR("Invalid file type.");

  position += sizeof(uint8_t);

  if (bytesRead < position + sizeof(uint32_t) || *(uint32_t *)(pContents + position) > lt_version) // `lt_version`.
    RETURN_ERROR("Incompatible file version.");

  position += sizeof(uint32_t);
  position += sizeof(uint64_t); // timestamp.

  if (bytesRead < position + sizeof(uint8_t))
    RETURN_ERROR("Insufficient file contents");

  const uint8_t productNameLength = pContents[position];

  position += sizeof(uint8_t); // productNameLength.

  if (bytesRead < position + productNameLength)
    RETURN_ERROR("Insufficient file contents");

  memcpy(productName, pContents + position, productNameLength);
  productName[productNameLength] = '\0';

  position += productNameLength; // productName.
  position += sizeof(uint64_t) * 2; // majorVersion, minorVersion.

  if (bytesRead < position + sizeof(uint8_t))
    RETURN_ERROR("Insufficient file contents");

  *pIsDebugBuild = pContents[position] != 0;

  position += sizeof(uint8_t); // isDebugBuild.

  if (bytesRead < position + sizeof(uint8_t))
    RETURN_ERROR("Insufficient file contents");

  const uint8_t remoteHostLength = pContents[position];

  position += sizeof(uint8_t); // remoteHostLength.

  if (bytesRead < position + remoteHostLength)
    RETURN_ERROR("Insufficient file contents");

  memcpy(hostname, pContents + position, remoteHostLength);
  hostname[remoteHostLength] = '\0';

  position += remoteHostLength; // remoteHost.

  return true;
}

bool HandleFile(HANDLE file)
{
  if (file == NULL || file == INVALID_HANDLE_VALUE)
    RETURN_ERROR("Invalid file handle.");

  LARGE_INTEGER size;

  if (!GetFileSizeEx(file, &size))
    RETURN_ERROR("Failed to retrieve file size from handle.");

  if ((size_t)size.QuadPart > 1024 * 256) // this has been increased from 128.
    RETURN_ERROR("File size exceeds 256 kb.");

  uint8_t *pContents = malloc(size.QuadPart);

  if (pContents == NULL)
    RETURN_ERROR("Failed to allocate memory.");

  DWORD bytesRead = 0;

  if (TRUE != ReadFile(file, pContents, size.LowPart /* Should be sufficient, because we're reading 1024 * 256  bytes max */, &bytesRead, NULL))
  {
    free(pContents);
    RETURN_ERROR("Failed to read file contents.");
  }

  if (bytesRead != size.QuadPart)
  {
    free(pContents);
    RETURN_ERROR("File could only be read partially.");
  }

  char productName[0x100] = "";
  char hostname[0x100] = "";
  bool isDebugBuild = false;

  if (!ParseFileInfo(pContents, size.QuadPart, productName, hostname, &isDebugBuild))
  {
    free(pContents);
    RETURN_ERROR("Failed to parse file contents.");
  }

  LOG_DESCRIPTION_VALUE("Product Name: ", productName);

  if (isDebugBuild)
  {
    free(pContents);
    RETURN_ERROR("The log file is from a debug build.");
  }

  if (!TransferToServer(hostname, productName, pContents, bytesRead))
  {
    free(pContents);
    RETURN_ERROR("Failed to send file contents.");
  }

  free(pContents);

  return true;
}

DWORD CALLBACK EntryPoint()
{
  _SetupSignalHandler();
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (argc == 0 || pArgv == NULL)
    FAIL("Failed to retrieve Args.");

  if (argc != 2)
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
      Sleep(2000);
    }
  }

  HANDLE mutex = CreateMutexW(NULL, TRUE, TEXT("Global_lemontree_ltsend"));

  if (mutex == NULL || mutex == INVALID_HANDLE_VALUE)
    FAIL("Failed to create global mutex.");

  if (GetLastError() == ERROR_ALREADY_EXISTS)
    if (WAIT_OBJECT_0 != WaitForSingleObject(mutex, INFINITE))
      FAIL("Failed to obtain global mutex.");

  // Iterate Directory of Log Files.
  {
    WIN32_FIND_DATAW fileData;

    HANDLE handle = FindFirstFileExW(TEXT("*"), FindExInfoBasic, &fileData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

    if (handle == NULL || handle == INVALID_HANDLE_VALUE)
    {
      ReleaseMutex(mutex);

      FAIL("Failed to Iterate Directory.");
    }

    do
    {
      if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        continue;

      HANDLE file = CreateFileW(fileData.cFileName, GENERIC_READ, FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, 0, NULL);

      if (file == NULL || file == INVALID_HANDLE_VALUE)
      {
        LOG_DESCRIPTION_VALUEW("Failed to open file: ", fileData.cFileName);
        continue;
      }
      else
      {
        LOG_DESCRIPTION_VALUEW("File: ", fileData.cFileName);
      }

      if (HandleFile(file))
        DeleteFileW(fileData.cFileName);
      else
        LOG("Failed to handle file.");

      CloseHandle(file);

    } while (FindNextFileW(handle, &fileData));
  }

  ReleaseMutex(mutex);

  LOG_VALUE("\n== END ==");

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
      WriteFile(_LogFile, utf8, count - 1, NULL, NULL);
  }
}

//////////////////////////////////////////////////////////////////////////

void _LogHex(const uint64_t value)
{
  static const char lut[] = "0123456789ABCDEF";
  char param[sizeof("FFFFFFFFFFFFFFFF")];
  size_t index = 0;
  uint64_t val = value;

  // Write process id as hex to `param` (reverse).
  while (val != 0)
  {
    param[index] = lut[val & 0xF];
    val >>= 4;
    index++;
  }

  param[index] = '\0';
  index--;

  size_t start = 0;

  // Reverse the reversed number in `param`.
  while (start < index)
  {
    const char t = param[start];
    param[start] = param[index];
    param[index] = t;

    start++;
    index--;
  }

  _Log(param);
}

BOOL WINAPI _SignalHandler(DWORD type)
{
  _Log("[CRITICAL] Signal raised: 0x");
  _LogHex(type);
  _Log(".\n");

  return TRUE;
}

LONG WINAPI TopLevelExceptionHandler(IN EXCEPTION_POINTERS *pExceptionInfo)
{
  _Log("[CRITICAL] Exception raised: 0x");
  _LogHex(pExceptionInfo->ExceptionRecord->ExceptionCode);

  PEB *pProcessEnvironmentBlock = (PEB *)__readgsqword(0x60);
  LDR_DATA_TABLE_ENTRY *pApplicationModuleEntry = CONTAINING_RECORD(pProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

  _Log(" at 0x");
  _LogHex((uint64_t)pExceptionInfo->ExceptionRecord->ExceptionAddress - (uint64_t)pApplicationModuleEntry->DllBase);
  _Log(".\n");

  return EXCEPTION_CONTINUE_SEARCH;
}

void _SetupSignalHandler()
{
  SetUnhandledExceptionFilter(TopLevelExceptionHandler);

  if (0 == SetConsoleCtrlHandler(_SignalHandler, TRUE))
    LOG("Failed to set ConsoleCrtHandler.");
}
