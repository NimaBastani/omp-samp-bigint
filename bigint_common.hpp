/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, AmyrAhmady.
 */
 #pragma once

#include <amx/amx.h>
#include <cstdint>
#include <limits>
#include <string>

#ifndef AMX_NATIVE_CALL
#define AMX_NATIVE_CALL
#endif

static inline cell* GetBigIntPtr(AMX* amx, cell param)
{
	cell* addr = nullptr;
	amx_GetAddr(amx, param, &addr);
	return addr;
}

static inline int64_t BigInt_Read(cell* p)
{
	uint32_t lo = static_cast<uint32_t>(p[0]);
	int32_t hi = static_cast<int32_t>(p[1]);
	return (static_cast<int64_t>(hi) << 32) | lo;
}

static inline void BigInt_Write(cell* p, int64_t v)
{
	uint32_t lo = static_cast<uint32_t>(v & 0xFFFFFFFFLL);
	int32_t hi = static_cast<int32_t>(v >> 32);
	p[0] = static_cast<cell>(lo);
	p[1] = static_cast<cell>(hi);
}

static inline uint64_t BigInt_ReadU(cell* p)
{
	uint32_t lo = static_cast<uint32_t>(p[0]);
	uint32_t hi = static_cast<uint32_t>(p[1]);
	return (static_cast<uint64_t>(hi) << 32) | lo;
}

static inline void BigInt_WriteU(cell* p, uint64_t v)
{
	uint32_t lo = static_cast<uint32_t>(v & 0xFFFFFFFFULL);
	uint32_t hi = static_cast<uint32_t>(v >> 32);
	p[0] = static_cast<cell>(lo);
	p[1] = static_cast<cell>(hi);
}

namespace BigIntNatives
{
	cell AMX_NATIVE_CALL FromInt(AMX* amx, cell* params)
	{
		auto ptr = GetBigIntPtr(amx, params[1]);
		BigInt_Write(ptr, static_cast<int64_t>(params[2]));
		return 1;
	}

	// BigInt_ToInt(const BigInt:value[]);
	cell AMX_NATIVE_CALL ToInt(AMX* amx, cell* params)
	{
		auto arr = GetBigIntPtr(amx, params[1]);
		int64_t v = BigInt_Read(arr);

		if (v > std::numeric_limits<int32_t>::max())
		{
			v = std::numeric_limits<int32_t>::max();
		}

		if (v < std::numeric_limits<int32_t>::min())
		{
			v = std::numeric_limits<int32_t>::min();
		}

		return static_cast<cell>(v);
	}

	// BigInt_FromParts(BigInt:value[], lo, hi);
	cell AMX_NATIVE_CALL FromParts(AMX* amx, cell* params)
	{
		cell* ptr = GetBigIntPtr(amx, params[1]);
		uint32_t lo = static_cast<uint32_t>(params[2]);
		int32_t hi = static_cast<int32_t>(params[3]);
		int64_t v = (static_cast<int64_t>(hi) << 32) | lo;
		BigInt_Write(ptr, v);
		return 1;
	}

	// BigInt_GetParts(const BigInt:value[], &lo, &hi);
	cell AMX_NATIVE_CALL GetParts(AMX* amx, cell* params)
	{
		cell* val = GetBigIntPtr(amx, params[1]);
		int64_t v = BigInt_Read(val);
		uint32_t lo = static_cast<uint32_t>(v & 0xFFFFFFFFLL);
		int32_t hi = static_cast<int32_t>(v >> 32);

		cell* loAddr = nullptr;
		cell* hiAddr = nullptr;
		amx_GetAddr(amx, params[2], &loAddr);
		amx_GetAddr(amx, params[3], &hiAddr);
		*loAddr = static_cast<cell>(lo);
		*hiAddr = static_cast<cell>(hi);
		return 1;
	}

	// BigInt_FromString(BigInt:value[], const str[]);
	cell AMX_NATIVE_CALL FromString(AMX* amx, cell* params)
	{
		cell* val = nullptr;
		amx_GetAddr(amx, params[1], &val);

		cell* strAddr = nullptr;
		amx_GetAddr(amx, params[2], &strAddr);
		
		int len = 0;
		amx_StrLen(strAddr, &len);
		if (len > 0)
		{
			char* str = new char[len + 1];
			amx_GetString(str, strAddr, 0, len + 1);
			try
			{
				long long parsed = std::stoll(str);
				BigInt_Write(val, static_cast<int64_t>(parsed));
				delete[] str;
				return 1;
			}
			catch (...)
			{
				delete[] str;
				return 0;
			}
		}
		return 0;
	}

