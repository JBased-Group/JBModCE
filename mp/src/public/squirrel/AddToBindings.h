SQ_FUNCTION_DECL(SQ_FUNCTION());

template <typename ReturnType, typename... Arguments>
int CONCAT(SQ_FUNCTION_NAME(SQ_FUNCTION()), _WRAPPED)(SquirrelScript script)
{
	constexpr unsigned int count = sizeof...(Arguments);
	HSQUIRRELVM vm = (HSQUIRRELVM)script;
	if constexpr (count > 0)
	{
		sq_gettype()
	}
}

constexpr auto CONCAT(CONCAT(LIBRARY_NAME, COUNTER_A),_SIG) = GetSignature(SQ_FUNCTION_NAME(SQ_FUNCTION()));
constexpr auto CONCAT(LIBRARY_NAME, COUNTER_A) = Append(CONCAT(LIBRARY_NAME, COUNTER_B), SQ_FUNCTION_NAME(SQ_FUNCTION()), CONCAT(CONCAT(LIBRARY_NAME, COUNTER_A),_SIG).Data);
#include INCREMENT_COUNTER_A
#include INCREMENT_COUNTER_B

SQ_FUNCTION_DECL(SQ_FUNCTION())