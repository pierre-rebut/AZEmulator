//
// Created by pierr on 19/01/2024.
//
#pragma once

#include "CpuEngine/data/DeviceCreateData.h"
#include "CpuEngine/engine/hardwareDevices/HardwareDevice.h"

namespace Astra::CPU::Core {

    class DiskDevice : public HardwareDevice
    {
    private:
        UUID m_diskId = 0;
        std::filesystem::path m_diskPath;
        size_t m_diskSize = 0;
        BYTE* m_diskData = nullptr;

        bool m_modified = false;
        bool m_readOnly = false;

        std::vector<std::pair<size_t, size_t>> m_addrList{};

    public:
        explicit DiskDevice(const DeviceCreateData& deviceInfo);
        ~DiskDevice() override;

        bool OpenDisk(UUID diskId, const std::filesystem::path& diskPath);
        void CloseDisk();

        void Push(DataFormat fmt, size_t address, LARGE val) override;
        LARGE Fetch(DataFormat fmt, size_t address) override;
        int FetchReadOnly(size_t address) const override;

        void SaveDisk();

        UUID GetCurrentDiskId() const {return m_diskId;}
        bool IsReadOnly() const {return m_readOnly;}
        void SetReadOnly(bool val) { m_readOnly = val;}

        const std::vector<std::pair<size_t, size_t>>& GetDeviceAddressList() const override { return m_addrList; };
    };

}
