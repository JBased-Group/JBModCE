extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "../public/lua/lua.h"
#include "tier1/interface.h"

class CLua : public ILua
{
public:
	virtual LuaScript LoadScript(const char* script, LuaAllocator allocator);
	virtual LuaValue CallFunction(LuaScript script, const char* fun, const char* types, ...);
	virtual void ShutdownScript(LuaScript script);
	virtual void AddFunction(LuaScript script, const char* name, LuaFunction fun);
	virtual bool GetArgs(LuaScript script, const char* types, ...);
	virtual void PushValue(LuaScript script, LuaValue val);
};

LuaScript CLua::LoadScript(const char* script, LuaAllocator allocator)
{
	lua_State* L = lua_newstate((lua_Alloc)allocator,0);
	luaL_openlibs(L);
	luaL_loadstring(L,script);
	if (lua_pcall(L, 0, 0, 0))
	{
		return 0;
	}
	return (LuaScript)L;
}

LuaValue CLua::CallFunction(LuaScript script, const char* fun, const char* types, ...)
{
	LuaValue ret;
	ret.type = LUA_INVALID;
	lua_State* L = (lua_State*)script;
	lua_getglobal(L, fun);
	if (lua_type(L, -1) != LUA_TFUNCTION)
	{
		return ret;
	}
	
	va_list args;
	va_start(args, types);
	
	int count = strlen(types);
	bool b;
	float f;
	for (int i = 0; i < count; ++i)
	{
		switch (types[i])
		{
		case 'i':
			lua_pushinteger(L, va_arg(args, int));
			break;
		case 'b':
			b = va_arg(args, int);
			lua_pushboolean(L, b);
			break;
		case 's':
			lua_pushstring(L, va_arg(args, const char*));
			break;
		case 'f':
			f = va_arg(args, double);
			lua_pushnumber(L, f);
			break;
		default:
			lua_pushnil(L);
			break;
		}
	}
	va_end(args);
	if (lua_pcall(L, count, 1, 0))
	{
		return ret;
	}
	if (lua_isstring(L, -1))
	{
		ret.type = LUA_STRING;
		ret.val_string = lua_tolstring(L, -1, 0);
	}
	else if (lua_isinteger(L, -1))
	{
		ret.type = LUA_INT;
		ret.val_int = lua_tointeger(L, -1);
	}
	else if (lua_isnumber(L, -1))
	{
		ret.type = LUA_FLOAT;
		ret.val_float = lua_tonumber(L, -1);
	}
	else if (lua_isboolean(L, -1))
	{
		ret.type = LUA_BOOL;
		ret.val_bool = (bool)lua_toboolean(L, -1);
	}

	return ret;
}
void CLua::ShutdownScript(LuaScript script)
{
	lua_State* L = (lua_State*)script;
	lua_close(L);
}

void CLua::AddFunction(LuaScript script, const char* name, LuaFunction fun)
{
	lua_State* L = (lua_State*)script;
	lua_pushcclosure(L, (lua_CFunction)fun, 1);
	lua_setglobal(L, name);
}


bool CLua::GetArgs(LuaScript script, const char* types, ...)
{
	lua_State* L = (lua_State*)script;
	va_list args;
	va_start(args, types);
	int count = strlen(types);
	for (int i = 0; i < count; ++i)
	{
		switch (types[i])
		{
		case 'i':
			if (lua_isinteger(L, i + 1))
			{
				*(va_arg(args, int*)) = lua_tointeger(L,  i + 1);
			}
			else if (lua_isnumber(L, i + 1))
			{
				*(va_arg(args, int*)) = lua_tointeger(L, i + 1);
			}
			else
			{
				va_end(args);
				return false;
			}
			
			break;
		case 'b':
			if (lua_isboolean(L, i + 1))
			{
				*(va_arg(args, bool*)) = (bool)lua_toboolean(L,  i + 1);
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		case 's':
			if (lua_isstring(L, i + 1))
			{
				*(va_arg(args, const char**)) = lua_tostring(L, i + 1);
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		case 'f':
			if (lua_isnumber(L, i + 1))
			{
				*(va_arg(args, float*)) = lua_tonumber(L,  i+1);
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		default:
			va_end(args);
			return false;
			break;
		}
	}
	return true;
}
void CLua::PushValue(LuaScript script, LuaValue val)
{
	lua_State* L = (lua_State*)script;
	switch (val.type)
	{
	case LUA_INT:
		lua_pushinteger(L, val.val_int);
		break;
	case LUA_BOOL:
		lua_pushboolean(L, val.val_bool);
		break;
	case LUA_STRING:
		lua_pushstring(L, val.val_string);
		break;
	case LUA_FLOAT:
		lua_pushnumber(L, val.val_float);
		break;
	default:
		lua_pushnil(L);
		break;
	}
}

InterfaceReg* InterfaceReg::s_pInterfaceRegs = NULL;

InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char* pName) :
	m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}

void* CreateInterfaceInternal(const char* pName, int* pReturnCode)
{
	InterfaceReg* pCur;

	for (pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext)
	{
		if (strcmp(pCur->m_pName, pName) == 0)
		{
			if (pReturnCode)
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}

	if (pReturnCode)
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;
}

void* CreateInterface(const char* pName, int* pReturnCode)
{
	return CreateInterfaceInternal(pName, pReturnCode);
}

EXPOSE_SINGLE_INTERFACE(CLua, ILua, INTERFACELUA_VERSION);