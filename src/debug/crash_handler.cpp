#include <mars/debug/crash_handler.hpp>

#include <atomic>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <string>

// Platform-specific includes must be at file scope, outside any namespace,
// so that their declarations land in the correct (global/std) namespaces.
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#elif defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <cstdio>
#else
#  error "mars: crash_handler.cpp: unsupported platform"
#endif

namespace mars::debug {
namespace {

crash_handler_fn g_handler = nullptr;
std::atomic<bool> g_installed{false};

void dispatch(crash_data data) {
    if (g_handler)
        g_handler(data);
}

} // namespace

void set_crash_handler(crash_handler_fn handler) {
    g_handler = handler;
}

// ============================================================
// Windows implementation
// ============================================================
#if defined(_WIN32)

namespace {

std::string build_callstack(CONTEXT* context) {
    HANDLE process = GetCurrentProcess();
    HANDLE thread  = GetCurrentThread();

    SymInitialize(process, nullptr, TRUE);

    STACKFRAME64 frame = {};
    DWORD machine_type;

#if defined(_M_X64)
    machine_type                = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset         = context->Rip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Rbp;
    frame.AddrFrame.Mode        = AddrModeFlat;
    frame.AddrStack.Offset      = context->Rsp;
    frame.AddrStack.Mode        = AddrModeFlat;
#elif defined(_M_IX86)
    machine_type                = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset         = context->Eip;
    frame.AddrPC.Mode           = AddrModeFlat;
    frame.AddrFrame.Offset      = context->Ebp;
    frame.AddrFrame.Mode        = AddrModeFlat;
    frame.AddrStack.Offset      = context->Esp;
    frame.AddrStack.Mode        = AddrModeFlat;
#else
#  error "mars: crash_handler: unsupported Windows architecture"
#endif

    std::ostringstream out;

    while (StackWalk64(machine_type, process, thread, &frame, context,
                       nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
        const DWORD64 address = frame.AddrPC.Offset;
        if (address == 0)
            break;

        char sym_buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto* symbol         = reinterpret_cast<PSYMBOL_INFO>(sym_buf);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = MAX_SYM_NAME;

        DWORD64 sym_disp = 0;
        if (SymFromAddr(process, address, &sym_disp, symbol))
            out << "  " << symbol->Name;
        else
            out << "  [unknown]";

        IMAGEHLP_LINE64 line  = {};
        line.SizeOfStruct     = sizeof(IMAGEHLP_LINE64);
        DWORD line_disp       = 0;
        if (SymGetLineFromAddr64(process, address, &line_disp, &line))
            out << " @ " << line.FileName << ':' << std::dec << line.LineNumber;

        out << '\n';
    }

    SymCleanup(process);
    return out.str();
}

LONG WINAPI win32_exception_handler(EXCEPTION_POINTERS* info) {
    std::ostringstream reason;
    reason << "Exception 0x" << std::hex << info->ExceptionRecord->ExceptionCode;

    crash_data data;
    data.reason    = reason.str();
    data.callstack = build_callstack(info->ContextRecord);
    dispatch(std::move(data));
    return EXCEPTION_EXECUTE_HANDLER;
}

void win32_terminate_handler() {
    crash_data data;
    data.reason = "std::terminate";

    if (const auto exc = std::current_exception()) {
        try {
            std::rethrow_exception(exc);
        } catch (const std::exception& e) {
            data.reason = std::string("std::terminate: ") + e.what();
        } catch (...) {
            data.reason = "std::terminate: non-std exception";
        }
    }

    CONTEXT ctx = {};
    RtlCaptureContext(&ctx);
    data.callstack = build_callstack(&ctx);
    dispatch(std::move(data));
    std::fflush(stdout);
    std::abort();
}

} // namespace

std::string capture_callstack() {
    CONTEXT ctx = {};
    RtlCaptureContext(&ctx);
    return build_callstack(&ctx);
}

void install_crash_handlers() {
    if (g_installed.exchange(true))
        return;
    SetUnhandledExceptionFilter(win32_exception_handler);
    std::set_terminate(win32_terminate_handler);
}

// ============================================================
// Linux implementation
// ============================================================
#elif defined(__linux__)

namespace {

std::string demangle(const char* symbol) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : symbol;
    std::free(demangled);
    return result;
}

std::string build_callstack_raw() {
    constexpr int max_frames = 64;
    void* addrs[max_frames];
    const int count = backtrace(addrs, max_frames);
    char** symbols  = backtrace_symbols(addrs, count);

    std::ostringstream out;
    for (int i = 1; i < count; ++i) {
        // Try to extract the mangled name from the symbol string.
        // Format: "./binary(mangled_name+offset) [address]"
        std::string sym = symbols ? symbols[i] : "[unknown]";
        const auto paren_open  = sym.find('(');
        const auto paren_close = sym.find(')', paren_open);
        const auto plus        = sym.find('+', paren_open);

        if (paren_open != std::string::npos && plus != std::string::npos && plus > paren_open) {
            const std::string mangled = sym.substr(paren_open + 1, plus - paren_open - 1);
            if (!mangled.empty())
                out << "  " << demangle(mangled.c_str()) << '\n';
            else
                out << "  " << sym << '\n';
        } else {
            out << "  " << sym << '\n';
        }
    }

    std::free(symbols);
    return out.str();
}

const char* signal_name(int sig) {
    switch (sig) {
    case SIGSEGV: return "SIGSEGV";
    case SIGFPE:  return "SIGFPE";
    case SIGILL:  return "SIGILL";
    case SIGABRT: return "SIGABRT";
    case SIGBUS:  return "SIGBUS";
    default:      return "unknown signal";
    }
}

void linux_signal_handler(int sig, siginfo_t*, void*) {
    crash_data data;
    data.reason    = std::string(signal_name(sig)) + " (" + std::to_string(sig) + ')';
    data.callstack = build_callstack_raw();
    dispatch(std::move(data));
    _Exit(1);
}

void linux_terminate_handler() {
    crash_data data;
    data.reason = "std::terminate";

    if (const auto exc = std::current_exception()) {
        try {
            std::rethrow_exception(exc);
        } catch (const std::exception& e) {
            data.reason = std::string("std::terminate: ") + e.what();
        } catch (...) {
            data.reason = "std::terminate: non-std exception";
        }
    }

    data.callstack = build_callstack_raw();
    dispatch(std::move(data));
    std::abort();
}

} // namespace

std::string capture_callstack() {
    return build_callstack_raw();
}

void install_crash_handlers() {
    if (g_installed.exchange(true))
        return;

    struct sigaction sa = {};
    sa.sa_sigaction = linux_signal_handler;
    sa.sa_flags     = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);

    std::set_terminate(linux_terminate_handler);
}

#else
#  error "mars: crash_handler.cpp: unsupported platform"
#endif

} // namespace mars::debug
