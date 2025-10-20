#include "ImageTexture.h"
#include <objbase.h>
#include <combaseapi.h>

DX11ImageTexture::~DX11ImageTexture() {
    Release();
    if (comInitialized) {
        UninitializeCOM();
    }
}

DX11ImageTexture::DX11ImageTexture(DX11ImageTexture&& other) noexcept
    : srv(other.srv), width(other.width), height(other.height), comInitialized(other.comInitialized) {
    other.srv = nullptr;
    other.width = 0;
    other.height = 0;
    other.comInitialized = false;
}

DX11ImageTexture& DX11ImageTexture::operator=(DX11ImageTexture&& other) noexcept {
    if (this != &other) {
        Release();
        if (comInitialized) {
            UninitializeCOM();
        }

        srv = other.srv;
        width = other.width;
        height = other.height;
        comInitialized = other.comInitialized;

        other.srv = nullptr;
        other.width = 0;
        other.height = 0;
        other.comInitialized = false;
    }
    return *this;
}

void DX11ImageTexture::InitializeCOM() {
    if (!comInitialized) {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        comInitialized = (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE);
    }
}

void DX11ImageTexture::UninitializeCOM() {
    if (comInitialized) {
        CoUninitialize();
        comInitialized = false;
    }
}

bool DX11ImageTexture::LoadFromFile(ID3D11Device* device, const wchar_t* filename) {
    Release();
    InitializeCOM();
    
    if (!device || !filename) {
        return false;
    }

    IWICImagingFactory* wicFactory = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool success = false;

    // Create WIC factory
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) {
        return false;
    }

    // Create decoder from filename
    hr = wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ,
                                               WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) {
        wicFactory->Release();
        return false;
    }

    // Get first frame
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        decoder->Release();
        wicFactory->Release();
        return false;
    }

    // Create format converter
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return false;
    }

    // Initialize converter to RGBA format
    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                               WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(hr)) {
        success = CreateTextureFromWICBitmap(device, converter);
    }

    // Cleanup
    converter->Release();
    frame->Release();
    decoder->Release();
    wicFactory->Release();

    return success;
}

bool DX11ImageTexture::LoadFromMemory(ID3D11Device* device, const void* data, size_t dataSize) {
    Release();
    InitializeCOM();
    
    if (!device || !data || dataSize == 0) {
        return false;
    }

    IWICImagingFactory* wicFactory = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool success = false;

    // Create WIC factory
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) {
        return false;
    }

    // Create stream from memory
    hr = wicFactory->CreateStream(&stream);
    if (FAILED(hr)) {
        wicFactory->Release();
        return false;
    }

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(static_cast<const BYTE*>(data)), static_cast<DWORD>(dataSize));
    if (FAILED(hr)) {
        stream->Release();
        wicFactory->Release();
        return false;
    }

    // Create decoder from stream
    hr = wicFactory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) {
        stream->Release();
        wicFactory->Release();
        return false;
    }

    // Get first frame
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        decoder->Release();
        stream->Release();
        wicFactory->Release();
        return false;
    }

    // Create format converter
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        frame->Release();
        decoder->Release();
        stream->Release();
        wicFactory->Release();
        return false;
    }

    // Initialize converter to RGBA format
    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                               WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(hr)) {
        success = CreateTextureFromWICBitmap(device, converter);
    }

    // Cleanup
    converter->Release();
    frame->Release();
    decoder->Release();
    stream->Release();
    wicFactory->Release();

    return success;
}

bool DX11ImageTexture::CreateTextureFromWICBitmap(ID3D11Device* device, IWICBitmapSource* bitmapSource) {
    // Get image dimensions
    HRESULT hr = bitmapSource->GetSize(&width, &height);
    if (FAILED(hr)) {
        return false;
    }

    // Calculate row pitch and total size
    UINT rowPitch = width * 4; // 4 bytes per pixel (RGBA)
    UINT imageSize = rowPitch * height;

    // Allocate memory for pixel data
    std::vector<BYTE> pixels(imageSize);

    // Copy pixels from WIC bitmap
    hr = bitmapSource->CopyPixels(nullptr, rowPitch, imageSize, pixels.data());
    if (FAILED(hr)) {
        return false;
    }

    // Create D3D11 texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    // Set up initial data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = rowPitch;

    // Create texture
    ID3D11Texture2D* texture = nullptr;
    hr = device->CreateTexture2D(&desc, &initData, &texture);
    if (FAILED(hr)) {
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release(); // Release texture as we only need the SRV

    return SUCCEEDED(hr);
}

void DX11ImageTexture::Release() {
    if (srv) {
        srv->Release();
        srv = nullptr;
    }
    width = 0;
    height = 0;
}