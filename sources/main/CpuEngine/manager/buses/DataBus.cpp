//
// Created by pierr on 25/08/2023.
//

#include "DataBus.h"

#include "CpuEngine/manager/devices/DevicesManager.h"

namespace Astra::CPU::Core {

    DataBus::DataBus(const BusCreateData& busInfo) : IDevice(busInfo.uuid, m_busName), m_busName(busInfo.name), m_busSize(busInfo.size) {
        m_ramMemory = CreateScope<RamMemory>(m_busSize);
        m_ramMemory->Clear();
        m_ramMemory->SetReadOnly(busInfo.isReadOnly);

        const auto& deviceManager = DevicesManager::Get();

        for (const auto& [deviceUUID, address, internalAddress]: busInfo.connectedDevices) {
            auto device = deviceManager.GetDeviceUUID(deviceUUID);
            if (!device) {
                LOG_CPU_WARN("[DataBus] Invalid device {} connection for bus {}, skipped", deviceUUID, deviceUUID);
                continue;
            }

            ConnectDevice(address, device, internalAddress);
        }
    }

    void DataBus::ConnectDevice(size_t address, const Ref<Device>& newDevice, int index) {
        if (newDevice->GetDeviceAddressList().size() <= index) {
            return;
        }

        const auto& [addressSize, internalAddress] = newDevice->GetDeviceAddressList()[index];

        ConnectedDevice newConnectedDevice(address, address + addressSize - 1, internalAddress, index, newDevice);

        if (!isConnectionValid(newConnectedDevice)) {
            NOTIFY_ERROR("Bus connect device failed", "Invalid device connection for {} at index {}", newDevice->GetName(), index);
            return;
        }

        auto pos = std::ranges::find_if(m_connectedDevices, [&newConnectedDevice](const auto& connectedDevice) {
            return newConnectedDevice.addressLow < connectedDevice.addressLow;
        });

        m_connectedDevices.insert(pos, newConnectedDevice);
    }

    bool DataBus::RefreshDeviceConnections(const Ref<Device>& device) {
        LOG_CPU_DEBUG("[DataBusManager] RefreshDeviceConnections {} (UUID: {})", m_busName, deviceUUID);

        bool isError = false;

        for (auto it = m_connectedDevices.begin(); it != m_connectedDevices.end();) {
            auto& currentConnection = *it;
            if (currentConnection.device == device) {
                auto addressHigh = (currentConnection.addressLow + device->GetDeviceAddressList()[currentConnection.index].first - 1);

                const auto nextConnection = std::next(it);
                if (addressHigh >= m_busSize || (nextConnection != m_connectedDevices.end() && nextConnection->addressLow < addressHigh)) {
                    it = m_connectedDevices.erase(it);
                    auto logMsg = AstraMessage::New(AstraMessageType::Warning);
                    logMsg->setTitle("DataBus {} (UUID : {}) refresh device {} (UUID {})", m_busName, deviceUUID, device->GetName(), device->deviceUUID);
                    logMsg->setContent("Invalid refresh : new address oversize bus size or another device overlap the new address size");
                    LogManager::Get().Notify(logMsg);
                    LOG_CPU_WARN("[DataBusManager] Address oversize or overlap bus data");
                    isError = true;
                } else {
                    currentConnection.addressHigh = addressHigh;
                    it++;
                }
            } else {
                it++;
            }
        }

        LOG_CPU_DEBUG("[DataBusManager] RemoveDeviceLink END");
        return isError;
    }

    void DataBus::DisconnectDevice(const Ref<Device>& deviceToDisconnect) {
        std::erase_if(m_connectedDevices, [&deviceToDisconnect](const auto& connection) {
            return connection.device == deviceToDisconnect;
        });
    }

    void DataBus::DisconnectDevice(const ConnectedDevice& connectedDevice) {
        std::erase_if(m_connectedDevices, [&connectedDevice](const auto& connection) {
            return connection.addressLow == connectedDevice.addressLow;
        });
    }

    LARGE DataBus::Fetch(DataFormat fmt, size_t address) {
        auto [newAddress, device] = getDeviceByAddress(address);
        return device->Fetch(fmt, newAddress);
    }

    void DataBus::Push(DataFormat fmt, size_t address, LARGE value) {
        auto [newAddress, device] = getDeviceByAddress(address);
        device->Push(fmt, newAddress, value);
    }

    bool DataBus::isConnectionValid(const ConnectedDevice& newDevice) const {
        if (newDevice.addressLow > newDevice.addressHigh || newDevice.addressHigh >= m_busSize) {
            return false;
        }

        for (const auto& device: m_connectedDevices) {
            auto isInvalid = false;
            isInvalid |= device.isAddressValid(newDevice.addressLow);
            isInvalid |= device.isAddressValid(newDevice.addressHigh);
            isInvalid |= newDevice.isAddressValid(device.addressLow);
            isInvalid |= newDevice.isAddressValid(device.addressHigh);

            if (isInvalid) {
                return false;
            }
        }

        return true;
    }

    std::pair<size_t, IComObject*> DataBus::getDeviceByAddress(size_t address) const {
        for (const auto& device: m_connectedDevices) {
            if (device.addressLow > address) {
                break;
            }

            if (device.isAddressValid(address)) {
                return {
                        address - device.addressLow + device.internalAddress, device.device.get()
                };
            }
        }

        return {
                address, m_ramMemory.get()
        };
    }

    std::string DataBus::toString() const {
        return myFormat("DataBus<{}>", deviceUUID);
    }

    void DataBus::Reset() {
        m_ramMemory->Clear();
    }
}