//
// Created by pierr on 21/03/2023.
//
#pragma once

#include <filesystem>

#include "EngineLib/data/Base.h"
#include "Commons/AstraException.h"
#include "ViewerApp/CoreLib/Resources/Texture.h"

namespace Astra::UI::App {

    class ViewerResources
    {
    public:
        ViewerResources() = delete;

        // Generic
        inline static Ref<UI::Core::Texture> InfoIcon = nullptr;
        inline static Ref<UI::Core::Texture> WarningIcon = nullptr;
        inline static Ref<UI::Core::Texture> ErrorIcon = nullptr;
        inline static Ref<UI::Core::Texture> DebugIcon = nullptr;
        inline static Ref<UI::Core::Texture> SuccessIcon = nullptr;

        // Textures
        inline static Ref<UI::Core::Texture> LogoTexture = nullptr;
        inline static Ref<UI::Core::Texture> ShadowTexture = nullptr;

        // Window
        inline static Ref<UI::Core::Texture> MinimizeIcon = nullptr;
        inline static Ref<UI::Core::Texture> MaximizeIcon = nullptr;
        inline static Ref<UI::Core::Texture> RestoreIcon = nullptr;
        inline static Ref<UI::Core::Texture> CloseIcon = nullptr;

        // Content Browser
        inline static Ref<UI::Core::Texture> FolderIcon = nullptr;
        inline static Ref<UI::Core::Texture> FileIcon = nullptr;
        inline static Ref<UI::Core::Texture> WAVFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> PNGFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> ASMFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> DISKFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> ConfigFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> FontFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> BINARYFileIcon = nullptr;
        inline static Ref<UI::Core::Texture> DumpFileIcon = nullptr;

        // Running
        inline static Ref<UI::Core::Texture> RunIcon = nullptr;
        inline static Ref<UI::Core::Texture> SelectCpuIcon = nullptr;
        inline static Ref<UI::Core::Texture> StepIcon = nullptr;
        inline static Ref<UI::Core::Texture> StopIcon = nullptr;

        static void Init() {
            // Generic
            InfoIcon = LoadTexture("Generic/Info.png");
            WarningIcon = LoadTexture("Generic/Warning.png");
            ErrorIcon = LoadTexture("Generic/Error.png");
            DebugIcon = LoadTexture("Generic/Debug.png");
            SuccessIcon = LoadTexture("Generic/Success.png");
            ShadowTexture = LoadTexture("Generic/Shadow.png");

            // Window
            MinimizeIcon = LoadTexture("Window/Minimize.png");
            MaximizeIcon = LoadTexture("Window/Maximize.png");
            RestoreIcon = LoadTexture("Window/Restore.png");
            CloseIcon = LoadTexture("Window/Close.png");

            // Content Browser
            FolderIcon = LoadTexture("ContentBrowser/Folder.png");
            FileIcon = LoadTexture("ContentBrowser/File.png");
            WAVFileIcon = LoadTexture("ContentBrowser/WAV.png");
            PNGFileIcon = LoadTexture("ContentBrowser/PNG.png");
            ConfigFileIcon = LoadTexture("ContentBrowser/Config.png");
            FontFileIcon = LoadTexture("ContentBrowser/Font.png");
            BINARYFileIcon = LoadTexture("ContentBrowser/BIN.png");
            DumpFileIcon = LoadTexture("ContentBrowser/Dump.png");
            ASMFileIcon = LoadTexture("ContentBrowser/ASM.png");
            DISKFileIcon = LoadTexture("ContentBrowser/Disk.png");

            // Textures
            LogoTexture = LoadTexture("AppIcon.png");

            // Running
            RunIcon = LoadTexture("Viewport/Run.png");
            SelectCpuIcon = LoadTexture("Viewport/SelectCpuIcon.png");
            StepIcon = LoadTexture("Viewport/Step.png");
            StopIcon = LoadTexture("Viewport/Pause.png");
        }

        static void Destroy() {
            // Generic
            InfoIcon = nullptr;
            WarningIcon = nullptr;
            ErrorIcon = nullptr;
            DebugIcon = nullptr;
            SuccessIcon = nullptr;
            ShadowTexture = nullptr;

            // Window
            MinimizeIcon = nullptr;
            MaximizeIcon = nullptr;
            RestoreIcon = nullptr;
            CloseIcon = nullptr;

            // Content Browser
            FolderIcon = nullptr;
            FileIcon = nullptr;
            WAVFileIcon = nullptr;
            PNGFileIcon = nullptr;
            ConfigFileIcon = nullptr;
            FontFileIcon = nullptr;
            DISKFileIcon = nullptr;
            ASMFileIcon = nullptr;
            BINARYFileIcon = nullptr;
            DumpFileIcon = nullptr;

            // Textures
            LogoTexture = nullptr;

            // Running
            RunIcon = nullptr;
            SelectCpuIcon = nullptr;
            StepIcon = nullptr;
            StopIcon = nullptr;
        }

    private:
        static Ref<UI::Core::Texture> LoadTexture(const std::filesystem::path& pRelativePath) {
            std::filesystem::path path = std::filesystem::path("resources") / "icons" / pRelativePath;
            if (!std::filesystem::exists(path)) {
                throw AstraException("ViewerResources: invalid resources path {}", pRelativePath);
            }
            return CreateRef<UI::Core::Texture>(path);
        }
    };

}
