# How to create a new core library

## What is a core library

A core library is an external lib, .dll (for windows), .so (for linux).

It's composed of two main class :
- **a Factory** : used to create core engine, must inherit from Astra::CPU::IFactory
- **a Core** : created by the factory, used as the core for engine, must inherit from Astra::CPU::ICpuCore

The build library file must be placed in a CoreLibrary/<category> directory in the emulator executable folder

## IFactory

- Constructor(RunInfo) : Must set the CoreConfigInfo, with information about the factory, RunInfo determine if the core is runnable
- CreateNewCore(IEntitiesService*) : Return a unique_ptr instance of the ICpuCore

## ICpuCore

- IsInit() : Used to check if the core is correctly initiated, can't run emulation if one of the core return false
- IsComplete() : Used for step mode, to check if an instruction is complete (including cycles)
- Reset() : Call when Reset button is pressed, allow internal value resetting
- Execute() : Only for runnable factory, execute one cycle
- Interrupt(bool, int) : Called by other components to generate interruption, bool define if interruption is a non maskable interrupt
- DebugExecute() : Used by the debug mode only, to retrieve data for each step. The factory config must define columns in debugHeader var
- LoadFromStream(istream) : Called when a file is drag&drop on the engine name (on the config panel)
- UpdateHardParameters(vector) : Called when hard parameters changed. The factory config must define columns in hardParameters var
- GetDeviceAddressList() : Used to define the different address space of the core. Called by DataBus (memory)
- Fetch(DataFormat, size_t) : Called by databus or other devices to fetch data, return a LARGE type
- Push(DataFormat, size_t, LARGE) : Called by databus or other devices to push data
- FetchReadOnly(size_t) : Used by Memory Panel to fetch data and display the hex table, must not modify internal data, return -1 to display XX
