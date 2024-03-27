// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "bot.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

void StartBot();
void CreateLogger();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		//MessageBoxA(nullptr, "DLL_PROCESS_ATTACH", "Redmatch2Bot", MB_OK);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartBot, nullptr, 0, nullptr);
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		BotCleanup();
        break;
    }
    return TRUE;
}

void StartBot()
{
	try
	{
		CreateLogger();
		spdlog::info(" ====================== Bot STARTED ======================");

		BotMain();
	}
	catch (const std::exception& e)
	{
		spdlog::error(e.what());
	}
	catch (...)
	{
		spdlog::error("An error occurred");
	}

	// Cleanup
	BotCleanup();

	// Turn the logger off
	spdlog::info(" ====================== Bot STOPPED ======================");
	spdlog::shutdown();

	// This is for the DLL to unload itself for debugging purposes (so that I can load it directly after)
	FreeLibraryAndExitThread(GetModuleHandle(L"Redmatch2Bot"), 0);
}

void CreateLogger()
{
	auto logger = spdlog::basic_logger_mt("redmatch2_bot_logger", "C:/Personal/Logs/Redmatch2Bot.log");
	spdlog::set_default_logger(logger);
	spdlog::flush_on(spdlog::level::trace);
	spdlog::set_level(spdlog::level::trace);
}