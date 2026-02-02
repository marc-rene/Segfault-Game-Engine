#include "CommandQueue.hpp"

#include "../Include/DaVinci.hpp"

/*
inline void ThrowIfFailed(HRESULT hr, std::string p_message = "")
{
    if (FAILED(hr))
    {
        if (p_message.empty())
        {
            ENGINE::GRAPHICS::DaVinci::Get_Instance()->Error("Error with CommandQueue!");
        }
        else
        {
            ENGINE::GRAPHICS::DaVinci::Get_Instance()
                ->Error(std::format("Error with CommandQueue! : {}", p_message));
        }
        throw std::exception();
    }
}


CommandQueue::CommandQueue(ComPtr<ID3D12Device2> dx_device, D3D12_COMMAND_LIST_TYPE type)
    : m_fence_value(0)
      , m_command_list_type(type)
      , m_d3d12_device(dx_device)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(m_d3d12_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12_command_queue)),
                  "Failed to create a command queue with CreateCommandQueue()");

    ThrowIfFailed(m_d3d12_device->CreateFence(m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12_fence)),
                  "Failed to create a Fence with the CreateFence()");

    m_fence_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_fence_event && "Failed to create fence event handle.");
}

CommandQueue::~CommandQueue()
{
    m_d3d12_command_queue->Release();

}


ComPtr<ID3D12GraphicsCommandList2> CommandQueue::Get_Command_List()
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList2> commandList;


    if (!m_command_allocator_queue.empty() && Is_Fence_Complete(m_command_allocator_queue.front().fence_value))
    {
        commandAllocator = m_command_allocator_queue.front().command_allocator;
        m_command_allocator_queue.pop();

        ThrowIfFailed(commandAllocator->Reset(),
                      "Failed to reset command allocator... in Get_Command_List()");
    }
    else
    {
        commandAllocator = create_command_allocator();
    }


    if (!m_command_list_queue.empty())
    {
        commandList = m_command_list_queue.front();
        m_command_list_queue.pop();

        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr),
                      "Failed to reset command list... in Get_Command_List()");
    }
    else
    {
        commandList = create_command_list(commandAllocator);
    }


    ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()),
                  "Failed to set the command list private data interface in Get_Command_List()");


    return commandList;
}


uint64_t CommandQueue::Execute_Command_List(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandAllocator* commandAllocator;
    UINT dataSize = sizeof(commandAllocator);
    ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator),
                  "Failed to get the CommandList data interface GetPrivateData() in Execute_Command_List()");


    ID3D12CommandList* const ppCommandLists[] =
    {
        commandList.Get()
    };


    m_d3d12_command_queue->ExecuteCommandLists(1, ppCommandLists);
    uint64_t fenceValue = Signal();

    m_command_allocator_queue.emplace(command_allocator_entry{fenceValue, commandAllocator});
    m_command_list_queue.push(commandList);
    
    // The ownership of the command allocator has been transferred to the ComPtr
    // in the command allocator queue. It's safe to release the reference 
    // in this temporary COM pointer here.
    commandAllocator->Release();
 
    return fenceValue;
}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValue = ++m_fence_value;
    m_d3d12_command_queue->Signal(m_d3d12_fence.Get(), fenceValue);
    return fenceValue;
}


bool CommandQueue::Is_Fence_Complete(uint64_t fenceValue)
{
    return m_d3d12_fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::Wait_For_Fence_Value(uint64_t fenceValue)
{
    if (!Is_Fence_Complete(fenceValue))
    {
        m_d3d12_fence->SetEventOnCompletion(fenceValue, m_fence_event);
        ::WaitForSingleObject(m_fence_event, DWORD_MAX);
    }
}

void CommandQueue::Flush()
{
    Wait_For_Fence_Value(Signal());
}

ComPtr<ID3D12CommandQueue> CommandQueue::Get_D3D12_Command_Queue() const
{
    return m_d3d12_command_queue;
}

ComPtr<ID3D12CommandAllocator> CommandQueue::create_command_allocator()
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(m_d3d12_device->CreateCommandAllocator(m_command_list_type, IID_PPV_ARGS(&commandAllocator)),
                  "CommandQueue failed with CreateCommandAllocator()... Investigate create_command_allocator()");

    return commandAllocator;
}


ComPtr<ID3D12GraphicsCommandList2> CommandQueue::create_command_list(ComPtr<ID3D12CommandAllocator> allocator)
{
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    ThrowIfFailed(
        m_d3d12_device->CreateCommandList(0, m_command_list_type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)),
        "CommandQueue failed with CreateCommandList()... Investigate create_command_list()");

    return commandList;
}
*/