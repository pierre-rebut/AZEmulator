//
// Created by pierr on 31/03/2023.
//
#pragma once

#include <filesystem>
#include <utility>
#include "ViewerApp/CoreLib/Events/AEvent.h"
#include "Commons/format.h"

namespace Astra::UI::App {
    class EditorOpenEvent : public UI::Core::AEvent
    {
    public:
        const UI::Core::AssetMetadata& metadata;
        const bool readOnly;

        explicit EditorOpenEvent(const UI::Core::AssetMetadata& pMetadata, bool isReadOnly = false) : metadata(pMetadata), readOnly(isReadOnly) {};

        std::string toString() const override {
            return myFormat("EditorOpenEvent<{}>", metadata.Handle);
        }

        UI::Core::EventCategory GetCategoryFlag() const override {
            return UI::Core::EventCategory::EventCategoryCustom;
        }

        UI::Core::EventType GetEventType() const override {
            return UI::Core::EventType::EditorOpen;
        }
    };
}
