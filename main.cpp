#include "Game.h"

int __stdcall WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	RECT rect = { 0, 0, GameUtils::SCREEN_WIDTH, GameUtils::SCREEN_HEIGHT };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	Game::windowAdjustX = (rect.right - rect.left) - GameUtils::SCREEN_WIDTH;
	Game::windowAdjustY = (rect.bottom - rect.top) - GameUtils::SCREEN_HEIGHT;

	Game* gameInstance = Game::GetInstance();

	HWND hWND = gameInstance->CreateGameWindow(hInstance, nCmdShow, GameUtils::SCREEN_WIDTH, GameUtils::SCREEN_HEIGHT);

	if (!gameInstance->InitGame(hWND)) {
		MessageBoxA(hWND, "Failed to create the game, see output console", "Error", MB_ICONERROR);
		return -1;
	}

	if (!gameInstance->InitDevice()) {
		MessageBoxA(hWND, "Failed to create the device", "Error", MB_ICONERROR);
		return -1;
	}

	RECT windowRect;
	GetWindowRect(hWND, &windowRect);
	unsigned int xPos = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 3;
	unsigned int yPos = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 4;

	gameInstance->LoadSettings("assets/data/settings.txt");
	SetWindowPos(hWND, nullptr, xPos, yPos, GameUtils::SCREEN_WIDTH * 3, GameUtils::SCREEN_HEIGHT * 3, SWP_NOOWNERZORDER | SWP_NOZORDER);
	gameInstance->GameRun();

	return 0;
}