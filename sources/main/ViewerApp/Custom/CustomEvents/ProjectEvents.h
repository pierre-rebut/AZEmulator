//
// Created by pierr on 26/03/2023.
//
#pragma once

#include <filesystem>
#include <utility>

#include "ViewerApp/CoreLib/Events/AEvent.h"
#include "Commons/format.h"

namespace Astra::UI::App {
    class ProjectLoadedEvent : public UI::Core::AEvent
    {
    public:

        std::string toString() const override {
            return myFormat("ProjectLoadedEvent");
        }

        UI::Core::EventCategory GetCategoryFlag() const override {
            return UI::Core::EventCategory::EventCategoryCustom;
        }

        UI::Core::EventType GetEventType() const override {
            return UI::Core::EventType::ProjectLoaded;
        }
    };

    class CpuChangedEvent : public UI::Core::AEvent
    {
    public:
        std::string toString() const override {
            return myFormat("CpuChangedEvent");
        }

        UI::Core::EventCategory GetCategoryFlag() const override {
            return UI::Core::EventCategory::EventCategoryCustom;
        }

        UI::Core::EventType GetEventType() const override {
            return UI::Core::EventType::CpuChanged;
        }
    };

}
