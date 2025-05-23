#pragma once

#define SafeClear(p) (p).clear();

/// <summary>
/// string(マルチバイト文字列)からwstring(ワイド文字列)を得る
/// </summary>
/// <param name="str">マルチバイト文字列</param>
/// <returns>変換されたワイド文字列</returns>
inline std::wstring stow(_In_ const std::string& str) {
	// 文字列数を得る
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0
	);

	std::wstring wstr;
	wstr.resize(num1);

	// 確保済みのwstrに変換文字列をコピー
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1
	);

	return wstr;
}

inline HRESULT stow_s(_In_ const std::string& str, _Out_ std::wstring& wstr) {
	// 文字列数を得る
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0
	);

	wstr.resize(num1);

	// 確保済みのwstrに変換文字列をコピー
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1
	);

	if (num1 != num2) { return S_FALSE; }

	return S_OK;
}


template<class T>
using com_ptr    = Microsoft::WRL::ComPtr<T>;

template<class T>
using shared_ptr = std::shared_ptr<T>;
template<class T>
using unique_ptr = std::unique_ptr<T>;
template<class T>
using weak_ptr   = std::weak_ptr<T>;
