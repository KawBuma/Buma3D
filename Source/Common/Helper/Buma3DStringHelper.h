#pragma once

/*
標準ライブラリ stringヘッダの実装を改造
*/

namespace buma3d
{
namespace hlp
{

inline constexpr bool IS_ENABLE_DEBUG_STRING = true;

inline void OutDebugStr(const char* _str)
{
    if constexpr (IS_ENABLE_DEBUG_STRING)
    {
    #if B3D_PLATFORM_IS_USE_WINDOWS
        //OutputDebugStringA("Buma3D: ");
        OutputDebugStringA(_str);
    #else
    #endif
    }
}

inline void OutDebugStr(const wchar_t* _str)
{
    if constexpr (IS_ENABLE_DEBUG_STRING)
    {
    #if B3D_PLATFORM_IS_USE_WINDOWS
        //OutputDebugStringW(L"Buma3D: ");
        OutputDebugStringW(_str);
    #else
    #endif
    }
}

inline void OutDebugStr(const util::String& _str)
{
    if constexpr (IS_ENABLE_DEBUG_STRING)
    {
    #if B3D_PLATFORM_IS_USE_WINDOWS
        //OutputDebugStringA("Buma3D: ");
        OutputDebugStringA(_str.c_str());
    #else
    #endif
    }
}

inline void OutDebugStr(const util::WString& _str)
{
    if constexpr (IS_ENABLE_DEBUG_STRING)
    {
    #if B3D_PLATFORM_IS_USE_WINDOWS
        //OutputDebugStringW(L"Buma3D: ");
        OutputDebugStringW(_str.c_str());
    #else
    #endif
    }
}

inline constexpr const char* FileName(const char* _path)
{
    const char* file = _path;
    // スラッシュを探して上書きしていく
    while (*_path != '\0')
    {
        if (*_path == '/' || *_path == '\\')
            file = _path + 1;
        _path++;
    }
    return file;
    //const char* file = _path;
    //while (*_path)
    //{
    //  if (*_path++ == '/')
    //  {
    //      file = _path;
    //  }
    //}
    //return file;
}

template<typename ...Args>
inline util::String StringConvolution(const Args&... _ar)
{
    util::StringStream ss;
    (ss << ... << _ar);
    return ss.str();
}

template<typename ...Args>
inline util::WString WStringConvolution(const Args&... _ar)
{
    util::WStringStream ss;
    (ss << ... << _ar);
    return ss.str();
}

template<typename ...Args>
inline util::StringStream SStreamConvolution(const Args&... _ar)
{
    util::StringStream ss;
    (ss << ... << _ar);
    return ss;
}

template<typename ...Args>
inline util::WStringStream WSStreamConvolution(const Args&... _ar)
{
    util::WStringStream ss;
    (ss << ... << _ar);
    return ss;
}

inline util::String GetUUIDString(const uint8_t _uuid[16])
{
#define B3DFW std::setfill('0') << std::setw(2) 
    util::StringStream ss;
    ss << std::hex 
        << B3DFW << (uint32_t)_uuid[0]  << B3DFW << (uint32_t)_uuid[1] << B3DFW << (uint32_t)_uuid[2] << B3DFW << (uint32_t)_uuid[3] << "-"
        << B3DFW << (uint32_t)_uuid[4]  << B3DFW << (uint32_t)_uuid[5] << '-'
        << B3DFW << (uint32_t)_uuid[6]  << B3DFW << (uint32_t)_uuid[7] << '-'
        << B3DFW << (uint32_t)_uuid[8]  << B3DFW << (uint32_t)_uuid[9] << '-'
        << B3DFW << (uint32_t)_uuid[10] << B3DFW << (uint32_t)_uuid[11] << B3DFW << (uint32_t)_uuid[12] << B3DFW << (uint32_t)_uuid[13] << B3DFW << (uint32_t)_uuid[14] << B3DFW << (uint32_t)_uuid[15]
        << std::dec;
    return ss.str();
#undef B3DFW
}

inline util::String GetLUIDString(const uint8_t _luid[8])
{
#define B3DFW std::setfill('0') << std::setw(2) 
    util::StringStream ss;
    ss << std::hex
        << "Low: "    << B3DFW << (uint32_t)_luid[0] << B3DFW << (uint32_t)_luid[1] << B3DFW << (uint32_t)_luid[2] << B3DFW << (uint32_t)_luid[3]
        << ", High: " << B3DFW << (uint32_t)_luid[4] << B3DFW << (uint32_t)_luid[5] << B3DFW << (uint32_t)_luid[6] << B3DFW << (uint32_t)_luid[7]
        << std::dec;
    return ss.str();
#undef B3DFW
}

inline const char* GetName(const INameableObject* _obj)
{
    auto name = _obj->GetName();
    return (name ? name : "(Unnamed)");
}

template<typename T>
inline util::String GetHexString(const T& _val)
{
    util::StringStream ss;
    ss << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(_val)) << _val << std::dec;
    return ss.str();
}

template<typename T>
inline std::ostream& PrintHex(util::StringStream& _ss, const T& _val)
{
    return _ss << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(_val)) << _val << std::dec;
}

