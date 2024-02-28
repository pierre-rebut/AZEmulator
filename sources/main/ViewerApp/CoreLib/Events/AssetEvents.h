//
// Created by pierr on 03/06/2023.
//
#pragma once

#include <string>
#include "Commons/format.h"
#include "ViewerApp/CoreLib/Events/AEvent.h"
#include "ViewerApp/CoreLib/Assets/AssetMetadata.h"

namespace Astra::UI::Core {
    class AssetChangedEvent : public AEvent
    {
    public:
        enum class Event
        {
            ASSET_DELETE,
            ASSET_RENAMED
        };

        const Event eventType;
        const AssetMetadata metadata;

        AssetChangedEvent(AssetMetadata pMetadata, Event pEvent) : eventType{pEvent}, metadata{std::move(pMetadata)} {}

        std::string toString() const override {
            return myFormat("AssetChangedEvent");
        }

        EventCategory GetCategoryFlag() const override {
            return EventCategory::EventCategoryCustom;
        }

        EventType GetEventType() const override {
            return EventType::AssetChanged;
        }
    };
}
