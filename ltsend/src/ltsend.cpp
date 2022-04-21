#include <inttypes.h>
#include <stdio.h>

#include <windows.h>
#include <wincrypt.h>

#include <time.h>

void _BuildInitialSample(uint8_t *pBytes, const size_t count)
{
  HCRYPTPROV cryptoProvider = 0;

  if (!CryptAcquireContext(&cryptoProvider, nullptr, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) || !CryptGenRandom(cryptoProvider, (DWORD)count, pBytes))
  {
    // TODO: Build Random Data Ourselves.
    __debugbreak();
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

  } while (i != 0 && pBytes[i] == 0);
}

extern "C" void sha512_compress(const uint8_t block[128], uint64_t state[8]);

__declspec(noinline) void _SHA512(const uint8_t *pData, size_t count, uint64_t hash[8])
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
    sha512_compress(&pData[i], hash);

  uint8_t block[128] = { 0 };
  size_t remaining = count - i;

  if (remaining > 0)
    memcpy(block, &pData[i], remaining);

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

static size_t _Tries = 0;

int main(void)
{
  uint8_t sample[1024];
  _BuildInitialSample(sample, sizeof(sample));

  uint64_t hash[8];

  const clock_t before = clock();

  while (true)
  {
    _Tries++;
    _SHA512(sample, sizeof(sample), hash);

    if ((hash[0] & 0xFFFFF) == 0x12345)
      break;

    _Increment(sample, sizeof(sample));
  }

  const clock_t after = clock();

  printf("Cracked in %f s (%" PRIu64 " tries -> %f MB/s | %f tries / bit).\n\n", (double)(after - before) / (double)(CLOCKS_PER_SEC), _Tries, (((double)_Tries * sizeof(sample)) / (1024.0 * 1024.0)) / ((double)(after - before) / (double)(CLOCKS_PER_SEC)), _Tries / (double)0xFFFFF);

  for (size_t i = 0; i < sizeof(sample); i += 32)
  {
    for (size_t j = 0; j < 32; j++)
      printf("%02" PRIX8 " ", sample[i + j]);

    puts("");
  }

  puts("\nHashes to:");

  for (size_t i = 0; i < sizeof(hash) / sizeof(hash[0]); i++)
    printf("%08" PRIx64 "", hash[i]);

  return 0;
}
