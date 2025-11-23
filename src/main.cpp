#include "SDL3/SDL.h"
#include "SDL3/SDL_system.h"
#define NOMINMAX
#include "Windows.h"

#include <d3d11_4.h>
#pragma comment(lib, "D3D11.lib")

#include <cstdio>
#include <vector>

#include "shared_file.hpp"

struct r8g8b8a8_pixel 
{
	unsigned char r, g, b, a;
};

std::vector<r8g8b8a8_pixel> CreateTestPattern(int w = 1920, int h = 1080) 
{
	const r8g8b8a8_pixel black = {
		0, 0, 0, 255
	};

	const r8g8b8a8_pixel green = {
		0, 255, 0, 255
	};

	const auto size = w * h;
	std::vector<r8g8b8a8_pixel> pixels(size);

	for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x)
	{
		int p = y * w + x;

		if (((y / 10) % 2) ^ ((x / 10) % 2))
			pixels[p] = green;
		else
			pixels[p] = black;
	}

	return pixels;
}

struct shared
{
	HANDLE share_handle = INVALID_HANDLE_VALUE;
	BOOL set = false;
};

int main(int argc, char** argv) 
{
	sharing_server<shared> server;
	if (auto* file = server.get(); file != nullptr) {
		printf("has allocated the shared file.\n");
		new (file) shared();
	}

	//Using SDL just to create a window and a message pump loop because it's easiest
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("dx11 share", 800, 600, 0);
	if (!window)
		return -1;
	
	// I am so glad this is now how this works. SDL2 SysWM API was annoying
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	D3D_FEATURE_LEVEL dxLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL usedLevel = {};

	DXGI_SWAP_CHAIN_DESC swapchain_desc{};
	swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 2;
	swapchain_desc.OutputWindow = hwnd;
	swapchain_desc.Windowed = true;

	IDXGISwapChain* pSwapchain = nullptr;
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;

	if (S_OK != (D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		dxLevels,
		2,
		D3D11_SDK_VERSION,
		&swapchain_desc,
		&pSwapchain,
		&pDevice,
		&usedLevel,
		&pContext)))
	{
		fprintf(stderr, "Did not create DirectX 11 context and swapchain\n");
		return -2;
	}

	D3D11_TEXTURE2D_DESC desc{};

	desc.Width = 1920;
	desc.Height = 1080;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D10_RESOURCE_MISC_SHARED; //TODO toggle NT_HANDLE to test.

	
	ID3D11Texture2D* share_me = nullptr;
	auto test_grid = CreateTestPattern(desc.Width, desc.Height);
	const auto status = pDevice->CreateTexture2D(&desc, nullptr, &share_me);

	if (S_OK != status)
	{
		printf("Failed to allocate texture.\n");
	}

	HANDLE share_handle = 0;

	if (share_me)
	{
		printf("texture allocated, attepting to get handle\n");
		IDXGIResource* resource = nullptr;
		share_me->QueryInterface(&resource);
		if (resource) 
		{
			printf("Got resource interface for texure");
			resource->GetSharedHandle(&share_handle);
			resource->Release();
		}
	}

	printf("We got value %lld for handle\n", (long long)share_handle);
	if (share_handle == INVALID_HANDLE_VALUE)
		printf("which is invalid :-(\n");
	else
	{
		if (auto* file = server.get(); file)
		{
			file->share_handle = share_handle;
			file->set = TRUE;
			printf("Has written the HANDLE into the shared file called %s\n", SHARE_NAME);
		}
	}


	bool running = true;
	while (running)
	{
		SDL_Event e{};
		SDL_PollEvent(&e);

		switch (e.type)
		{
		default:
			break;

		case SDL_EVENT_QUIT:
			running = false;
			break;
		}

	}

	SDL_Quit();
	(void)getchar();

	if (auto* file = server.get(); file)
	{
		::operator delete(file);
	}

	return 0;
}