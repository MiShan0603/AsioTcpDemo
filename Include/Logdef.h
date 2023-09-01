#pragma once

#include <time.h>
#include <stdio.h>
#include <string>

static std::string GetCurrentDateTime()
{
    struct tm* stTm;
    time_t tm = time(NULL);
    stTm = localtime(&tm);
    char buffer[50] = { 0 };
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", stTm->tm_year + 1900, stTm->tm_mon + 1, stTm->tm_mday, stTm->tm_hour, stTm->tm_min, stTm->tm_sec);
    return std::string(buffer);
}

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#define X_PRINT(...) do {\
	fprintf(stdout, "%s[%s %s:%d]", GetCurrentDateTime().c_str(), __FILENAME__, __FUNCTION__, __LINE__);\
	fprintf(stdout, __VA_ARGS__); \
	fprintf(stdout, "\n"); \
	fflush(stdout); \
}while(0);
