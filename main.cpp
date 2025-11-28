/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, AmyrAhmady.
 */

#include "bigint_common.hpp"

typedef void (*logprintf_t)(const char* format, ...);
logprintf_t logprintf;

#ifdef BUILD_SAMP_PLUGIN
// SA:MP Plugin Mode
#include "samp_plugin.h"
#include <amx/amx.h>

extern void *pAMXFunctions;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];	
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	
	if (!pAMXFunctions)
	{
		if (logprintf)
			logprintf(" * bigint plugin: ERROR - Failed to get AMX function table!");
		return false;
	}
	
	if (logprintf)
		logprintf(" * bigint plugin was loaded.");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
}

AMX_NATIVE_INFO natives[] =
{
	{ "BigInt_FromInt", BigIntNatives::FromInt },
	{ "BigInt_ToInt", BigIntNatives::ToInt },
	{ "BigInt_FromParts", BigIntNatives::FromParts },
	{ "BigInt_GetParts", BigIntNatives::GetParts },
	{ "BigInt_FromString", BigIntNatives::FromString },
	{ "BigInt_ToString", BigIntNatives::ToString },
	{ "BigInt_Add", BigIntNatives::Add },
	{ "BigInt_Sub", BigIntNatives::Sub },
	{ "BigInt_Mul", BigIntNatives::Mul },
	{ "BigInt_Div", BigIntNatives::Div },
	{ "BigInt_Mod", BigIntNatives::Mod },
	{ "BigInt_AddInt", BigIntNatives::AddInt },
	{ "BigInt_SubInt", BigIntNatives::SubInt },
	{ "BigInt_MulInt", BigIntNatives::MulInt },
	{ "BigInt_DivInt", BigIntNatives::DivInt },
	{ "BigInt_ModInt", BigIntNatives::ModInt },
	{ "BigInt_Cmp", BigIntNatives::Cmp },
	{ "BigInt_CmpInt", BigIntNatives::CmpInt },
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	return AMX_ERR_NONE;
}

#else
// open.mp Component Mode
#include <sdk.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

class OmpBigInt final : public IComponent, public PawnEventHandler
{
private:
	ICore* core_ = nullptr;
	IPawnComponent* pawn_ = nullptr;

public:
	PROVIDE_UID(0xFA1215D0DE6F4C74);

	~OmpBigInt()
	{
		if (pawn_)
		{
			pawn_->getEventDispatcher().removeEventHandler(this);
		}
	}

	void onAmxLoad(IPawnScript& script) override
	{
		pawn_natives::AmxLoad(script.GetAMX());
	}

	void onAmxUnload(IPawnScript& script) override
	{
	}

	StringView componentName() const override
	{
		return "open.mp BigInt component";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(1, 0, 0, 0);
	}

	void onLoad(ICore* c) override
	{
		core_ = c;
		core_->printLn("open.mp BigInt component loaded.");
		setAmxLookups(core_);
	}

	void onInit(IComponentList* components) override
	{
		pawn_ = components->queryComponent<IPawnComponent>();

		if (pawn_)
		{
			setAmxFunctions(pawn_->getAmxFunctions());
			setAmxLookups(components);
			pawn_->getEventDispatcher().addEventHandler(this);
		}
	}

	void onReady() override
	{
	}

	void onFree(IComponent* component) override
	{
		if (component == pawn_)
		{
			pawn_ = nullptr;
			setAmxFunctions();
			setAmxLookups();
		}
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
	}
};

COMPONENT_ENTRY_POINT()
{
	return new OmpBigInt();
}

// open.mp native wrappers using SCRIPT_API
// BigInt_FromInt(BigInt:value[], v);
SCRIPT_API(BigInt_FromInt, bool(cell bigIntAddr, int value))
{
	auto ptr = GetBigIntPtr(GetAMX(), bigIntAddr);
	BigInt_Write(ptr, static_cast<int64_t>(value));
	return true;
}

