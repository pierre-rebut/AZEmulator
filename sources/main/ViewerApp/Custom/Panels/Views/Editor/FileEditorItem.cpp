//
// Created by pierr on 06/08/2023.
//
#include "FileEditorItem.h"
#include "Commons/Profiling.h"
#include "Commons/Log.h"
#include "ViewerApp/CoreLib/CoreEngine.h"
#include "ViewerApp/CoreLib/Events/NotificationEvent.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "ViewerApp/CoreLib/System/I18N.h"
#include "imgui_markdown.h"
#include "ViewerApp/CoreLib/Plateform/FileManager.h"

#include <fstream>

namespace Astra::UI::App {
    FileEditorItem::FileEditorItem(const UI::Core::AssetMetadata& fileMetadata)
            : EditorItem(fileMetadata) {
        if (fileMetadata.FilePath.empty()) {
            return;
        }

        switch (fileMetadata.Type) {
            case UI::Core::AssetType::ASM: {
                m_textEditor.SetLanguageDefinition(UI::Core::Editor::LanguageDefinition::ASM());
                m_textEditor.SetShowWhitespaces(false);
                break;
            }
            case UI::Core::AssetType::CPP: {
                m_textEditor.SetLanguageDefinition(UI::Core::Editor::LanguageDefinition::CPlusPlus());
                m_textEditor.SetShowWhitespaces(false);
                break;
            }
            default:
                break;
        }

        std::ifstream file(UI::Core::AssetMetadata::GetFileSystemPath(fileMetadata));
        if (file.good()) {
            std::stringstream strStream;
            strStream << file.rdbuf();

            m_textEditor.SetText(strStream.str());

            file.close();
        }
    }

    static void LinkCallback(ImGui::MarkdownLinkCallbackData data_) {
        std::string url(data_.link, data_.linkLength);
        if (!data_.isImage) {
            Core::FileManager::OpenExternally(url);
        }
    }

