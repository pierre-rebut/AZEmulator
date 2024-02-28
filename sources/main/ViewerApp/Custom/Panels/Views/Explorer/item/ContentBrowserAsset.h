//
// Created by pierr on 10/08/2023.
//
#pragma once

#include "ContentBrowserItem.h"

namespace Astra::UI::App {

    class ContentBrowserAsset : public ContentBrowserItem
    {
    public:
        ContentBrowserAsset(const UI::Core::AssetMetadata& assetInfo, const Ref<UI::Core::Texture>& icon);

        const UI::Core::AssetMetadata& GetAssetInfo() const { return m_AssetInfo; }

        void Delete() override;
        bool Move(const std::filesystem::path& destination) override;

    private:
        void Activate(CBItemActionResult& actionResult) override;
        void OnRenamed(const std::string& newName) override;
        void OnDragDropItem() const override;
    public:
        void PasteCopiedAssets(const std::filesystem::path& originalFilePath) override;

    private:
        UI::Core::AssetMetadata m_AssetInfo;
    };

}
