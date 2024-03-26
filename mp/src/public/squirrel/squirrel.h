#ifndef SQUIRREL_H
#define SQUIRREL_H
#define INTERFACESQUIRREL_VERSION				"JBSquirrel001"

#include "../public/counter.h"



typedef void* SquirrelScript;

enum SquirrelType
{
	SQUIRREL_INT,
	SQUIRREL_FLOAT,
	SQUIRREL_STRING,
	SQUIRREL_BOOL,
	SQUIRREL_USERDATA,

	SQUIRREL_INVALID // must be last
};


#pragma pack(push, 1)
union SquirrelHandle
{
	struct PoolId
	{
		unsigned char pool;
		unsigned char id[3];
	} p;
	int poolid;
};
#pragma pack(pop)

#define SPMASK 0x00ffffff

enum SquirrelPool
{
	SP_VGUI,

	SPCOUNT
};

struct SquirrelValue  // I FUCKING LOVE TYPED UNIONS GRAAAAAAAAAAAAAAAAAAAH
{
	union
	{
		int val_int;
		float val_float;
		const char* val_string;
		bool val_bool;
		SquirrelHandle val_userdata;
	};
	SquirrelType type;
};

typedef int (*SquirrelFunction)(SquirrelScript);





template <typename Type, typename NotTheSameType>
constexpr bool Same = false;

template <typename Type>
constexpr bool Same<Type, Type> = true;

template <typename Value_Type>
constexpr const char GetType()
{
	if (Same<Value_Type, bool>)
		return 'b';
	if (Same<Value_Type, float>)
		return 'f';
	if (Same<Value_Type, int>)
		return 'i';
	if (Same<Value_Type, void>)
		return 'v';
	if (Same<Value_Type, const char*>)
		return 's';
	if (Same<Value_Type, char*>)
		return 's';
	return '?';
}

template <unsigned int Size>
struct ReturnableString
{
	char Data[Size];

	operator char* () const
	{
		return Data;
	}

	operator const char* () const
	{
		return Data;
	}
};

template <unsigned int Size>
constexpr ReturnableString<Size> operator +(ReturnableString<Size> Left, char Character)
{
	for (int i = 0; i < Size - 1; i++)
	{
		if (Left.Data[i] == '\x00')
		{
			Left.Data[i] = Character;
			return Left;
		}
	}
	return Left;
}

template <typename Return_Type, typename... Argument_Types>
constexpr ReturnableString<sizeof...(Argument_Types) + 2> GetSignature(Return_Type(*)(Argument_Types...))
{
	ReturnableString<sizeof...(Argument_Types) + 2> Ret{};
	Ret.Data[0] = GetType<Return_Type>();
	((Ret = Ret + GetType<Argument_Types>()), ...);
	return Ret;
}


template <typename Return_Type, typename... Argument_Types>
struct SquirrelFunction2
{
	SquirrelFunction2(Return_Type(*Func)(Argument_Types...))
	{
		Signature.Data[0] = GetType<Return_Type>();
		((Signature = Signature + GetType<Argument_Types>()), ...);
		FunctionPointer = Func;
	}
	ReturnableString<sizeof...(Argument_Types) + 2> Signature{};
	Return_Type(*FunctionPointer)(Argument_Types...);
};

struct SquirrelBinding
{
	void* FunctionPtr;
	const char* Sig;
};

template <unsigned int Size> 
struct ListOfStuff
{
	SquirrelBinding Data[Size];
};

template <>
struct ListOfStuff<0>
{
};

template <unsigned int Count>
constexpr ListOfStuff<Count + 1> Append(ListOfStuff<Count> List, void* Function, const char* Sig)
{
	ListOfStuff<Count + 1> Output;
	if constexpr (Count != 0)
	{
		for (int i = 0; i < Count; i++)
		{
			Output.Data[i] = List.Data[i];
		}
	}
	Output.Data[Count].FunctionPtr = Function;
	Output.Data[Count].Sig = Sig;
	return Output;
}


#define CONCAT_A(a,b) a##b
#define CONCAT(a,b) CONCAT_A(a,b)

#define CONCAT3_A(a,b,c) a b c
#define CONCAT3(a,b,c) CONCAT3_A(a,b,c)


#include INCREMENT_COUNTER_A

#define LIBRARY_NAME SampleBindings

#define SQ_FUNCTION_NAME_IMPL(a,b,c) b
#define SQ_FUNCTION_NAME(a) SQ_FUNCTION_NAME_IMPL a
#define SQ_FUNCTION_DECL(a) CONCAT3 a


#if !defined( IGAMEEVENTS_H )

ListOfStuff<0> SampleBindings0x0000;
bool TestFunction(int, float, bool) { return true; }
SquirrelFunction2 test2(TestFunction);

#define SQ_FUNCTION() (int,EpicFunction,(int Hi, int multitest))
#include "AddToBindings.h" 
{
	return Hi;
}

#undef SQ_FUNCTION
#define SQ_FUNCTION() (const char*,AnotherOne,(const char* Woo, float Yeah))
#include "AddToBindings.h" 
{
	char* willIt = (char*)g_pMemAlloc->Alloc(64);
	V_snprintf(willIt, 64, "%s: %f", Woo, Yeah);
	return willIt;
}



typedef const char* (*TheFunc)(...);

union FourByteValue
{
	const char* v_s;
	int v_i;
	float v_f;
};
VV* argi;


#include <windows.h>
int p = 2;
class Test
{
public:
	Test()
	{
		__debugbreak();
		
		
		argi = (VV*)_alloca(8);
		argi[0].v_s = "hi lol";
		argi[1].v_f = 2.0;
		MessageBoxA(0, ((TheFunc)(SampleBindings0x0002.Data[1].FunctionPtr))(), "LOOL", MB_OK);
	}
};

Test test;
#endif

class ISquirrel
{
public:
	virtual SquirrelScript LoadScript(const char* script) = 0;
	virtual SquirrelValue CallFunction(SquirrelScript script, const char* fun, const char* types, ...) = 0;
	virtual void ShutdownScript(SquirrelScript script) = 0;
	virtual void AddFunction(SquirrelScript script, const char* name, SquirrelFunction fun) = 0;
	virtual bool GetArgs(SquirrelScript script, const char* types, ...) = 0;
	virtual void PushValue(SquirrelScript script, SquirrelValue val) = 0;
	virtual void PushArray(SquirrelScript script) = 0;
	virtual void AppendToArray(SquirrelScript script) = 0;

};



#endif

