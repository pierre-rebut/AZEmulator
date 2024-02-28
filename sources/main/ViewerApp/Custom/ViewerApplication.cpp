//
// Created by pierr on 15/03/2023.
//

#include "ViewerApplication.h"

#include "Commons/Log.h"
#include "Commons/Profiling.h"

#include "ViewerResources.h"
#include "ViewerConstants.h"
#include "CustomMainScreen.h"
#include "ViewerApp/CoreLib/System/I18N.h"

namespace Astra::UI::App {
    ViewerApplication::ViewerApplication() {
        LOG_INFO("ViewerApplication: Init");

        m_imGuiEngine = CreateScope<UI::Core::CoreEngine>();
        ViewerResources::Init();
        m_imGuiEngine->initScreen<CustomMainScreen>();

        Core::AsyncJob::Get().PushTask(&CPU::Core::CoreLibManager::FindCoreLib, &CPU::Core::CoreLibManager::Get(), ViewerConstants::CORE_LIB_DIR);

        LOG_INFO("ViewerApplication: Init end");
    }

    ViewerApplication::~ViewerApplication() {
        LOG_INFO("ViewerApplication: destroy");
        ENGINE_PROFILE_FUNCTION();

        ViewerResources::Destroy();

#ifdef EXPORT_TRAD
        I18NImpl::Get().ExportTrad();
#endif

        LOG_INFO("ViewerApplication: destroy end");
    }

    void ViewerApplication::run() {
        LOG_INFO("ViewerApplication: run");

        m_imGuiEngine->Run();

        LOG_INFO("ViewerApplication: run end");
    }

}
