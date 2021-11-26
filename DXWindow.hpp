#pragma once

#include <Windows.h>
#include <d3d11_1.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <atlcomcli.h>

#include <memory>
#include <string>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "DXGI.lib")

namespace Smurf {
    CComPtr<IDXGISwapChain>         swapchain;
    CComPtr<ID3D11Device>           device;
    CComPtr<ID3D11DeviceContext>    deviceContext;
    CComPtr<ID3D11RenderTargetView> backbuffer;
    std::unique_ptr<concurrency::graphics::texture_view<concurrency::graphics::float_4, 2>> bufferTexture;

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_DESTROY: 
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    void InitWindow(HWND& hWnd, HINSTANCE& hInst) {
        RECT wr = {0, 0, 640, 480};
        AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(WNDCLASSEX));

        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc.lpszClassName = "WindowClass1";
        
        if (!RegisterClassEx(&wc)) {
            throw std::runtime_error("RegisterClassEx() failed.");
        }

        hWnd = CreateWindowEx(0, "WindowClass1", "Smurfs Raytracer",
                              WS_OVERLAPPEDWINDOW, 300, 300, wr.right - wr.left, wr.bottom - wr.top,
                              nullptr, nullptr, hInst, nullptr
                             );

        if (!hWnd) {
            throw std::runtime_error("CreateWindowEx() failed.");
        }
    }

    void InitD3D(HWND& hWnd) {
        auto createDeviceFlags = 0U;
        #ifdef _DEBUG
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        RECT window;
        GetClientRect(hWnd, &window);

        DXGI_SWAP_CHAIN_DESC scd;
        ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
        scd.BufferCount                        = 1;
        scd.BufferDesc.Width                   = window.right - window.left;
        scd.BufferDesc.Height                  = window.bottom - window.top;
        scd.BufferDesc.Format                  = DXGI_FORMAT_R16G16B16A16_FLOAT;
        scd.BufferDesc.RefreshRate.Numerator   = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.OutputWindow                       = hWnd;
        scd.SampleDesc.Count                   = 1;
        scd.SampleDesc.Quality                 = 0;
        scd.Windowed                           = TRUE;
	    scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_SHADER_INPUT;

        D3D_FEATURE_LEVEL FeatureLevel[] = { D3D_FEATURE_LEVEL_11_0 };

	    auto hr = D3D11CreateDeviceAndSwapChain(
                      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                      createDeviceFlags, FeatureLevel, 1,
                      D3D11_SDK_VERSION, &scd, &swapchain, &device, nullptr, &deviceContext
                  );

        CComPtr<ID3D11Texture2D> pBackBuffer;

        // Bind the texture to the backbuffer
        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));

        // Do arcane magic
        auto acceleratorView = concurrency::direct3d::create_accelerator_view(device);

        // Create an amp texture for interop
        auto ampTexture = Concurrency::graphics::direct3d::make_texture<concurrency::graphics::float_4, 2>(acceleratorView, pBackBuffer);
        bufferTexture.reset(new Concurrency::graphics::texture_view<concurrency::graphics::float_4, 2>(ampTexture));
    }
} // namespace Smurf