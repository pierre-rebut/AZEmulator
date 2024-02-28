//
// Created by pierr on 06/08/2023.
//
#pragma once

#include "ViewerApp/CoreLib/Resources/Texture.h"
#include "EditorItem.h"
#include "EngineLib/data/Base.h"

namespace Astra::UI::App {

    class ImageEditorItem : public EditorItem
    {
    private:
        Ref<UI::Core::Texture> texture;

    public:
        explicit ImageEditorItem(const UI::Core::AssetMetadata& fileMetadata);

        void Render(ImVec2 pSize) override;
    };

}