    static inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data_) {
        // In your application you would load an image based on data_ input. Here we just use the imgui font texture.
        ImTextureID image = ImGui::GetIO().Fonts->TexID;
        // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
        ImGui::MarkdownImageData imageData;
        imageData.isValid = true;
        imageData.useLinkCallback = false;
        imageData.user_texture_id = image;
        imageData.size = ImVec2(40.0f, 20.0f);

        // For image resize when available size.x > image width, add
        ImVec2 const contentSize = ImGui::GetContentRegionAvail();
        if (imageData.size.x > contentSize.x) {
            float const ratio = imageData.size.y / imageData.size.x;
            imageData.size.x = contentSize.x;
            imageData.size.y = contentSize.x * ratio;
        }

        return imageData;
    }

    static void ExampleMarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_) {
        // Call the default first so any settings can be overwritten by our implementation.
        // Alternatively could be called or not called in a switch statement on a case by case basis.
        // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
        ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

        switch (markdownFormatInfo_.type) {
            // example: change the colour of heading level 2
            case ImGui::MarkdownFormatType::HEADING: {
                if (markdownFormatInfo_.level == 2) {
                    if (start_) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    } else {
                        ImGui::PopStyleColor();
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    void FileEditorItem::Render(ImVec2 pSize) {
        if (!m_textEditor.IsReadOnly() || metadata.Type != UI::Core::AssetType::TEXT) {
            m_textEditor.Render("TextEditor", pSize);
            m_isModified = m_isModified || m_textEditor.IsTextChanged();
        } else {
            ImGui::MarkdownConfig mdConfig;

            // todo amelioration markdown

            const auto& fonts = ImGui::GetIO().Fonts->Fonts;

            mdConfig.linkCallback = LinkCallback;
            mdConfig.tooltipCallback = nullptr;
            mdConfig.imageCallback = ImageCallback;
            mdConfig.linkIcon = ICON_FA_LINK;
            mdConfig.headingFormats[0] = {fonts[Core::Fonts::LARGE], true};
            mdConfig.headingFormats[1] = {fonts[Core::Fonts::BOLD], true};
            mdConfig.headingFormats[2] = {fonts[Core::Fonts::SMALL], false};
            mdConfig.userData = nullptr;
            mdConfig.formatCallback = ExampleMarkdownFormatCallback;

            const auto fileText = m_textEditor.GetText();
            ImGui::BeginChild("##markdown", pSize);
            ImGui::Markdown(fileText.c_str(), fileText.length(), mdConfig);
            ImGui::EndChild();
        }
    }

    void FileEditorItem::DrawMenuBar() {
        if (ImGui::BeginMenu(I18N::Get("FILE"))) {
            drawMenuBarFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(I18N::Get("EDIT"))) {
            drawMenuBarEdit();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(I18N::Get("VIEW"))) {
            drawMenuBarView();
            ImGui::EndMenu();
        }
    }

    void FileEditorItem::drawMenuBarView() {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::MenuItem("Dark palette")) {
            m_textEditor.SetPalette(UI::Core::TextEditor::GetDarkPalette());
        }
        if (ImGui::MenuItem("Light palette")) {
            m_textEditor.SetPalette(UI::Core::TextEditor::GetLightPalette());
        }
        if (ImGui::MenuItem("Retro blue palette")) {
            m_textEditor.SetPalette(UI::Core::TextEditor::GetRetroBluePalette());
        }
    }

    void FileEditorItem::drawMenuBarEdit() {
        ENGINE_PROFILE_FUNCTION();

        bool ro = m_textEditor.IsReadOnly();
        if (ImGui::MenuItem(I18N::Get("READ_ONLY"), nullptr, &ro)) {
            m_textEditor.SetReadOnly(ro);
        }
        ImGui::Separator();

        if (ImGui::MenuItem(I18N::Get("UNDO"), "ALT-Backspace", nullptr, !ro && m_textEditor.CanUndo())) {
            m_textEditor.Undo();
        }
        if (ImGui::MenuItem(I18N::Get("REDO"), "Ctrl-Y", nullptr, !ro && m_textEditor.CanRedo())) {
            m_textEditor.Redo();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(I18N::Get("COPY"), "Ctrl-C", nullptr, m_textEditor.HasSelection())) {
            m_textEditor.Copy();
        }
        if (ImGui::MenuItem(I18N::Get("CUT"), "Ctrl-X", nullptr, !ro && m_textEditor.HasSelection())) {
            m_textEditor.Cut();
        }
        if (ImGui::MenuItem(I18N::Get("DELETE"), "Del", nullptr, !ro && m_textEditor.HasSelection())) {
            m_textEditor.Delete();
        }
        if (ImGui::MenuItem(I18N::Get("PASTE"), "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) {
            m_textEditor.Paste();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(I18N::Get("SELECT_ALL"), nullptr, nullptr)) {
            m_textEditor.SetSelection(UI::Core::Editor::Coordinates(), UI::Core::Editor::Coordinates(m_textEditor.GetTotalLines(), 0));
        }
    }

    void FileEditorItem::drawMenuBarFile() {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::MenuItem(I18N::Get("SAVE"), "Ctrl + S")) {
            SaveFile();
        }
    }

    void FileEditorItem::SaveFile() {
        ENGINE_PROFILE_FUNCTION();

        if (!m_isModified) {
            return;
        }

        LOG_DEBUG("[FileEditorItem] Saving file : {}", metadata.FilePath);

        auto textToSave = m_textEditor.GetText();
        const auto path = AstraProject::CurrentProject()->rootDirectory / metadata.FilePath;

        std::ofstream file(path);
        if (file.is_open()) {
            file << textToSave;
        }
        file.close();

        UI::Core::Events::Get().OnEvent<UI::Core::NotificationEvent>(
                AstraMessage::New2(AstraMessageType::Info, I18N::Get("FILE_SAVED"), metadata.FilePath.filename()));
        m_isModified = false;
    }

    void FileEditorItem::DrawStatusBar() {
        auto cpos = m_textEditor.GetCursorPosition();

        ImGui::Text("%6d/%-6d %6d %s", cpos.mLine + 1, cpos.mColumn + 1,
                    m_textEditor.GetTotalLines(), I18N::Get("LINES"));

        std::string str = myFormat("{} | {}",
                                   m_textEditor.IsOverwrite() ? "Ovr" : "Ins",
                                   m_textEditor.GetLanguageDefinition().mName.c_str());

        ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(str.c_str()).x - 10);
        ImGui::Text(str.c_str());
    }

    void FileEditorItem::SetReadOnly(bool val) {
        m_textEditor.SetReadOnly(val);
    }
}
