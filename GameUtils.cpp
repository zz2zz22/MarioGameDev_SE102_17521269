﻿#include "GameUtils.h"

namespace GameUtils {
	ID3D10Device* directDevice = nullptr;
	ID3DX10Sprite* spriteHandler = nullptr;
	ID3DX10Font* font = nullptr;

	void GetFontRect(LPCSTR text, RECT* rect) {
		//Dùng DT_CALCRECT để tính toán kích thước text trước khi vẽ lên canvas
		//https://stackoverflow.com/questions/7719025/calculating-size-of-text-before-drawing-to-a-canvas
		font->DrawTextA(
			spriteHandler,
			text,
			-1,
			rect,
			DT_CALCRECT,
			D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)
		);
	}

	void WriteTextToScreen(LPCSTR text, RECT* rect, UINT format, D3DXCOLOR fontColor) {
		GetFontRect(text, rect);
		font->DrawTextA(spriteHandler, text, -1, rect, format, fontColor);
	}

	//Tách dòng kí tự trong file txt data
	std::vector<std::string> SplitStr(std::string line, std::string delimeter) {
		std::vector<std::string> tokens;
		size_t last = 0, next = 0;

		while ((next = line.find(delimeter, last)) != std::string::npos) {
			tokens.push_back(line.substr(last, next - last));
			last = next + 1;
		}

		tokens.push_back(line.substr(last));

		return tokens;
	}

	std::vector<unsigned int> SplitDigit(unsigned int number) {
		std::vector<unsigned int> digits;

		while (number > 0) {
			unsigned int digit = number % 10;
			number /= 10;
			digits.insert(digits.begin(), digit);
		}

		return digits;
	}
	LPCWSTR ToLPCWSTR(std::string str) {
		const char* s = str.c_str();

		size_t newSize = strlen(s) + 1;
		wchar_t* wCString = new wchar_t[newSize];

		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wCString, newSize, s, _TRUNCATE);
		std::wstring* wStr = new std::wstring(wCString);

		return wStr->c_str();
	}

	std::string ToStr(std::wstring& wStr) {
		typedef std::codecvt_utf8<wchar_t> convertTypeX;
		std::wstring_convert<convertTypeX, wchar_t> converterX;

		return converterX.to_bytes(wStr);
	}
}
