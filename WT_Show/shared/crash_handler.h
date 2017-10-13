#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include "shared_global.h"

#include <Windows.h>
#include <QString>

class SHAREDSHARED_EXPORT CrashHandler
{
public:
    static void HookUnhandledExceptions(const QString& processName);

private:
    static LONG WINAPI CustomCrashHandler(EXCEPTION_POINTERS* exceptionInfo);
    static QString GetSymbol(EXCEPTION_POINTERS * exceptionInfo);
    static void SaveCrashDump(EXCEPTION_POINTERS *seh);
    static QString GetDescription(EXCEPTION_POINTERS * exceptionInfo);
    static QString FormatSystemError(DWORD error);

    static QString m_ProcessName;
};

#endif // CRASH_HANDLER_H
