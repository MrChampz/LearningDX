#pragma once

#include "D3DUtil.h"
#include "GameTimer.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
protected:
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:
	static D3DApp* GetApp();

	HINSTANCE GetAppInst() const;
	HWND	  GetAppWnd() const;
	float	  GetAspectRatio() const;

	bool GetMSAAState() const;
	void SetMSAAState(bool value);

	int Run();

	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void OnResize();
	virtual void Update(const GameTimer& timer) = 0;
	virtual void Draw(const GameTimer& timer) = 0;

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CreateCommandObjects();
	void CreateSwapChain();
	virtual void CreateRtvAndDsvDescriptorHeaps();

	void FlushCommandQueue();

	ID3D12Resource* GetCurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

	void CalculateFrameStats();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:
	static D3DApp* m_App;

	HINSTANCE m_AppInst = nullptr; // Application instance handle
	HWND      m_AppWnd  = nullptr; // Application window handle

	bool      m_AppPaused  = false; // Is the application paused?
	bool      m_Minimized  = false; // Is the application minimized?
	bool      m_Maximized  = false; // Is the application maximized?
	bool      m_Resizing   = false; // Are the resize bars being dragged?
	bool m_FullscreenState = false; // Fullscreen enabled

	// Set true to use MSAA. The default level is 4.
	bool m_msaaState = false;
	UINT m_msaaLevel = 4;
	UINT m_msaaQuality = 0;

	// Used to keep track of the "delta-time" and game time.
	GameTimer m_Timer;

	// DX State
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Device>  m_d3dDevice;

	// Fence
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence = 0;

	// Command objects
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	// Swapchain
	static const int SwapChainBufferCount = 2;
	int m_CurrentBackBuffer;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	// Descriptor Heaps
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;

	// Viewport and Scissor rect.
	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	// Derived class should set these in derived contructor to customize starting values.
	std::wstring m_MainWndCaption = L"D3D App";
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	int m_ClientWidth = 1280;
	int m_ClientHeight = 720;
};