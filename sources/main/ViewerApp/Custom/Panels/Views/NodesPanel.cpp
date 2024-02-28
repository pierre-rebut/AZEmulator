//
// Created by pierr on 27/08/2023.
//

#include "NodesPanel.h"
#include "Commons/Profiling.h"
#include "imnodes.h"
#include "CpuEngine/manager/EngineManager.h"
#include "CpuEngine/manager/buses/DataBusManager.h"
#include "CpuEngine/manager/devices/DevicesManager.h"
#include "ViewerApp/CoreLib/IconsFontAwesome6.h"
#include "ViewerApp/Custom/AstraProject.h"

namespace Astra::UI::App {
    void NodesPanel::OnUpdate(const UI::Core::FrameInfo& pFrameInfo) {
        if (!m_isAppearing) {
            return;
        }

        m_nodeGraph.clear();

        int otherNodeId = 10000;
        std::unordered_map<UUID, Ref<NodeItem>> tmp;

        for (const auto& [deviceUUID, device]: CPU::Core::DevicesManager::Get().GetDevices()) {
            auto newNode = CreateRef<NodeItem>();

            newNode->nodeId = getIndexByUUID(deviceUUID);
            newNode->itemUUID = deviceUUID;
            newNode->name = myFormat("{} {}", getIconByDeviceType(device->type), device->GetName());

            newNode->pins.emplace_back(NodePin{otherNodeId++, "node", PinMode::INPUT});

            m_nodeGraph.emplace_back(newNode);
            tmp[deviceUUID] = newNode;
        }

        for (const auto& [busUUID, bus]: CPU::Core::DataBusManager::Get().GetDataBuses()) {
            auto newNode = CreateRef<NodeItem>();

            newNode->itemUUID = busUUID;
            newNode->nodeId = getIndexByUUID(busUUID);
            newNode->name = myFormat("{} {}", ICON_FA_MEMORY, bus->GetName());

            newNode->pins.emplace_back(NodePin{otherNodeId++, "cpu", PinMode::INPUT});
            newNode->pins.emplace_back(NodePin{otherNodeId++, "device", PinMode::OUTPUT});

            for (const auto& device: bus->GetLinkedDevices()) {
                newNode->connections.emplace_back(NodeLink{
                        otherNodeId++, newNode->pins.at(1).id, tmp.at(device.device->deviceUUID)->pins.at(0).id
                });
            }

            m_nodeGraph.emplace_back(newNode);
            tmp[busUUID] = newNode;
        }

        for (const auto& [engineUUID, engine]: CPU::Core::EngineManager::Get().getEngines()) {
            Ref<NodeItem> newNode = tmp.at(engineUUID);

            for (const auto& [devName, dev]: engine->GetEntities().GetDevices()) {
                int busPin = otherNodeId++;

                newNode->pins.emplace_back(NodePin{busPin, devName, PinMode::OUTPUT});

                if (const auto busValue = dev->GetValue()) {
                    newNode->connections.emplace_back(NodeLink{
                            otherNodeId++, busPin, tmp.at(busValue->deviceUUID)->pins.at(0).id
                    });
                }
            }
        }
    }

    void NodesPanel::OnImGuiRender(const UI::Core::FrameInfo& pFrameInfo) {
        m_isAppearing = false;

        if (!m_isOpen || (UI::Core::Project::CurrentProject()->isIsFullScreen())) { return; }

        if (ImGui::Begin(NAME, &m_isOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse)) {
            m_isAppearing = true;
            drawPanelContent();
        }

        ImGui::End();
    }

    void NodesPanel::drawPanelContent() {
        ENGINE_PROFILE_FUNCTION();

        if (ImGui::BeginMenuBar()) {

            if (ImGui::BeginMenu("CPU")) {
                //drawMenuBarFile();
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Memory")) {
                //drawMenuBarFile();
            }

            if (ImGui::BeginMenu("Devices")) {
                //drawMenuBarFile();
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImNodes::BeginNodeEditor();

        auto currentEngineUUID = AstraProject::CurrentProject()->getCurrentEngine() ? AstraProject::CurrentProject()->getCurrentEngine()->deviceUUID : 0;

        for (const auto& node: m_nodeGraph) {
            if (node->itemUUID == currentEngineUUID) {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(211, 10, 10, 255));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(211, 100, 10, 255));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(211, 148, 10, 255));
            }

            ImNodes::BeginNode(node->nodeId);
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node->name.c_str());
            ImNodes::EndNodeTitleBar();

            for (const auto& pin: node->pins) {
                if (pin.mode == PinMode::INPUT) {
                    ImNodes::BeginInputAttribute(pin.id);
                    ImGui::TextUnformatted(pin.name.c_str());
                    ImNodes::EndInputAttribute();
                } else {
                    ImNodes::BeginOutputAttribute(pin.id);
                    const float text_width = ImGui::CalcTextSize(pin.name.c_str()).x;
                    ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
                    ImGui::TextUnformatted(pin.name.c_str());
                    ImNodes::EndOutputAttribute();
                }
            }

            ImNodes::EndNode();

            if (node->itemUUID == currentEngineUUID) {
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }

            for (const auto& link: node->connections) {
                ImNodes::Link(link.id, link.pinIn, link.pinOut);
            }
        }

        ImNodes::EndNodeEditor();

        int start_attr, end_attr;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
            LOG_DEBUG("New link : {}, {}", start_attr, end_attr);
        }

    }

    int NodesPanel::getIndexByUUID(UUID uuid) {
        auto& projectNodeData = AstraProject::CurrentProject()->GetData().nodePanel;

        if (projectNodeData.nodeId.contains(uuid)) {
            return projectNodeData.nodeId.at(uuid);
        }

        auto id = projectNodeData.nodeCurrentIndex;
        projectNodeData.nodeCurrentIndex++;
        projectNodeData.nodeId[uuid] = id;
        return id;
    }

    const char* NodesPanel::getIconByDeviceType(CPU::Core::DeviceType type) {
        switch (type) {
            case CPU::Core::DeviceType::ENGINE:
                return ICON_FA_MICROCHIP;
            case CPU::Core::DeviceType::KEYBOARD:
                return ICON_FA_KEYBOARD;
            case CPU::Core::DeviceType::MOUSE:
                return ICON_FA_COMPUTER_MOUSE;
            case CPU::Core::DeviceType::SCREEN:
                return ICON_FA_DISPLAY;
            case CPU::Core::DeviceType::AUDIO:
                return ICON_FA_VOLUME_LOW;
            case CPU::Core::DeviceType::SERIAL:
                return ICON_FA_TERMINAL;
            case CPU::Core::DeviceType::DISK:
                return ICON_FA_HARD_DRIVE;
            default:
                return "";
        }
    }
} // Astra