#include "crash_handler.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <DbgHelp.h>

QString CrashHandler::m_ProcessName;

void CrashHandler::HookUnhandledExceptions(const QString& processName)
{
    m_ProcessName = processName;

    // By setting the error mode (http://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx).
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

    // And setting the callback to win32 unhandled exceptions.
    SetUnhandledExceptionFilter(CustomCrashHandler);
}

LONG WINAPI CrashHandler::CustomCrashHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    SaveCrashDump(exceptionInfo);

    // Error messages from: http://msdn.microsoft.com/en-us/library/windows/desktop/aa363082%28v=vs.85%29.aspx
    // Notice: we are writing the error messages in our code to make it easier for developers to find messages in the
    // source.

    QString description = GetDescription(exceptionInfo);

    // Try to get extra log information
    EXCEPTION_RECORD * exceptionRecord = exceptionInfo->ExceptionRecord;
    MEMORY_BASIC_INFORMATION mbi;
    size_t cb = VirtualQuery(exceptionRecord->ExceptionAddress, &mbi, sizeof(mbi));

    QString exceptionString;

    ULONG_PTR addr = (ULONG_PTR)exceptionRecord->ExceptionAddress - (ULONG_PTR)mbi.AllocationBase;
    if (cb == sizeof(mbi))
    {
        wchar_t szModule[MAX_PATH];
        if (GetModuleFileName((HMODULE)mbi.AllocationBase, szModule, MAX_PATH))
        {
            QTextStream(&exceptionString) << "Exception at '"
                                          << QString::fromWCharArray(szModule)
                                          << "' 0x" << hex << addr << ".";
        }
    }

    QString symbolString = GetSymbol(exceptionInfo);

    if (exceptionInfo->ExceptionRecord->ExceptionCode == 0x40000015 &&
        exceptionInfo->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE &&
        exceptionInfo->ExceptionRecord->NumberParameters == 1)
    {
        char * extra_info = (char *)exceptionInfo->ExceptionRecord->ExceptionInformation[0];
        qCritical() << L"0x40000015: " << extra_info;
    }

    // Than log it, and force our process to close.
    QString crashMsg;
    QTextStream(&crashMsg) << m_ProcessName << " just got an evil unhandled exception from the OS: "
             << "0x" << hex << exceptionInfo->ExceptionRecord->ExceptionCode << " " << description << " "
             << exceptionString << symbolString;

    try
    {
        qFatal(crashMsg.toUtf8().data());
    }
    catch (...)
    {

    }

    return 0;
}

QString CrashHandler::GetSymbol(EXCEPTION_POINTERS * exceptionInfo)
{
    QString symbol(" (");

    HANDLE hProcess = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    if (SymInitialize(hProcess, NULL, TRUE))
    {
        EXCEPTION_RECORD * exceptionRecord = exceptionInfo->ExceptionRecord;
        DWORD64  dwDisplacement = 0;
        DWORD64  dwAddress = (ULONG_PTR)exceptionRecord->ExceptionAddress;

        byte buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;

        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        IMAGEHLP_LINEW64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

        DWORD dwDisplacement32 = 0;
        if (SymGetLineFromAddrW64(hProcess, dwAddress, &dwDisplacement32, &line))
        {
            QTextStream(&symbol) << " File:" << QString::fromWCharArray(line.FileName)
                                 << ":" << dec << line.LineNumber << " ";
        }
        else
        {
#ifdef _DEBUG
            DWORD error = GetLastError();
            qCritical() << "SymGetLineFromAddrW64 failed: " << FormatSystemError(error);
#endif
        }

        if (SymFromAddrW(hProcess, dwAddress, &dwDisplacement, pSymbol))
        {
            QTextStream(&symbol) << "Function: " << QString::fromWCharArray(pSymbol->Name);
        }
        else
        {
           DWORD error = GetLastError();
           qCritical() << "SymFromAddrW failed: " << FormatSystemError(error);
        }

        if (SymCleanup(hProcess))
        {
            // SymCleanup returned success
        }
        else
        {
           DWORD error = GetLastError();
           qCritical() << "SymCleanup failed: " << FormatSystemError(error);
        }
    }
    else
    {
           DWORD error = GetLastError();
           qCritical() << "SymInitialize failed: " << FormatSystemError(error);
    }

    QTextStream(&symbol) << ")";
    return symbol;
}

