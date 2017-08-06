#include "vix_all.h"

#ifdef WIN32

void meLog(int level, char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsprintf(buffer, format, args);

    if (Exporter->LogDebug >= level)
#ifdef _DEBUG
		OutputDebugString(buffer);
#else
		std::cerr << buffer << std::endl;
#endif
    if (Exporter->LogLevel >= level)
		*(Exporter->LogFile) << buffer << std::endl;
    va_end(args);
}



#else	// not WIN32

#include <stdarg.h>
 
void meLog(int level, char *format, ...)
{
    va_list args;

    va_start(args, format);
    if (Exporter->LogLevel >= level)
		vfprintf(Exporter->LogFile, format, args);
    if (Exporter->LogDebug >= level)
		vfprintf(stderr, format, args);
    va_end(args);
}

#endif

void meDebugPrint(char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsprintf(buffer, format, args);
	*(Exporter->LogFile) << buffer << std::endl;
	vfprintf(stderr, format, args);
	std::cerr << std::endl;
    va_end(args);
}

