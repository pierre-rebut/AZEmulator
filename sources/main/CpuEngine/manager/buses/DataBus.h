//
// Created by pierr on 25/08/2023.
//

#pragma once

#include <list>

#include "EngineLib/data/Base.h"
#include "Commons/AObject.h"

#include "EngineLib/data/IDevice.h"
#include "CpuEngine/data/BusCreateData.h"
#include "CpuEngine/engine/Device.h"
#include "CpuEngine/engine/hardwareDevices/RamMemory.h"

namespace Astra::CPU::Core {
    struct ConnectedDevice
    {
        size_t addressLow;
        size_t addressHigh;
        size_t internalAddress;
        int index;
        Ref<Device> device;

        inline bool isAddressValid(size_t address) const {
            return address >= addressLow && address <= addressHigh;
        }
    };

    class DataBus : public IDevice, public AObject
    {
    private:
        std::string m_busName;
        size_t m_busSize;
        std::list<ConnectedDevice> m_connectedDevices;
        Scope<RamMemory> m_ramMemory;

    public:
        explicit DataBus(const BusCreateData& busInfo);

        void ConnectDevice(size_t address, const Ref<Device>& newDevice, int index);
        void DisconnectDevice(const Ref<Device>& device);
        void DisconnectDevice(const ConnectedDevice& connectedDevice);

        inline const std::string& GetName() const { return m_busName; }

        inline bool IsReadOnly() const {return m_ramMemory->IsReadOnly();}
        inline void SetReadOnly(bool readOnly) const {m_ramMemory->SetReadOnly(readOnly);}

        void SetName(const std::string_view& name) { m_busName = name; }

        inline size_t GetBusSize() const { return m_busSize; }

        const std::list<ConnectedDevice>& GetLinkedDevices() const { return m_connectedDevices; }

        const RamMemory& GetRamMemory() const { return *m_ramMemory; }

        LARGE Fetch(DataFormat fmt, size_t address) override;

        void Push(DataFormat fmt, size_t address, LARGE value) override;

        void Reset();

        bool RefreshDeviceConnections(const Ref<Device>& device);

        std::string toString() const override;

    private:
        std::pair<size_t, IComObject*> getDeviceByAddress(size_t address) const;
        bool isConnectionValid(const ConnectedDevice& newDevice) const;
    };
}