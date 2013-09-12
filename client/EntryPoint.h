#ifdef WIN32
#	define DLLEXPORT __declspec(dllexport)
#	define C_DLLEXPORT extern "C" DLLEXPORT
#	ifdef linux
#		undef linux
#	endif
#else
#	define C_DLLEXPORT
#	define DLLEXPORT
#endif
