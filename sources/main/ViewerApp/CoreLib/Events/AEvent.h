//
// Created by pierr on 13/03/2022.
//

#pragma once

#include "Commons/AObject.h"

namespace Astra::UI::Core {

    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowMinimize, WindowMoved, WindowDrop,
        SettingsUpdated, AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        CpuRun, CpuStop,

        DelayedAction, Notification,

        // custom event
        ProjectLoaded,
        EditorOpen,
        AssetChanged,
        CpuChanged
    };

    enum class EventCategory
    {
        EventCategoryApplication,
        EventCategoryInput,
        EventCategoryCustom
    };

    class AEvent : public AObject
    {
    public:
        bool Handled = false;

        virtual EventType GetEventType() const = 0;

        virtual EventCategory GetCategoryFlag() const = 0;
    };
}
