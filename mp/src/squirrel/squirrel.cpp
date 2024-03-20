#include "squirrel.h"
#include "../public/squirrel/squirrel.h"
#include "platform.h"
#include "tier1/interface.h"
#include <stdarg.h>
#include "sqstdstring.h"
#include "sqstdmath.h"


class CSquirrel : public ISquirrel
{
public:
	virtual SquirrelScript LoadScript(const char* script);
	virtual SquirrelValue CallFunction(SquirrelScript script, const char* fun, const char* types, ...);
	virtual void ShutdownScript(SquirrelScript script);
	virtual void AddFunction(SquirrelScript script, const char* name, SquirrelFunction fun);
	virtual bool GetArgs(SquirrelScript script, const char* types, ...);
	virtual void PushValue(SquirrelScript script, SquirrelValue val);
	virtual void PushArray(SquirrelScript script);
	virtual void AppendToArray(SquirrelScript script);

};

SquirrelScript CSquirrel::LoadScript(const char* script)
{
	HSQUIRRELVM v = sq_open(2048);
	sq_pushroottable(v);
	sqstd_register_mathlib(v);
	sqstd_register_stringlib(v);
	sq_compilebuffer(v, script, strlen(script),"squirrel",SQTrue);
	sq_pushroottable(v);
	if(SQ_FAILED(sq_call(v, 1, false, true)))
	{
		return 0;
	}
	sq_settop(v, 0);
	sq_collectgarbage(v);
	return (SquirrelScript)v;
}

SquirrelValue CSquirrel::CallFunction(SquirrelScript script, const char* fun, const char* types, ...)
{
	SquirrelValue ret;
	ret.type = SQUIRREL_INVALID;
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	sq_pushroottable(v);
	sq_pushstring(v, fun,-1);
	sq_get(v, -2);

	if (sq_gettype(v, -1) != OT_CLOSURE)
	{
		sq_pop(v, 1);
		return ret;
	}
	sq_pushroottable(v);
	int count = strlen(types);
	
	va_list args;
	va_start(args, types);
	
	for (int i = 0; i < count; i++)
	{
		switch (types[i])
		{
		case 'i':
			sq_pushinteger(v, va_arg(args, int));
			break;
		case 'b':
			sq_pushbool(v, va_arg(args, bool));
			break;
		case 's':
			sq_pushstring(v, va_arg(args, const char*),-1);
			break;
		case 'f':
			sq_pushfloat(v, (float)(va_arg(args, double)));
			break;
		case 'u':
			sq_pushuserpointer(v, *(void**)&va_arg(args, SquirrelHandle));
			break;
		default:
			sq_pushnull(v);
			break;
		}
	}
	va_end(args);
	if (sq_call(v, count+1, 1, 0))
	{
		return ret;
	}
	SQObjectType type = sq_gettype(v, -1);
	if (type == OT_STRING)
	{
		ret.type = SQUIRREL_STRING;
		sq_getstring(v, -1, &ret.val_string);
	}
	else if (type == OT_INTEGER)
	{
		ret.type = SQUIRREL_INT;
		sq_getinteger(v, -1, &ret.val_int);
	}
	else if (type == OT_FLOAT)
	{
		ret.type = SQUIRREL_FLOAT;
		sq_getfloat(v, -1, &ret.val_float);
	}
	else if (type == OT_BOOL)
	{
		ret.type = SQUIRREL_BOOL;
		SQBool p;
		sq_getbool(v, -1, &p);
		ret.val_bool = p;
	}
	else if (type == OT_USERDATA)
	{
		ret.type = SQUIRREL_USERDATA;
		sq_getuserpointer(v, -1, (void**)&ret.val_userdata);
	}
	sq_pop(v, 2);
	return ret;
}
void CSquirrel::ShutdownScript(SquirrelScript script)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	sq_close(v);
}

void CSquirrel::AddFunction(SquirrelScript script, const char* name, SquirrelFunction fun)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	sq_pushroottable(v);
	sq_pushstring(v, name, -1);
	sq_newclosure(v, (SQFUNCTION)fun, 0);
	sq_newslot(v, -3, false);
	sq_pop(v, 1);
}


bool CSquirrel::GetArgs(SquirrelScript script, const char* types, ...)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	va_list args;
	va_start(args, types);
	int count = strlen(types);
	for (int i = 1; i < count+1; ++i)
	{
		SQObjectType type = sq_gettype(v, i + 1);
		switch (types[i-1])
		{
		case 'i':
			if (type == OT_INTEGER)
			{
				sq_getinteger(v, i+1, (va_arg(args, int*)));
			}
			else if (type == OT_FLOAT)
			{
				sq_getinteger(v, i + 1, (va_arg(args, int*)));
			}
			else
			{
				va_end(args);
				return false;
			}
			
			break;
		case 'b':
			if (type == OT_BOOL)
			{
				SQBool p;
				sq_getbool(v, i + 1, &p);
				*(va_arg(args, bool*)) = (bool)p;
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		case 's':
			if (type == OT_STRING)
			{
				sq_getstring(v, i + 1, (va_arg(args, const char**)));
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		case 'f':
			if (type == OT_FLOAT)
			{
				sq_getfloat(v, i + 1, (va_arg(args, float*)));
			}
			else
			{
				va_end(args);
				return false;
			}
			break;
		case 'u':
			if (type == OT_USERPOINTER)
			{
				sq_getuserpointer(v, i + 1, (void**)(va_arg(args, SquirrelHandle*)));
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
void CSquirrel::PushValue(SquirrelScript script, SquirrelValue val)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	switch (val.type)
	{
	case SQUIRREL_INT:
		sq_pushinteger(v, val.val_int);
		break;
	case SQUIRREL_BOOL:
		sq_pushbool(v, val.val_bool);
		break;
	case SQUIRREL_STRING:
		sq_pushstring(v, val.val_string,-1);
		break;
	case SQUIRREL_FLOAT:
		sq_pushfloat(v, val.val_float);
		break;
	case SQUIRREL_USERDATA:
		sq_pushuserpointer(v, *(void**)&val.val_userdata);
		break;
	default:
		sq_pushnull(v);
		break;
	}
}

void CSquirrel::PushArray(SquirrelScript script)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	sq_newarray(v, 0);
}

void CSquirrel::AppendToArray(SquirrelScript script)
{
	HSQUIRRELVM v = (HSQUIRRELVM)script;
	sq_arrayappend(v, -2);
}

EXPOSE_SINGLE_INTERFACE(CSquirrel, ISquirrel, INTERFACESQUIRREL_VERSION);
