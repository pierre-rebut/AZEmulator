//
// Created by pierr on 06/08/2023.
//
#include "ImageEditorItem.h"

namespace Astra::UI::App {
    ImageEditorItem::ImageEditorItem(const UI::Core::AssetMetadata& fileMetadata) : EditorItem(fileMetadata) {
        texture = CreateRef<UI::Core::Texture>(UI::Core::AssetMetadata::GetFileSystemPath(fileMetadata));
    }

    void ImageEditorItem::Render(ImVec2 pSize) {
        ImGui::Image(texture->textureId(), pSize);
    }
}
