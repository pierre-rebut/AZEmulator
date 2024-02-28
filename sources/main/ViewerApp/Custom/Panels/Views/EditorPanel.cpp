//
// Created by pierr on 26/03/2023.
//
#include "EditorPanel.h"

#include "Commons/Profiling.h"
#include "imgui.h"
#include "ViewerApp/Custom/AstraProject.h"
#include "Commons/format.h"
#include "ViewerApp/Custom/CustomEvents/EditorEvents.h"
#include "ViewerApp/Custom/Panels/Views/Editor/EditorItem.h"
#include "ViewerApp/CoreLib/Assets/AssetManager.h"
#include "ViewerApp/CoreLib/Events/AssetEvents.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"
#include "ViewerApp/Custom/Panels/Views/Editor/FileEditorItem.h"
#include "ViewerApp/Custom/Panels/Views/Editor/ImageEditorItem.h"
#include "Commons/Log.h"

namespace Astra::UI::App {
    EditorPanel::EditorPanel() : APanel(NAME, UI::Core::PanelType::VIEW) {
    }

    void EditorPanel::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        if (!m_isOpen || (UI::Core::Project::CurrentProject()->isIsFullScreen())) { return; }

        ENGINE_PROFILE_FUNCTION();
        if (ImGui::Begin(NAME, &m_isOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
            drawPanelContent();
        }

        ImGui::End();
    }

    void EditorPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        drawMenuBar();

        static const ImGuiTableFlags flags = ImGuiTabBarFlags_Reorderable |
                                             ImGuiTabBarFlags_FittingPolicyScroll;

        if (ImGui::BeginTabBar("##tabs", flags)) {
            drawEditorTabs();
            ImGui::EndTabBar();
        }