void CrashHandler::SaveCrashDump(EXCEPTION_POINTERS* seh)
{
    QDir tmpFolder(QStandardPaths::writableLocation(QStandardPaths::TempLocation));

    if (!tmpFolder.exists())
    {
        return;
    }

    QString dumpFile(m_ProcessName + "-" + QString::number(QDateTime::currentDateTime().toTime_t()) + ".dmp");
    QString fullPath = tmpFolder.filePath(dumpFile);

    qInfo() << "Saving dump file to " << fullPath;

    HANDLE hFile = CreateFile(fullPath.toStdWString().c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        qCritical() << "CreateFile failed: " << FormatSystemError(error);
        return;
    }


    HANDLE hProc = GetCurrentProcess();
    DWORD procID = GetProcessId(hProc);

    MINIDUMP_EXCEPTION_INFORMATION sehInfo = { 0 };
    sehInfo.ThreadId = GetCurrentThreadId();
    sehInfo.ExceptionPointers = seh;
    sehInfo.ClientPointers = FALSE;
    MINIDUMP_EXCEPTION_INFORMATION *sehPtr = &sehInfo;


    MINIDUMP_TYPE flags = static_cast<MINIDUMP_TYPE>(
          MiniDumpWithFullMemory
        | MiniDumpWithHandleData
        | MiniDumpWithUnloadedModules
        | MiniDumpWithUnloadedModules
        | MiniDumpWithProcessThreadData
        | MiniDumpWithFullMemoryInfo
        | MiniDumpWithThreadInfo
        | MiniDumpWithFullAuxiliaryState
        | MiniDumpIgnoreInaccessibleMemory
        | MiniDumpWithTokenInformation
        );
    BOOL result = MiniDumpWriteDump(hProc, procID, hFile, flags, sehPtr, NULL, NULL);

    if (!result)
    {
        DWORD error = GetLastError();
        qCritical() << "MiniDumpWriteDump failed: " << FormatSystemError(error);
        return;
    }
    CloseHandle(hFile);
}

QString CrashHandler::GetDescription(EXCEPTION_POINTERS* exceptionInfo)
{
    switch (exceptionInfo->ExceptionRecord->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        return "The thread tried to read from or write to a virtual address for which it does not have the appropriate "
                "access.";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        return "The thread tried to access an array element that is out of bounds and the underlying hardware supports "
               "bounds checking.";
    case EXCEPTION_BREAKPOINT:
        return "A breakpoint was encountered.";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        return "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. "
               "For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, "
               "and so on.";
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        return "One of the operands in a floating-point operation is denormal. A denormal value is one that is too "
               "small to represent as a standard floating-point value.";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        return "The thread tried to divide a floating-point value by a floating-point divisor of zero.";
    case EXCEPTION_FLT_INEXACT_RESULT:
        return "The result of a floating-point operation cannot be represented exactly as a decimal fraction.";
    case EXCEPTION_FLT_INVALID_OPERATION:
        return "This exception represents any floating-point exception not included in this list.";
    case EXCEPTION_FLT_OVERFLOW:
        return "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding "
               "type.";
    case EXCEPTION_FLT_STACK_CHECK:
        return "The stack overflowed or underflowed as the result of a floating-point operation.";
    case EXCEPTION_FLT_UNDERFLOW:
        return "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding "
               "type.";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        return "The thread tried to execute an invalid instruction.";
    case EXCEPTION_IN_PAGE_ERROR:
        return "The thread tried to access a page that was not present, and the system was unable to load the page. "
               "For example, this exception might occur if a network connection is lost while running a program over "
               "the network.";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return "The thread tried to divide an integer value by an integer divisor of zero.";
    case EXCEPTION_INT_OVERFLOW:
        return "The result of an integer operation caused a carry out of the most significant bit of the result.";
    case EXCEPTION_INVALID_DISPOSITION:
        return "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a "
               "high-level language such as C should never encounter this exception.";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        return "The thread tried to continue execution after a noncontinuable exception occurred.";
    case EXCEPTION_PRIV_INSTRUCTION:
        return "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.";
    case EXCEPTION_SINGLE_STEP:
        return "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.";
    case EXCEPTION_STACK_OVERFLOW:
        return "The thread used up its stack.";
    default:
        return "Not defined.";
    }
}

QString CrashHandler::FormatSystemError(DWORD error)
{
    if (error)
    {
        LPWSTR errorText;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorText,
            0,
            NULL);

        QString errorStr;
        if (bufLen)
        {
            // cleaning line ends
            for (auto i = errorText; i < errorText + bufLen; i++)
            {
                if (*i == L'\r' || *i == L'\n')
                {
                    *i = L'#';
                }
            }


            QTextStream(&errorStr) << error << " (0x" << hex << error << ") - "
                                   << QString::fromWCharArray(errorText);
        }
        else
        {
            QTextStream(&errorStr) << error << " (0x" << std::hex << error << ")";
        }
        return errorStr;
    }
    return QString();
}
