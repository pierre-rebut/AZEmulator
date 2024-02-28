//
// Created by pierr on 28/07/2023.
//
#ifdef WIN32
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

#include "SharedLibrary.h"
#include "Commons/AstraException.h"

namespace Astra {
    SharedLibrary::SharedLibrary(const std::filesystem::path& lib) {
#ifdef WIN32
        m_lib = LoadLibrary(lib.string().c_str());
#else
        m_libUUID = dlopen(lib.string().c_str(), RTLD_NOW);
#endif

        AstraException::assertV(m_lib, "SharedLibrary: can not load lib {}", lib);
    }

    SharedLibrary::~SharedLibrary() {
#ifdef WIN32
        FreeLibrary((HMODULE) m_lib);
#else
        dlclose(m_libUUID);
#endif
    }

    void* SharedLibrary::getFunction(const std::string& pName) const {
        void* ptr = nullptr;

#ifdef WIN32
        ptr = (void*) GetProcAddress((HMODULE) m_lib, pName.c_str());
#else
        ptr = dlsym(m_libUUID, pName.c_str());
#endif

        AstraException::assertV(ptr, "SharedLibrary: can not find function {} in lib", pName);

        return ptr;
    }
}