// BigInt_ToInt(const BigInt:value[]);
SCRIPT_API(BigInt_ToInt, int(cell bigIntAddr))
{
	auto arr = GetBigIntPtr(GetAMX(), bigIntAddr);
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
SCRIPT_API(BigInt_FromParts, bool(cell bigIntAddr, int lo, int hi))
{
	cell* ptr = GetBigIntPtr(GetAMX(), bigIntAddr);
	uint32_t _lo = static_cast<uint32_t>(lo);
	int32_t _hi = static_cast<int32_t>(hi);
	int64_t v = (static_cast<int64_t>(hi) << 32) | lo;
	BigInt_Write(ptr, v);
	return true;
}

// BigInt_GetParts(const BigInt:value[], &lo, &hi);
SCRIPT_API(BigInt_GetParts, bool(cell bigIntAddr, int& lo, int& hi))
{
	cell* val = GetBigIntPtr(GetAMX(), bigIntAddr);

	int64_t v = BigInt_Read(val);
	uint32_t _lo = static_cast<uint32_t>(v & 0xFFFFFFFFLL);
	int32_t _hi = static_cast<int32_t>(v >> 32);

	lo = static_cast<int>(_lo);
	hi = static_cast<int>(_hi);
	return true;
}

// BigInt_FromString(BigInt:value[], const str[]);
SCRIPT_API(BigInt_FromString, bool(cell bigIntAddr, const std::string& str))
{
	cell* val;
	amx_GetAddr(GetAMX(), bigIntAddr, &val);

	char* end = nullptr;
	long long parsed = std::stoll(str);

	BigInt_Write(val, static_cast<int64_t>(parsed));
	return true;
}

// BigInt_ToString(const BigInt:value[], dest[], size);
SCRIPT_API(BigInt_ToString, int(cell bigIntAddr, OutputOnlyString& str))
{
	cell* val = GetBigIntPtr(GetAMX(), bigIntAddr);
	int64_t v = BigInt_Read(val);
	str = std::to_string(v);
	return std::get<StringView>(str).length();
}

// BigInt_Add(BigInt:dst[], const BigInt:src[]);
SCRIPT_API(BigInt_Add, bool(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* src = GetBigIntPtr(GetAMX(), bigInt2Addr);

	int64_t a = BigInt_Read(dst);
	int64_t b = BigInt_Read(src);
	BigInt_Write(dst, a + b);
	return true;
}

// BigInt_Sub(BigInt:dst[], const BigInt:src[]);
SCRIPT_API(BigInt_Sub, bool(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* src = GetBigIntPtr(GetAMX(), bigInt2Addr);

	int64_t a = BigInt_Read(dst);
	int64_t b = BigInt_Read(src);
	BigInt_Write(dst, a - b);
	return true;
}

// BigInt_Mul(BigInt:dst[], const BigInt:src[]);
SCRIPT_API(BigInt_Mul, bool(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* src = GetBigIntPtr(GetAMX(), bigInt2Addr);

	int64_t a = BigInt_Read(dst);
	int64_t b = BigInt_Read(src);
	BigInt_Write(dst, a * b);
	return true;
}

// BigInt_Div(BigInt:dst[], const BigInt:src[]);
SCRIPT_API(BigInt_Div, bool(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* src = GetBigIntPtr(GetAMX(), bigInt2Addr);

	int64_t a = BigInt_Read(dst);
	int64_t b = BigInt_Read(src);
	if (b == 0)
	{
		return false;
	}
	BigInt_Write(dst, a / b);
	return true;
}

// BigInt_Mod(BigInt:dst[], const BigInt:src[]);
SCRIPT_API(BigInt_Mod, bool(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* src = GetBigIntPtr(GetAMX(), bigInt2Addr);

	int64_t a = BigInt_Read(dst);
	int64_t b = BigInt_Read(src);
	if (b == 0)
	{
		return false;
	}
	BigInt_Write(dst, a % b);
	return true;
}

// BigInt_AddInt(BigInt:dst[], v);
SCRIPT_API(BigInt_AddInt, bool(cell bigIntAddr, int value))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigIntAddr);
	int64_t a = BigInt_Read(dst);
	BigInt_Write(dst, a + value);
	return true;
}

// BigInt_SubInt(BigInt:dst[], v);
SCRIPT_API(BigInt_SubInt, bool(cell bigIntAddr, int value))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigIntAddr);
	int64_t a = BigInt_Read(dst);
	BigInt_Write(dst, a - value);
	return true;
}

// BigInt_MulInt(BigInt:dst[], v);
SCRIPT_API(BigInt_MulInt, bool(cell bigIntAddr, int value))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigIntAddr);
	int64_t a = BigInt_Read(dst);
	BigInt_Write(dst, a * value);
	return true;
}

// BigInt_DivInt(BigInt:dst[], v);
SCRIPT_API(BigInt_DivInt, bool(cell bigIntAddr, int value))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigIntAddr);

	if (value == 0)
	{
		return false;
	}

	int64_t a = BigInt_Read(dst);
	BigInt_Write(dst, a / value);
	return true;
}

// BigInt_ModInt(BigInt:dst[], v);
SCRIPT_API(BigInt_ModInt, bool(cell bigIntAddr, int value))
{
	cell* dst = GetBigIntPtr(GetAMX(), bigIntAddr);

	if (value == 0)
	{
		return false;
	}

	int64_t a = BigInt_Read(dst);
	BigInt_Write(dst, a % value);
	return true;
}

// BigInt_Cmp(const BigInt:a[], const BigInt:b[]);
SCRIPT_API(BigInt_Cmp, int(cell bigInt1Addr, cell bigInt2Addr))
{
	cell* a = GetBigIntPtr(GetAMX(), bigInt1Addr);
	cell* b = GetBigIntPtr(GetAMX(), bigInt2Addr);

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
SCRIPT_API(BigInt_CmpInt, int(cell bigIntAddr, int value))
{
	cell* a = GetBigIntPtr(GetAMX(), bigIntAddr);
	int64_t va = BigInt_Read(a);
	int64_t vb = static_cast<int64_t>(value);

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

#endif
