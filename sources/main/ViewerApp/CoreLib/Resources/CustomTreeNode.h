//
// Created by pierr on 16/03/2023.
//
#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "Texture.h"
#include "EngineLib/data/Base.h"


namespace Astra::UI::Core {

    bool TreeNode(const std::string& id, const std::string& label, ImGuiTreeNodeFlags flags = 0, const Ref<UI::Core::Texture>& icon = nullptr);

    bool TreeNodeWithIcon(const Ref<Texture>& icon, ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end,
                          ImColor iconTint = IM_COL32_WHITE);
    bool TreeNodeWithIcon(const Ref<Texture>& icon, const void* ptr_id, ImGuiTreeNodeFlags flags, ImColor iconTint, const char* fmt, ...);

    bool TreeNodeWithIcon(const Ref<Texture>& icon, const char* label, ImGuiTreeNodeFlags flags, ImColor iconTint = IM_COL32_WHITE);
}