	// BigInt_ToString(const BigInt:value[], dest[], size);
	cell AMX_NATIVE_CALL ToString(AMX* amx, cell* params)
	{
		cell* val = GetBigIntPtr(amx, params[1]);
		int64_t v = BigInt_Read(val);
		std::string str = std::to_string(v);
		
		cell* destAddr = nullptr;
		amx_GetAddr(amx, params[2], &destAddr);
		amx_SetString(destAddr, str.c_str(), 0, 0, params[3]);
		return static_cast<cell>(str.length());
	}

	// BigInt_Add(BigInt:dst[], const BigInt:src[]);
	cell AMX_NATIVE_CALL Add(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		cell* src = GetBigIntPtr(amx, params[2]);
		int64_t a = BigInt_Read(dst);
		int64_t b = BigInt_Read(src);
		BigInt_Write(dst, a + b);
		return 1;
	}

	// BigInt_Sub(BigInt:dst[], const BigInt:src[]);
	cell AMX_NATIVE_CALL Sub(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		cell* src = GetBigIntPtr(amx, params[2]);
		int64_t a = BigInt_Read(dst);
		int64_t b = BigInt_Read(src);
		BigInt_Write(dst, a - b);
		return 1;
	}

	// BigInt_Mul(BigInt:dst[], const BigInt:src[]);
	cell AMX_NATIVE_CALL Mul(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		cell* src = GetBigIntPtr(amx, params[2]);
		int64_t a = BigInt_Read(dst);
		int64_t b = BigInt_Read(src);
		BigInt_Write(dst, a * b);
		return 1;
	}

	// BigInt_Div(BigInt:dst[], const BigInt:src[]);
	cell AMX_NATIVE_CALL Div(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		cell* src = GetBigIntPtr(amx, params[2]);
		int64_t a = BigInt_Read(dst);
		int64_t b = BigInt_Read(src);
		if (b == 0)
		{
			return 0;
		}
		BigInt_Write(dst, a / b);
		return 1;
	}

	// BigInt_Mod(BigInt:dst[], const BigInt:src[]);
	cell AMX_NATIVE_CALL Mod(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		cell* src = GetBigIntPtr(amx, params[2]);
		int64_t a = BigInt_Read(dst);
		int64_t b = BigInt_Read(src);
		if (b == 0)
		{
			return 0;
		}
		BigInt_Write(dst, a % b);
		return 1;
	}

	// BigInt_AddInt(BigInt:dst[], v);
	cell AMX_NATIVE_CALL AddInt(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		int64_t a = BigInt_Read(dst);
		BigInt_Write(dst, a + params[2]);
		return 1;
	}

	// BigInt_SubInt(BigInt:dst[], v);
	cell AMX_NATIVE_CALL SubInt(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		int64_t a = BigInt_Read(dst);
		BigInt_Write(dst, a - params[2]);
		return 1;
	}

	// BigInt_MulInt(BigInt:dst[], v);
	cell AMX_NATIVE_CALL MulInt(AMX* amx, cell* params)
	{
		cell* dst = GetBigIntPtr(amx, params[1]);
		int64_t a = BigInt_Read(dst);
		BigInt_Write(dst, a * params[2]);
		return 1;
	}

	// BigInt_DivInt(BigInt:dst[], v);
	cell AMX_NATIVE_CALL DivInt(AMX* amx, cell* params)
	{
		if (params[2] == 0)
		{
			return 0;
		}
		cell* dst = GetBigIntPtr(amx, params[1]);
		int64_t a = BigInt_Read(dst);
		BigInt_Write(dst, a / params[2]);
		return 1;
	}

	// BigInt_ModInt(BigInt:dst[], v);
	cell AMX_NATIVE_CALL ModInt(AMX* amx, cell* params)
	{
		if (params[2] == 0)
		{
			return 0;
		}
		cell* dst = GetBigIntPtr(amx, params[1]);
		int64_t a = BigInt_Read(dst);
		BigInt_Write(dst, a % params[2]);
		return 1;
	}

	// BigInt_Cmp(const BigInt:a[], const BigInt:b[]);
	cell AMX_NATIVE_CALL Cmp(AMX* amx, cell* params)
	{
		cell* a = GetBigIntPtr(amx, params[1]);
		cell* b = GetBigIntPtr(amx, params[2]);
		int64_t va = BigInt_Read(a);
		int64_t vb = BigInt_Read(b);

		if (va < vb)
		{
			return -1;
		}
		if (va > vb)
		{
			return 1;
		}
		return 0;
	}

	// BigInt_CmpInt(const BigInt:a[], v);
	cell AMX_NATIVE_CALL CmpInt(AMX* amx, cell* params)
	{
		cell* a = GetBigIntPtr(amx, params[1]);
		int64_t va = BigInt_Read(a);
		int64_t vb = static_cast<int64_t>(params[2]);

		if (va < vb)
		{
			return -1;
		}
		if (va > vb)
		{
			return 1;
		}
		return 0;
	}
}

