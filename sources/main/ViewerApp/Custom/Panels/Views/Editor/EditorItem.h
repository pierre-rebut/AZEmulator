//
// Created by pierr on 31/03/2023.
//
#pragma once

#include <filesystem>
#include <utility>
#include "ViewerApp/CoreLib/Assets/AssetMetadata.h"
#include "imgui.h"

namespace Astra::UI::App {
    class EditorItem
    {
    protected:
        bool m_isModified = false;

    public:
        UI::Core::AssetMetadata metadata;

        explicit EditorItem(UI::Core::AssetMetadata fileMetadata) : metadata(std::move(fileMetadata)) {}

        virtual ~EditorItem() = default;
        virtual void Render(ImVec2 pSize) = 0;
        virtual void DrawMenuBar() { /* do nothing */ }
        virtual void DrawStatusBar() { /* do nothing */ }
        virtual void SaveFile() { /* do nothing*/ }
        virtual void SetReadOnly(bool val) { /* do nothing */ }

        bool isModified() const {return m_isModified;}
    };

}
