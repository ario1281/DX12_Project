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
using com_ptr = Microsoft::WRL::ComPtr<T>;