        if (m_currentFile) {
            m_currentFile->DrawStatusBar();
        }
    }

    void EditorPanel::drawEditorTabs() {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::TabItemButton(ICON_FA_PLUS, ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
            const auto& metadata = UI::Core:: AssetManager::Get().CreateNewAsset("New.txt");
            m_filesList.emplace_back(CreateScope<FileEditorItem>(metadata));
            m_nextFile = m_filesList.back().get();
        }

        std::erase_if(m_filesList, [this](const auto& file) {

            int tabFlags = file->isModified() ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None;
            if (m_nextFile == file.get()) {
                tabFlags |= ImGuiTabItemFlags_SetSelected;
                m_nextFile = nullptr;
            }

            bool closed = true;
            if (ImGui::BeginTabItem(file->metadata.FilePath.string().c_str(), &closed, tabFlags)) {
                m_currentFile = file.get();

                const ImVec2& regionAvail = ImGui::GetContentRegionAvail();
                file->Render(ImVec2(
                        regionAvail.x,
                        regionAvail.y - ImGui::GetTextLineHeightWithSpacing()
                ));

                ImGui::EndTabItem();
            }

            if (!closed) {
                auto& openedFiles = AstraProject::CurrentProject()->GetData().openedEditorFiles;
                openedFiles.erase(std::ranges::find(openedFiles.begin(), openedFiles.end(), file->metadata.Handle));
                if (m_currentFile == file.get()) {
                    m_currentFile = nullptr;
                }
                return true;
            }

            return false;
        });
    }

    void EditorPanel::drawMenuBar() {
        ENGINE_PROFILE_FUNCTION();

        if (!m_currentFile) {
            return;
        }

        if (ImGui::BeginMenuBar()) {
            m_currentFile->DrawMenuBar();
            ImGui::EndMenuBar();
        }
    }

    void EditorPanel::OnEvent(UI::Core::AEvent& pEvent) {
        switch (pEvent.GetEventType()) {
            case UI::Core::EventType::ProjectLoaded: {
                m_currentFile = nullptr;
                saveAllFiles();
                m_filesList.clear();

                std::erase_if(AstraProject::CurrentProject()->GetData().openedEditorFiles, [this](const auto& fileUUID) {
                    const auto& metadata = UI::Core:: AssetManager::Get().GetMetadata(fileUUID);
                    return !openFile(metadata);
                });

                break;
            }
            case UI::Core::EventType::EditorOpen: {
                pEvent.Handled = true;
                const auto& editorEvt = dynamic_cast<const EditorOpenEvent&>(pEvent);
                openFile(editorEvt.metadata, true, editorEvt.readOnly);
                ImGui::SetWindowFocus(NAME);
                break;
            }
            case UI::Core::EventType::AssetChanged: {
                const auto& assetEvt = dynamic_cast<const UI::Core::AssetChangedEvent&>(pEvent);
                auto it = std::ranges::find_if(m_filesList.begin(), m_filesList.end(),
                                               [&assetEvt](const std::vector<Scope<EditorItem>>::value_type& elem) {
                                                   return elem->metadata.Handle == assetEvt.metadata.Handle;
                                               });

                if (it != m_filesList.end()) {
                    switch (assetEvt.eventType) {
                        case UI::Core::AssetChangedEvent::Event::ASSET_DELETE: {
                            m_filesList.erase(it);
                            m_currentFile = nullptr;
                            break;
                        }
                        case UI::Core::AssetChangedEvent::Event::ASSET_RENAMED: {
                            (*it)->metadata = assetEvt.metadata;
                            break;
                        }
                    }
                }
                break;
            }
            case UI::Core::EventType::WindowFocus: {
                const auto& windowFocusEvt = dynamic_cast<const UI::Core::WindowFocusEvent&>(pEvent);
                if (!windowFocusEvt.isFocus()) {
                    saveAllFiles();
                }
                break;
            }
            default:
                break;
        }
    }

    void EditorPanel::saveAllFiles() {
        LOG_DEBUG("EditorPanel: saveAllFiles");

        for (const auto& file: m_filesList) {
            file->SaveFile();
        }

        LOG_DEBUG("EditorPanel: saveAllFiles END");
    }

    bool EditorPanel::openFile(const UI::Core::AssetMetadata& fileMetadata, bool pSaveOpened, bool openReadOnly) {
        LOG_INFO("[EditorPanel] Opening file {}", fileMetadata.FilePath.string());
        if (!fileMetadata.isValid()) {
            LOG_WARN("[EditorPanel] Invalid metadata, can not open file. Skipped");
            return false;
        }

        auto it = std::ranges::find_if(m_filesList.begin(), m_filesList.end(),
                                       [&fileMetadata](const Scope<EditorItem>& elem) {
                                           return elem->metadata.Handle == fileMetadata.Handle;
                                       });

        if (it != m_filesList.end()) {
            m_nextFile = it->get();
            return true;
        } else if (!std::filesystem::exists(UI::Core::AssetMetadata::GetFileSystemPath(fileMetadata))) {
            LOG_WARN("[EditorPanel] File not found {}", fileMetadata.FilePath);
            return false;
        }

        try {
            switch (fileMetadata.Type) {
                using enum UI::Core::AssetType;
                case TEXT:
                case CONFIG:
                case CPP:
                case ASM: {
                    m_filesList.emplace_back(CreateScope<FileEditorItem>(fileMetadata));
                    break;
                }
                case IMAGE: {
                    m_filesList.emplace_back(CreateScope<ImageEditorItem>(fileMetadata));
                    break;
                }
                default: {
                    LOG_WARN("[EditorPanel] Type {} not supported", UI::Core::AssetMetadata::AssetTypeToString(fileMetadata.Type));
                    return false;
                }
            }

            if (pSaveOpened) {
                AstraProject::CurrentProject()->GetData().openedEditorFiles.emplace_back(fileMetadata.Handle);
            }

            m_nextFile = m_filesList.back().get();
            m_nextFile->SetReadOnly(openReadOnly);
        } catch (const std::exception& e) {
            LOG_ERROR("[EditorPanel] can not open file {}: {}", fileMetadata.FilePath, e.what());
            return false;
        }

        LOG_DEBUG("File opened");
        return true;
    }

}
