//
// Created by pierr on 26/03/2023.
//
#pragma once

#include <string>
#include <filesystem>

#include "ViewerApp/CoreLib/Windows/APanel.h"
#include "ViewerApp/CoreLib/Resources/TextEditor.h"
#include "ViewerApp/CoreLib/Assets/AssetMetadata.h"
#include "ViewerApp/Custom/Panels/Views/Editor/EditorItem.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"

namespace Astra::UI::App {

    class EditorPanel : public UI::Core::APanel
    {
    public:
        static constexpr const char* NAME = ICON_FA_PEN " Editor";

        EditorPanel();

        bool openFile(const UI::Core::AssetMetadata& fileMetadata, bool pSaveOpened = false, bool openReadOnly = false);

        void saveAllFiles();

    private:
        std::vector<Scope<EditorItem>> m_filesList;
        EditorItem* m_currentFile = nullptr;
        EditorItem* m_nextFile = nullptr;

        void drawMenuBar();

        void OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) override;

        void OnEvent(UI::Core::AEvent& pEvent) override;

        void drawEditorTabs();

        void drawPanelContent() override;
    };

}
