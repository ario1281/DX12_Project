#pragma once

void DebugLog(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

template<class T>
void SafeRelease(T*& p) {
	if (p) {
		p->Release();
		p = nullptr;
	}
}
template<class T>
void SafeDelete(T*& p) {
	if (p) {
		delete p;
		p = nullptr;
	}
}

#define SafeClear(p) (p).clear()

template<class T>
using com_ptr = Microsoft::WRL::ComPtr<T>;
