//
// Created by pierr on 06/08/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Resources/TextEditor.h"
#include "EditorItem.h"

namespace Astra::UI::App {

    class FileEditorItem : public EditorItem
    {
    private:
        UI::Core::TextEditor m_textEditor;

    public:
        explicit FileEditorItem(const UI::Core::AssetMetadata& fileMetadata);

        void Render(ImVec2 pSize) override;
        void DrawMenuBar() override;
        void DrawStatusBar() override;
        void SaveFile() override;
        void SetReadOnly(bool val) override;

    private:
        void drawMenuBarFile();
        void drawMenuBarEdit();
        void drawMenuBarView();
    };

}
