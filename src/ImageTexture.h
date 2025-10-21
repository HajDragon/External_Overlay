#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wincodec.h>
#include <vector>

class DX11ImageTexture {
public:
    ID3D11ShaderResourceView* srv = nullptr;
    UINT width = 0;
    UINT height = 0;

    DX11ImageTexture() = default;
    ~DX11ImageTexture();

    // Disable copy constructor and assignment operator
    DX11ImageTexture(const DX11ImageTexture&) = delete;
    DX11ImageTexture& operator=(const DX11ImageTexture&) = delete;

    // Enable move constructor and assignment operator
    DX11ImageTexture(DX11ImageTexture&& other) noexcept;
    DX11ImageTexture& operator=(DX11ImageTexture&& other) noexcept;

    // Load image from file path (supports PNG, JPG, BMP, GIF, TIFF, etc.)
    bool LoadFromFile(ID3D11Device* device, const wchar_t* filename);

    // Load image from memory buffer
    bool LoadFromMemory(ID3D11Device* device, const void* data, size_t dataSize);

    // Release the texture resources
    void Release();

    // Check if texture is valid
    bool IsValid() const { return srv != nullptr; }

    // Get ImGui texture ID for rendering
    void* GetImGuiTextureID() const { return static_cast<void*>(srv); }

private:
    bool CreateTextureFromWICBitmap(ID3D11Device* device, IWICBitmapSource* bitmapSource);
    void InitializeCOM();
    void UninitializeCOM();
    
    bool comInitialized = false;
};