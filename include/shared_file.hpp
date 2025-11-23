#pragma once


#define SHARE_NAME "MyDX11SharingTest"

#define NOMINMAX
#include <Windows.h>



template <typename T> class sharing_server
{
public:
	sharing_server() 
	{
		mapped_file = CreateFileMappingA(
			INVALID_HANDLE_VALUE,
			nullptr,
			PAGE_READWRITE,
			0,
			sizeof(T),
			SHARE_NAME //TODO compose a string that may use something passed to this ctor, if you want multiple of those.
		);

		if (mapped_file) 
		{
			view_of_file = MapViewOfFile(
				mapped_file,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				sizeof(T)
			);
		}
	}

	T* get()
	{
		return (T*)view_of_file;
	}

	~sharing_server() {
		if (view_of_file) {
			UnmapViewOfFile(view_of_file);
			view_of_file = nullptr;
		}

		if (mapped_file) {
			CloseHandle(mapped_file);
			mapped_file = INVALID_HANDLE_VALUE;
		}
	}

	/* there's the move and copy stuff that we ain't gonna use so I am not bothering*/

private:
	HANDLE mapped_file = INVALID_HANDLE_VALUE;
	LPVOID view_of_file = nullptr;
};

template <typename T> class sharing_client 
{
public:
	sharing_client() 
	{
		mapped_file = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARE_NAME);

		if (mapped_file) 
		{
			view_of_file = MapViewOfFile(
				mapped_file,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				sizeof(T)
			);
		}
	}

	T* get()
	{
		return (T*)view_of_file;
	}

	~sharing_client()
	{
		if (view_of_file)
		{
			UnmapViewOfFile(view_of_file);
			view_of_file = nullptr;
		}

		if (mapped_file)
		{
			CloseHandle(mapped_file);
			mapped_file = INVALID_HANDLE_VALUE;
		}
	}

private:

	HANDLE mapped_file = INVALID_HANDLE_VALUE;
	LPVOID view_of_file = nullptr;
};
