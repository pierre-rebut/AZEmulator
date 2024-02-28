//
// Created by pierr on 14/09/2021.
//

#pragma once

#ifdef PROFILING_ENABLE

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

#include "Base.h"
#include "Log.h"

namespace Astra {

    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult
    {
        std::string Name;

        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
        std::thread::id ThreadID;
    };

    struct ProfilingSession
    {
        std::string Name;

        explicit ProfilingSession(std::string pName);
    };

    class Profiling
    {
    public:
        Profiling(const Profiling&) = delete;

        Profiling(Profiling&&) = delete;

        ~Profiling() {
            EndSession();
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json") {
            std::scoped_lock<std::mutex> lock(m_Mutex);
            if (m_CurrentSession) {
                if (Log::GetLogger()) {
                    LOG_ERROR("Profiling::BeginSession('{0}') when session '{1}' already open.", name,
                              m_CurrentSession->Name);
                }
                InternalEndSession();
            }
            m_OutputStream.open(filepath);

            if (m_OutputStream.is_open()) {
                m_CurrentSession = CreateScope<ProfilingSession>(name);
                WriteHeader();
            } else {
                if (Log::GetLogger()) {
                    LOG_ERROR("Profiling could not open results file '{0}'.", filepath);
                }
            }
        }

        void EndSession() {
            std::scoped_lock<std::mutex> lock(m_Mutex);
            InternalEndSession();
        }

        void WriteProfile(const ProfileResult& result) {
            std::stringstream json;

            json << std::setprecision(3) << std::fixed;
            json << ",{";
            json << R"("cat":"function",)";
            json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
            json << R"("name":")" << result.Name << "\",";
            json << R"("ph":"X",)";
            json << "\"pid\":0,";
            json << "\"tid\":" << result.ThreadID << ",";
            json << "\"ts\":" << result.Start.count();
            json << "}";

            std::scoped_lock<std::mutex> lock(m_Mutex);
            if (m_CurrentSession) {
                m_OutputStream << json.str();
                m_OutputStream.flush();
            }
        }

        static Profiling& Get() {
            static Profiling instance;
            return instance;
        }

    private:
        Profiling() = default;

        void WriteHeader() {
            m_OutputStream << R"({"otherData": {},"traceEvents":[{})";
            m_OutputStream.flush();
        }

        void WriteFooter() {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        void InternalEndSession() {
            if (m_CurrentSession) {
                WriteFooter();
                m_OutputStream.close();
                m_CurrentSession.reset();
            }
        }

        std::mutex m_Mutex;
        Scope<ProfilingSession> m_CurrentSession;
        std::ofstream m_OutputStream;
    };

    class ProfilingTimer
    {
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> mStartTimepoint;
        bool m_Stopped = false;

    public:
        explicit ProfilingTimer(const char* name);

        explicit ProfilingTimer(const ProfilingTimer&) = delete;

        explicit ProfilingTimer(ProfilingTimer&&) = delete;

        ~ProfilingTimer();

        void Stop();
    };

    namespace profiling_utils {

        template<size_t N, size_t K>
        constexpr std::array<char, N> CleanupOutputString(const std::array<char, N>& expr, const std::array<char, N>& remove) {
            std::array<char, N> result = {};

            size_t srcIndex = 0;
            size_t dstIndex = 0;
            while (srcIndex < N) {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 &&
                       expr[srcIndex + matchIndex] == remove[matchIndex]) {
                    matchIndex++;
                }
                if (matchIndex == K - 1) {
                    srcIndex += matchIndex;
                }
                result[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }

            return result;
        }
    }
}

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define ENGINE_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define ENGINE_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define ENGINE_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define ENGINE_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define ENGINE_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define ENGINE_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define ENGINE_FUNC_SIG __func__
#else
#define ENGINE_FUNC_SIG "HZ_FUNC_SIG unknown!"
#endif

#define ENGINE_PROFILE_FRAME(name, filepath) ::Astra::Profiling::Get().BeginSession(name, filepath)
#define ENGINE_PROFILE_SHUTDOWN() ::Astra::Profiling::Get().EndSession()
#define ENGINE_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Astra::profiling_utils::CleanupOutputString(name, "__cdecl ");\
                                                   ::Astra::ProfilingTimer timer##line(fixedName##line.Data)
#define ENGINE_PROFILE_SCOPE_LINE(name, line) ::Astra::ProfilingTimer timer##line(name) //ENGINE_PROFILE_SCOPE_LINE2(name, line)
#define ENGINE_PROFILE_SCOPE(name) ENGINE_PROFILE_SCOPE_LINE(name, __LINE__)
#define ENGINE_PROFILE_FUNCTION() ENGINE_PROFILE_SCOPE(ENGINE_FUNC_SIG)

#define ENGINE_PROFILE_THREAD(name)

#else
#ifdef PROFILING_OPTICK

#include "optick.h"

#define ENGINE_PROFILE_FRAME(...)           OPTICK_FRAME(__VA_ARGS__)
#define ENGINE_PROFILE_SHUTDOWN()           OPTICK_SHUTDOWN()
#define ENGINE_PROFILE_FUNCTION(...)        OPTICK_EVENT(__VA_ARGS__)
#define ENGINE_PROFILE_SCOPE(NAME, ...)
#define ENGINE_PROFILE_SCOPE_DYNAMIC(NAME)  OPTICK_EVENT_DYNAMIC(NAME)
#define ENGINE_PROFILE_THREAD(...)          OPTICK_THREAD(__VA_ARGS__)

#else

#define ENGINE_PROFILE_FRAME(...)
#define ENGINE_PROFILE_SHUTDOWN()
#define ENGINE_PROFILE_END_SESSION()
#define ENGINE_PROFILE_SCOPE(name)
#define ENGINE_PROFILE_FUNCTION()
#define ENGINE_PROFILE_THREAD(name)

#endif
#endif