template<typename T>
inline std::wostream& PrintHex(util::WStringStream& _ss, const T& _val)
{
    return _ss << L"0x" << std::hex << std::setfill(L'0') << std::setw(sizeof(_val)) << _val << std::dec;
}

// sto* NARROW CONVERSIONS
inline int stoi(const util::String& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert string to int
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const long _Ans = _CSTD strtol(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoi argument");
    }

    if (_Errno_ref == ERANGE || _Ans < INT_MIN || INT_MAX < _Ans)
    {
        std::_Xout_of_range("stoi argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return static_cast<int>(_Ans);
}

inline long stol(const util::String& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert string to long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const long _Ans = _CSTD strtol(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stol argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stol argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline unsigned long stoul(const util::String& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert string to unsigned long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const unsigned long _Ans = _CSTD strtoul(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoul argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoul argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline long long stoll(const util::String& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert string to long long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const long long _Ans = _CSTD strtoll(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoll argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoll argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline unsigned long long stoull(const util::String& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert string to unsigned long long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const unsigned long long _Ans = _CSTD strtoull(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoull argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoull argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline float stof(const util::String& _Str, size_t* _Idx = nullptr)
{ // convert string to float
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const float _Ans = _CSTD strtof(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stof argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stof argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline double stod(const util::String& _Str, size_t* _Idx = nullptr)
{ // convert string to double
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const double _Ans = _CSTD strtod(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stod argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stod argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline long double stold(const util::String& _Str, size_t* _Idx = nullptr)
{ // convert string to long double
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const char* _Ptr = _Str.c_str();
    char* _Eptr;
    _Errno_ref = 0;
    const long double _Ans = _CSTD strtold(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stold argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stold argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

// sto* WIDE CONVERSIONS
inline int stoi(const util::WString& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert wstring to int
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const long _Ans = _CSTD wcstol(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoi argument");
    }

    if (_Errno_ref == ERANGE || _Ans < INT_MIN || INT_MAX < _Ans)
    {
        std::_Xout_of_range("stoi argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return static_cast<int>(_Ans);
}

inline long stol(const util::WString& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert wstring to long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const long _Ans = _CSTD wcstol(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stol argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stol argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline unsigned long stoul(const util::WString& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert wstring to unsigned long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const unsigned long _Ans = _CSTD wcstoul(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoul argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoul argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline long long stoll(const util::WString& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert wstring to long long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const long long _Ans = _CSTD wcstoll(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoll argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoll argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline unsigned long long stoull(const util::WString& _Str, size_t* _Idx = nullptr, int _Base = 10)
{
    // convert wstring to unsigned long long
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const unsigned long long _Ans = _CSTD wcstoull(_Ptr, &_Eptr, _Base);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stoull argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stoull argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline float stof(const util::WString& _Str, size_t* _Idx = nullptr)
{ // convert wstring to float
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const float _Ans = _CSTD wcstof(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stof argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stof argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline double stod(const util::WString& _Str, size_t* _Idx = nullptr)
{ // convert wstring to double
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const double _Ans = _CSTD wcstod(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stod argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stod argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

inline long double stold(const util::WString& _Str, size_t* _Idx = nullptr)
{ // convert wstring to long double
    int& _Errno_ref = errno; // Nonzero cost, pay it once
    const wchar_t* _Ptr = _Str.c_str();
    wchar_t* _Eptr;
    _Errno_ref = 0;
    const long double _Ans = _CSTD wcstold(_Ptr, &_Eptr);

    if (_Ptr == _Eptr)
    {
        std::_Xinvalid_argument("invalid stold argument");
    }

    if (_Errno_ref == ERANGE)
    {
        std::_Xout_of_range("stold argument out of range");
    }

    if (_Idx)
    {
        *_Idx = static_cast<size_t>(_Eptr - _Ptr);
    }

    return _Ans;
}

// HELPERS FOR to_string AND to_wstring
template <class _Elem, class _UTy>
_Elem* _UIntegral_to_buff(_Elem* _RNext, _UTy _UVal)
{ // format _UVal into buffer *ending at* _RNext
    static_assert(std::is_unsigned_v<_UTy>, "_UTy must be unsigned");

#ifdef _WIN64
    auto _UVal_trunc = _UVal;
#else // ^^^ _WIN64 ^^^ // vvv !_WIN64 vvv

    constexpr bool _Big_uty = sizeof(_UTy) > 4;
    if
        _CONSTEXPR_IF(_Big_uty)
    { // For 64-bit numbers, work in chunks to avoid 64-bit divisions.
        while (_UVal > 0xFFFFFFFFU)
        {
            auto _UVal_chunk = static_cast<unsigned long>(_UVal % 1000000000);
            _UVal /= 1000000000;

            for (int _Idx = 0; _Idx != 9; ++_Idx)
            {
                *--_RNext = static_cast<_Elem>('0' + _UVal_chunk % 10);
                _UVal_chunk /= 10;
            }
        }
    }

    auto _UVal_trunc = static_cast<unsigned long>(_UVal);
#endif // _WIN64

    do
    {
        *--_RNext = static_cast<_Elem>('0' + _UVal_trunc % 10);
        _UVal_trunc /= 10;
    } while (_UVal_trunc != 0);
    return _RNext;
}

template <class _Elem, class _Ty>
std::basic_string<_Elem, std::char_traits<_Elem>, util::details::b3d_allocator<_Elem>> _Integral_to_string(const _Ty _Val)
{ // convert _Val to string
    static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
    using _UTy = std::make_unsigned_t<_Ty>;
    _Elem _Buff[21]; // can hold -2^63 and 2^64 - 1, plus NUL
    _Elem* const _Buff_end = _STD end(_Buff);
    _Elem* _RNext = _Buff_end;
    const auto _UVal = static_cast<_UTy>(_Val);
    if (_Val < 0)
    {
        _RNext = std::_UIntegral_to_buff(_RNext, 0 - _UVal);
        *--_RNext = '-';
    }
    else
    {
        _RNext = std::_UIntegral_to_buff(_RNext, _UVal);
    }

    return std::basic_string<_Elem, std::char_traits<_Elem>, util::details::b3d_allocator<_Elem>>(_RNext, _Buff_end);
}

// to_string NARROW CONVERSIONS
_NODISCARD inline util::String to_string(int _Val)
{ // convert int to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(unsigned int _Val)
{ // convert unsigned int to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(long _Val)
{ // convert long to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(unsigned long _Val)
{ // convert unsigned long to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(long long _Val)
{ // convert long long to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(unsigned long long _Val)
{ // convert unsigned long long to string
    return _Integral_to_string<char>(_Val);
}

_NODISCARD inline util::String to_string(double _Val)
{ // convert double to string
    const auto _Len = static_cast<size_t>(_CSTD _scprintf("%f", _Val));
    util::String _Str(_Len, '\0');
    _CSTD sprintf_s(&_Str[0], _Len + 1, "%f", _Val);
    return _Str;
}

_NODISCARD inline util::String to_string(float _Val)
{ // convert float to string
    return to_string(static_cast<double>(_Val));
}

_NODISCARD inline util::String to_string(long double _Val)
{ // convert long double to string
    return to_string(static_cast<double>(_Val));
}

// to_wstring WIDE CONVERSIONS
_NODISCARD inline util::WString to_wstring(int _Val)
{ // convert int to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(unsigned int _Val)
{ // convert unsigned int to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(long _Val)
{ // convert long to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(unsigned long _Val)
{ // convert unsigned long to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(long long _Val)
{ // convert long long to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(unsigned long long _Val)
{ // convert unsigned long long to wstring
    return _Integral_to_string<wchar_t>(_Val);
}

_NODISCARD inline util::WString to_wstring(double _Val)
{ // convert double to wstring
    const auto _Len = static_cast<size_t>(_CSTD _scwprintf(L"%f", _Val));
    util::WString _Str(_Len, L'\0');
    _CSTD swprintf_s(&_Str[0], _Len + 1, L"%f", _Val);
    return _Str;
}

_NODISCARD inline util::WString to_wstring(float _Val)
{ // convert float to wstring
    return to_wstring(static_cast<double>(_Val));
}

_NODISCARD inline util::WString to_wstring(long double _Val)
{ // convert long double to wstring
    return to_wstring(static_cast<double>(_Val));
}

/* CP <=> Wide */

inline util::String ConvertWideToCp(uint32_t _code_page /*= CP_UTF8*/, int _len_with_null_term, const wchar_t* _wstr)
{
    auto l = WideCharToMultiByte(_code_page, 0, _wstr, _len_with_null_term, nullptr, 0, nullptr, FALSE);
    if (l == 0) return util::String();

    util::String str(l, '\0');// 結果のUnicode文字列にはnull終端文字があり、関数によって返される長さにはこの文字が含まれます。
    if (WideCharToMultiByte(_code_page, 0, _wstr, _len_with_null_term, str.data(), l, nullptr, FALSE) == 0)
        return util::String();

    return str;
}
inline util::WString ConvertCpToWide(uint32_t _code_page /*= CP_UTF8*/, int _len_with_null_term, const char* _str)
{
    auto l = MultiByteToWideChar(_code_page, 0, _str, _len_with_null_term, nullptr, 0);
    if (l == 0)
        return util::WString();

    util::WString str(l, L'\0');// 結果のUnicode文字列にはnull終端文字があり、関数によって返される長さにはこの文字が含まれます。
    if (MultiByteToWideChar(_code_page, 0, _str, _len_with_null_term, str.data(), l) == 0)
        return util::WString();

    return str;
}

inline util::String  ConvertWideToCp  (uint32_t _code_page, const util::WString& _wstr)    { return ConvertWideToCp(_code_page, int(_wstr.size() + 1ull), _wstr.c_str()); }
inline util::WString ConvertCpToWide  (uint32_t _code_page, const util::String& _str)      { return ConvertCpToWide(_code_page, int(_str.size() + 1ull), _str.c_str()); }

inline util::WString ConvertUtf8ToWide(const char*          _str)                          { return ConvertCpToWide(CP_UTF8, _str); }
inline util::WString ConvertUtf8ToWide(const util::String&  _str)                          { return ConvertCpToWide(CP_UTF8, int(_str.size() + 1ull), _str.c_str()); }
inline util::String  ConvertWideToUtf8(const wchar_t*       _wstr)                         { return ConvertWideToCp(CP_UTF8, _wstr); }
inline util::String  ConvertWideToUtf8(const util::WString& _wstr)                         { return ConvertWideToCp(CP_UTF8, int(_wstr.size() + 1ull), _wstr.c_str()); }

inline util::WString ConvertAnsiToWide(const char*          _str)                          { return ConvertCpToWide(CP_ACP, _str); }
inline util::WString ConvertAnsiToWide(const util::String&  _str)                          { return ConvertCpToWide(CP_ACP, int(_str.size() + 1ull), _str.c_str()); }
inline util::String  ConvertWideToAnsi(const wchar_t*       _wstr)                         { return ConvertWideToCp(CP_ACP, _wstr); }
inline util::String  ConvertWideToAnsi(const util::WString& _wstr)                         { return ConvertWideToCp(CP_ACP, int(_wstr.size() + 1ull), _wstr.c_str()); }

#if _HAS_CXX20
// XXX
//inline std::u8string ConvertStrToU8Str(const util::String&    _str)                       { return std::u8string(_str.begin(), _str.end()); }
//inline std::u8string ConvertU8StrToStr(const std::u8string&   _str)                       { return std::u8string(_str.begin(), _str.end()); }
//inline std::u8string ConvertWideToU8Str(const wchar_t*       _wstr)                      { return ConvertStrToU8Str(ConvertWideToCp(CP_UTF8, _wstr)); }
//inline std::u8string ConvertWideToU8Str(const util::WString& _wstr)                      { return ConvertStrToU8Str(ConvertWideToCp(CP_UTF8, _wstr.size() + 1ull, _wstr.c_str())); }
#endif

/* CP <=> CP */

inline util::String ConvertCpToCp(uint32_t _src_code_page /*= CP_ACP*/, uint32_t _dst_code_page /*= CP_UTF8*/, const char* _str)
{
    auto s = ConvertCpToWide(_src_code_page, int(std::strlen(_str) + 1ull), _str);
    return   ConvertWideToCp(_dst_code_page, int(s.size()          + 1ull), s.c_str());
}
inline util::String ConvertCpToCp(uint32_t _src_code_page /*= CP_ACP*/, uint32_t _dst_code_page /*= CP_UTF8*/, const util::String& _str)
{
    auto s = ConvertCpToWide(_src_code_page, int(_str.size() + 1ull), _str.c_str());
    return   ConvertWideToCp(_dst_code_page, int(s   .size() + 1ull), s   .c_str());
}

inline util::String ConvertUtf8ToAnsi(const char* _str) { return ConvertCpToCp(CP_UTF8, CP_ACP, _str); }
inline util::String ConvertAnsiToUtf8(const char* _str) { return ConvertCpToCp(CP_ACP, CP_UTF8, _str); }

inline util::String  to_string (const wchar_t*       _wstr) { return ConvertWideToUtf8(_wstr); }
inline util::WString to_wstring(const char*          _str)  { return ConvertUtf8ToWide(_str); }
inline util::String  to_string (const util::WString& _wstr) { return ConvertWideToUtf8(_wstr); }
inline util::WString to_wstring(const util::String&  _str)  { return ConvertUtf8ToWide(_str); }


}// namespace hlp
}// namespace buma3d
