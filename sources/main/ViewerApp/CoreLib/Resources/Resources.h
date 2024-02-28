//
// Created by pierr on 16/03/2023.
//
#pragma once

#include <filesystem>

#include "Texture.h"

#include "Commons/utils/Singleton.h"
#include "Commons/Log.h"

#include "imgui.h"

namespace Astra::UI::Core::Fonts {

    enum FontsName
    {
        DEFAULT = 0,
        SMALL,
        BOLD,
        MONO_BOLD,
        LARGE
    };

}
