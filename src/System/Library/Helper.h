#pragma once

#define SafeClear(p) (p).clear();

/// <summary>
/// string(�}���`�o�C�g������)����wstring(���C�h������)�𓾂�
/// </summary>
/// <param name="str">�}���`�o�C�g������</param>
/// <returns>�ϊ����ꂽ���C�h������</returns>
inline std::wstring stow(_In_ const std::string& str) {
	// �����񐔂𓾂�
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0
	);

	std::wstring wstr;
	wstr.resize(num1);

	// �m�ۍς݂�wstr�ɕϊ���������R�s�[
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1
	);

	return wstr;
}

inline HRESULT stow_s(_In_ const std::string& str, _Out_ std::wstring& wstr) {
	// �����񐔂𓾂�
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0
	);

	wstr.resize(num1);

	// �m�ۍς݂�wstr�ɕϊ���������R�s�[
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
