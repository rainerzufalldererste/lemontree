#include "lta_types.h"

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(FILE *pFile, const char *string)
{
  if (string == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

  char singleChar[2];
  singleChar[1] = '\0';

  size_t i = 0;

  while (true)
  {
    singleChar[0] = string[i++];

    if (singleChar[0] == '\0')
      break;

    switch (singleChar[0])
    {
    case '\\': fputs("\\\\", pFile); break;
    case '\"': fputs("\\\"", pFile); break;
    case '\b': fputs("\\b", pFile); break;
    case '\f': fputs("\\f", pFile); break;
    case '\n': fputs("\\n", pFile); break;
    case '\r': fputs("\\r", pFile); break;
    case '\t': fputs("\\t", pFile); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        fprintf(pFile, "\\u00%02" PRIX8, singleChar[0]);
      else
        fputs(singleChar, pFile);

      break;
    }
    }
  }

  fputs("\"", pFile);
}

void print_string_as_json(FILE *pFile, const wchar_t *string)
{
  if (string == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

  wchar_t singleChar[2];
  singleChar[1] = L'\0';

  size_t i = 0;

  while (true)
  {
    singleChar[0] = string[i++];

    if (singleChar[0] == L'\0')
      break;

    switch (singleChar[0])
    {
    case L'\\': fputs("\\\\", pFile); break;
    case L'\"': fputs("\\\"", pFile); break;
    case L'\b': fputs("\\b", pFile); break;
    case L'\f': fputs("\\f", pFile); break;
    case L'\n': fputs("\\n", pFile); break;
    case L'\r': fputs("\\r", pFile); break;
    case L'\t': fputs("\\t", pFile); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        fprintf(pFile, "\\u00%02" PRIX8, singleChar[0]);
      else
        fputws(singleChar, pFile);

      break;
    }
    }
  }

  fputs("\"", pFile);
}

void print_bytes_as_base64string(FILE *pFile, const uint8_t *pData, const size_t size)
{
  if (pData == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

  // Encode to Base64.
  {
    const char lut[] =
    {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '+', '/'
    };

    char next[5];
    next[4] = '\0';

    for (size_t i = 0; i < size; i += 3)
    {
      uint32_t data[3];

      data[0] = (i + 0) < size ? pData[i] : 0;
      data[1] = (i + 1) < size ? pData[i + 1] : 0;
      data[2] = (i + 2) < size ? pData[i + 2] : 0;

      const uint32_t triple = (data[0] << 0x10) + (data[1] << 0x08) + data[2];

      next[0] = lut[(triple >> 18) & 0x3F];
      next[1] = lut[(triple >> 12) & 0x3F];
      next[2] = lut[(triple >> 6) & 0x3F];
      next[3] = lut[triple & 0x3F];

      fputs(next, pFile);
    }
  }

  // Add Padding.
  {
    const size_t bytesMod3 = size % 3;

    if (bytesMod3 != 0)
      for (size_t i = 0; i < 3 - bytesMod3; i++)
        fputs("=", pFile);
  }

  fputs("\"", pFile);
}
