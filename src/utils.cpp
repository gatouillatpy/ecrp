
#include <string>

#include "utils.h"

//----------------------------------------------------------------------

namespace geodis
{
	byte hex2byte(char c)
	{
		const char np = '0';
		const char nq = '9';
		const char lp = 'a';
		const char lq = 'f';
		const char up = 'A';
		const char uq = 'F';

		if (c >= np && c <= nq)
			return c - np;
		else if (c >= lp && c <= lq)
			return c - lp + 10;
		else if (c >= up && c <= uq)
			return c - up + 10;
		else
			return 0xFF;
	}

	char byte2hex(byte b)
	{
		if (b >= 0 && b <= 9)
			return '0' + b;
		else if (b >= 10 && b <= 16)
			return 'a' + b - 10;
		else
			return '?';
	}

	uint64_t retrieveKey(const std::string& key)
	{
		byte b[8];

		int k = 0;
		int n = key.length() - 16;

		for (int i = 0; i < 8; i++)
		{
			byte p = k + n >= 0 ? hex2byte(key.at(k + n)) : 0; k++;

			if (p != 0xFF)
				b[i] = p << 4;
			else
				return 0;

			byte q = k + n >= 0 ? hex2byte(key.at(k + n)) : 0; k++;

			if (q != 0xFF)
				b[i] |= q;
			else
				return 0;
		}

		return *(uint64_t*)b;
	}

	std::string formatKey(uint64_t key)
	{
		byte b[8];
		*(uint64_t*)b = key;

		int k = 0;
		char c[17];

		for (int i = 0; i < 8; i++)
		{
			c[k] = byte2hex(b[i] >> 4); k++;
			c[k] = byte2hex(b[i] & 0x0F); k++;
		}

		c[k] = '\0';

		return c;
	}
}
