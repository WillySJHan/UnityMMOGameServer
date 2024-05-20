#pragma once


 //Crash
#define SJ_CRASH(cause)						\
{											\
	uint32_t* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define SJ_ASSERT(expr)			\
{									\
	if (!(expr))					\
	{								\
		SJ_CRASH("ASSERT");		\
		__analysis_assume(expr);	\
	}								\
}


