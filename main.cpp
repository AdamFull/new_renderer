#include <edge/Edge.h>
#include <engine/EngineApplication.h>

#include <edge/util/Logger.h>

#include <iostream>

//#include <DbgHelp.h>
//#include <tlhelp32.h>
//#include <psapi.h>

//#include <winsock2.h>
//#include <ws2tcpip.h>

//#pragma comment(lib, "Dbghelp.lib")
//#pragma comment(lib, "Psapi.lib")

//struct FStackTraceElement
//{
//    uint64_t address{};
//    uint64_t offsetInModule{};
//    uint32_t line{};
//
//    std::string fileName{};
//    std::string name{};
//    std::string demangledName{};
//    std::string moduleName{};
//};
//
//struct FCrashDumpData
//{
//    std::vector<FStackTraceElement> stacktrace{};
//    std::map<std::string, uint64_t> registers;
//    std::vector<std::string> modules{};
//
//    uint32_t threadId{};
//
//    std::string architecture{};
//    std::string buildType{};
//    std::string processName;
//    std::string osVersion;
//};
//
//BOOL CALLBACK EnumSymbolsCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) 
//{
//    auto parameters = reinterpret_cast<std::vector<std::pair<std::string, std::string>>*>(UserContext);
//    std::string symbolName = pSymInfo->Name;
//    std::string symbolType = "Unknown";
//
//    WCHAR* typeName = NULL;
//    if (SymGetTypeInfo(GetCurrentProcess(), pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_SYMNAME, &typeName)) 
//    {
//        wchar_t* wTypeName = typeName;
//        using convert_type = std::codecvt_utf8<wchar_t>;
//        std::wstring_convert<convert_type, wchar_t> converter;
//        symbolType = converter.to_bytes(wTypeName);
//
//        LocalFree(typeName);
//    }
//
//    parameters->emplace_back(symbolName, symbolType);
//
//    return TRUE;
//}
//
//void FetchFunctionParameters(DWORD64 address, std::vector<std::pair<std::string, std::string>>& parameters) 
//{
//    HANDLE process = GetCurrentProcess();
//
//    DWORD64 baseOfDll = SymGetModuleBase64(process, address);
//    if (baseOfDll == 0)
//        return;
//
//    SymEnumSymbols(process, baseOfDll, NULL, EnumSymbolsCallback, &parameters);
//}
//
//void CaptureStackTrace(FCrashDumpData& crashdump)
//{
//    HANDLE process = GetCurrentProcess();
//    HANDLE thread = GetCurrentThread();
//
//    SymInitialize(process, NULL, TRUE);
//
//    CONTEXT context;
//    memset(&context, 0, sizeof(CONTEXT));
//    context.ContextFlags = CONTEXT_FULL;
//    RtlCaptureContext(&context);
//
//    STACKFRAME64 stack;
//    memset(&stack, 0, sizeof(STACKFRAME64));
//
//#ifdef _M_IX86
//    DWORD machine = IMAGE_FILE_MACHINE_I386;
//    stack.AddrPC.Offset = context.Eip;
//    stack.AddrFrame.Offset = context.Ebp;
//    stack.AddrStack.Offset = context.Esp;
//#elif _M_X64
//    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
//    stack.AddrPC.Offset = context.Rip;
//    stack.AddrFrame.Offset = context.Rbp;
//    stack.AddrStack.Offset = context.Rsp;
//#else
//#error "Platform not supported!"
//#endif
//
//    stack.AddrPC.Mode = AddrModeFlat;
//    stack.AddrFrame.Mode = AddrModeFlat;
//    stack.AddrStack.Mode = AddrModeFlat;
//
//    while (StackWalk64(machine, process, thread, &stack, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) 
//    {
//        DWORD64 address = stack.AddrPC.Offset;
//        if (address == 0)
//            break;
//
//        DWORD64 displacementSym = 0;
//        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
//        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
//        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
//        symbol->MaxNameLen = MAX_SYM_NAME;
//
//        auto& new_trace_line = crashdump.stacktrace.emplace_back();
//        if (SymFromAddr(process, address, &displacementSym, symbol))
//        {
//            char demangledName[1024];
//            UnDecorateSymbolName(symbol->Name, demangledName, sizeof(demangledName), UNDNAME_COMPLETE);
//            new_trace_line.demangledName = demangledName;
//
//            new_trace_line.address = symbol->Address;
//            new_trace_line.name = symbol->Name;
//        }
//        else
//        {
//            new_trace_line.address = address;
//            new_trace_line.name = "Unknown";
//        }
//
//        DWORD displacementLine = 0;
//        IMAGEHLP_LINE64 line = { 0 };
//        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
//
//        if (SymGetLineFromAddr64(process, address, &displacementLine, &line)) 
//        {
//            new_trace_line.fileName = line.FileName;
//            new_trace_line.line = line.LineNumber;
//        }
//
//        IMAGEHLP_MODULE64 moduleInfo = { 0 };
//        moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
//
//        if (SymGetModuleInfo64(process, address, &moduleInfo))
//        {
//            new_trace_line.moduleName = moduleInfo.ModuleName;
//            new_trace_line.offsetInModule = address - moduleInfo.BaseOfImage;
//        }
//
//        //FetchFunctionParameters(address, new_trace_line.parameters);
//    }
//
//    SymCleanup(process);
//}
//
//void CaptureRegistersState(FCrashDumpData& crashdump)
//{
//    CONTEXT context;
//    RtlCaptureContext(&context);
//
//    crashdump.registers["Rax"] = context.Rax;
//    crashdump.registers["Rbx"] = context.Rbx;
//    crashdump.registers["Rcx"] = context.Rcx;
//    crashdump.registers["Rdx"] = context.Rdx;
//    crashdump.registers["Rsi"] = context.Rsi;
//    crashdump.registers["Rdi"] = context.Rdi;
//    crashdump.registers["Rbp"] = context.Rbp;
//    crashdump.registers["Rsp"] = context.Rsp;
//    crashdump.registers["Rip"] = context.Rip;
//    crashdump.registers["R8"] = context.R8;
//    crashdump.registers["R9"] = context.R9;
//    crashdump.registers["R10"] = context.R10;
//    crashdump.registers["R11"] = context.R11;
//    crashdump.registers["R12"] = context.R12;
//    crashdump.registers["R13"] = context.R13;
//    crashdump.registers["R14"] = context.R14;
//    crashdump.registers["R15"] = context.R15;
//
//    crashdump.registers["Cs"] = context.SegCs;
//    crashdump.registers["Ds"] = context.SegDs;
//    crashdump.registers["Es"] = context.SegEs;
//    crashdump.registers["Fs"] = context.SegFs;
//    crashdump.registers["Gs"] = context.SegGs;
//    crashdump.registers["Ss"] = context.SegSs;
//
//    crashdump.registers["EFlags"] = context.EFlags;
//}
//
//void CaptureArchitecture(FCrashDumpData& crashdump)
//{
//#if defined(_M_X64) || defined(__x86_64__)
//    crashdump.architecture = "x64";
//#elif defined(_M_IX86) || defined(__i386__)
//    crashdump.architecture = "x86";
//#elif defined(_M_ARM64) || defined(__aarch64__)
//    crashdump.architecture = "ARM64";
//#elif defined(_M_ARM) || defined(__arm__)
//    crashdump.architecture = "ARM";
//#else
//    crashdump.architecture = "Unknown";
//#endif
//}
//
//void CaptureBuildType(FCrashDumpData& crashdump)
//{
//#if defined(NDEBUG)
//#if defined(_DEBUG_INFO)
//    crashdump.buildType = "RelWithDebInfo";
//#else
//    crashdump.buildType = "Release";
//#endif
//#else
//    crashdump.buildType = "Debug";
//#endif
//}
//
//void CaptureUsedLibraries(FCrashDumpData& crashdump)
//{
//    HANDLE process = GetCurrentProcess();
//    HMODULE modules[1024];
//    DWORD cbNeeded;
//
//    if (EnumProcessModules(process, modules, sizeof(modules), &cbNeeded)) {
//        int modulesCount = cbNeeded / sizeof(HMODULE);
//
//        for (int i = 0; i < modulesCount; i++) 
//        {
//            TCHAR szModName[MAX_PATH];
//            if (GetModuleFileNameEx(process, modules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
//                crashdump.modules.emplace_back(szModName);
//        }
//    }
//}
//
//void CaptureProcessName(FCrashDumpData& crashdump)
//{
//    TCHAR buffer[MAX_PATH] = { 0 };
//    if (GetModuleFileNameEx(GetCurrentProcess(), NULL, buffer, MAX_PATH))
//        crashdump.processName = buffer;
//}
//
//void CaptureOSVersion(FCrashDumpData& crashdump)
//{
//    OSVERSIONINFO osvi;
//    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
//    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//
//    if (GetVersionEx(&osvi)) 
//    {
//        std::string version = "Windows " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
//        crashdump.osVersion = version;
//    }
//}
//
//
//FCrashDumpData CaptureCrashDump()
//{
//    FCrashDumpData crashdump{};
//
//    CaptureRegistersState(crashdump);
//    CaptureProcessName(crashdump);
//    CaptureOSVersion(crashdump);
//    CaptureUsedLibraries(crashdump);
//    CaptureStackTrace(crashdump);
//    CaptureArchitecture(crashdump);
//    CaptureBuildType(crashdump);
//    crashdump.threadId = GetCurrentThreadId();
//
//    return crashdump;
//}

class MyApp : public engine::EngineApplication
{
public:
    ~MyApp() = default;
private:

};

int RuntimeMain(const edge::IRuntimeContext& context)
{
    auto runtime = edge::create_application_context(context);

    edge::ApplicationCreateInfo app_ci{};
    app_ci.name = "MyApp";
    app_ci.app_allocator = []() { return std::make_unique<MyApp>(); };
    
    runtime->request_application(&app_ci);
    
    auto code = runtime->initialize();
    if (code == edge::ExitCode::Success)
    {
    	code = runtime->main_loop();
    }
    
    runtime->terminate(code);

    return 0;
}