#pragma once

#include "../Include/DaVinci.hpp"

#include <cstdint>  // For uint64_t
#include <queue>    // For std::queue


// Wrapper Class for ID3D12CommandQueue
struct CommandQueue
{
    CommandQueue(ComPtr<ID3D12Device2> dx_device, D3D12_COMMAND_LIST_TYPE type);


    virtual ~CommandQueue();


    // Get an available command list from the command queue.
    ComPtr<ID3D12GraphicsCommandList2> Get_Command_List();


    // Execute a command list.
    // Returns the fence value to wait for this command list.
    uint64_t Execute_Command_List(ComPtr<ID3D12GraphicsCommandList2> commandList);


    uint64_t Signal();


    bool Is_Fence_Complete(uint64_t fenceValue);


    void Wait_For_Fence_Value(uint64_t fenceValue);


    void Flush();


    ComPtr<ID3D12CommandQueue> Get_D3D12_Command_Queue() const;

protected:
    ComPtr<ID3D12CommandAllocator> create_command_allocator();
    ComPtr<ID3D12GraphicsCommandList2> create_command_list(ComPtr<ID3D12CommandAllocator> allocator);

private:
    
    // Keep track of command allocators that are "in-flight"
    struct command_allocator_entry
    {
        uint64_t fence_value;
        ComPtr<ID3D12CommandAllocator> command_allocator;
    };

    using CommandAllocatorQueue = std::queue<command_allocator_entry>;
    using CommandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList2>>;

    D3D12_COMMAND_LIST_TYPE m_command_list_type;
    ComPtr<ID3D12Device2> m_d3d12_device;
    ComPtr<ID3D12CommandQueue> m_d3d12_command_queue;
    ComPtr<ID3D12Fence> m_d3d12_fence;
    HANDLE m_fence_event;
    uint64_t m_fence_value;

    CommandAllocatorQueue m_command_allocator_queue;
    CommandListQueue m_command_list_queue;
};
