#pragma once
#include "../Buma3D.h"

namespace buma3d
{
namespace util
{
namespace details
{

template<typename T>
class PtrRefBase
{
public:
    using InterfaceType = typename T::InterfaceType;

    operator ISharedBase** () const
    {
        static_assert(std::is_base_of_v<ISharedBase, InterfaceType>, "Invalid cast: InterfaceType does not derive from ISharedBase");
        return reinterpret_cast<ISharedBase**>(ptr->ReleaseAndGetAddressOf());
    }

protected:
    T* ptr;

};

template<typename T>
class PtrRef : public PtrRefBase<T>
{
    using Super = PtrRefBase<T>;
    using InterfaceType = typename Super::InterfaceType;

public:
    PtrRef(T* _ptr)
    { this->ptr = _ptr; }

    // Conversion operators
    operator void** () const
    { return reinterpret_cast<void**>(this->ptr->ReleaseAndGetAddressOf()); }

    template<typename U, std::enable_if_t<std::is_base_of_v<U, InterfaceType>, int> = 0>
    operator U** () const
    { return reinterpret_cast<U**>(this->ptr->ReleaseAndGetAddressOf()); } 

    // This is our operator Ptr<U> (or the latest derived class from Ptr (e.g. WeakRef))
    operator T* ()
    {
        *this->ptr = nullptr;
        return this->ptr;
    }

    // We define operator InterfaceType**() here instead of on PtrRefBase<T>, since
    // if InterfaceType is IUnknown or IInspectable, having it on the base will collide.
    operator InterfaceType** ()
    { return this->ptr->ReleaseAndGetAddressOf(); }

    // This is used for IID_PPV_ARGS in order to do __uuidof(**(ppType)).
    // It does not need to clear  ptr at this point, it is done at IID_PPV_ARGS_Helper(PtrRef&) later in this file.
    InterfaceType* operator*()
    { return this->ptr->Get(); }

    // Explicit functions
    InterfaceType* const* GetAddressOf() const
    { return this->ptr->GetAddressOf(); }

    InterfaceType** ReleaseAndGetAddressOf()
    { return this->ptr->ReleaseAndGetAddressOf(); }

};


}// namespace details
}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace util
{

/**
 * @brief ISharedBaseを継承するインターフェースのポインターラッパー
 * @tparam T IsharedBaseを継承するインターフェース型
 * @note Microsoft::WRL::ComPtr<T>がベースです。
*/
template<typename T>
class Ptr
{
public:
    using InterfaceType = T;

protected:
    template<typename U>
    friend class Ptr;

public:
    Ptr() : ptr(nullptr)
    {}

    Ptr(std::nullptr_t) : ptr(nullptr)
    {}

    template<typename U>
    Ptr(U* _other) : ptr(_other)
    { InternalAddRef(); }

    Ptr(const Ptr& _other) : ptr(_other.ptr)
    { InternalAddRef(); }

    // copy constructor that allows to instantiate class when U* is convertible to T*
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Ptr(const Ptr<U>& _other) : ptr(_other.ptr)
    { InternalAddRef(); }

    Ptr(Ptr&& _other) noexcept : ptr(nullptr)
    {
        if (this != _other.GetThis())
            Swap(_other);
    }

    // Move constructor that allows instantiation of a class when U* is convertible to T*
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    Ptr(Ptr<U>&& _other) : ptr(_other.ptr)
    { _other.ptr = nullptr; }

    ~Ptr()
    { InternalRelease(); }

    Ptr* GetThis()
    { return this; }

    const Ptr* GetThis() const
    { return this; }

#pragma region assignment

    Ptr& operator=(std::nullptr_t)
    {
        InternalRelease();
        return *this;
    }

    Ptr& operator=(T* _other)
    {
        if (ptr != _other)
        { Ptr(_other).Swap(*this); }// _otherとthisがスワップされ、thisのみReleaseされる。
        return *this;
    }

    template <typename U>
    Ptr& operator=(U* _other)
    {
        Ptr(_other).Swap(*this);
        return *this;
    }

    Ptr& operator=(const Ptr& _other)
    {
        if (ptr != _other.ptr)
        { Ptr(_other).Swap(*this); }
        return *this;
    }

    template<typename U>
    Ptr& operator=(const Ptr<U>& _other)
    {
        Ptr(_other).Swap(*this);
        return *this;
    }

    Ptr& operator=(Ptr&& _other) noexcept
    {
        Ptr(std::move(_other)).Swap(*this);
        return *this;
    }

    template<typename U>
    Ptr& operator=(Ptr<U>&& _other)
    {
        Ptr(std::move(_other)).Swap(*this);
        return *this;
    }

#pragma endregion assignment

#pragma region modifiers

    void Swap(Ptr&& _r)
    {
        T* tmp = ptr;
        ptr = _r.ptr;
        _r.ptr = tmp;
    }

    void Swap(Ptr& _v)
    {
        T* tmp = ptr;
        ptr = _v.ptr;
        _v.ptr = tmp;
    }

#pragma endregion modifiers

    operator bool() const
    { return ptr; }

    T* Get() const
    { return ptr; }

    InterfaceType* operator->() const
    { return ptr; }

    details::PtrRef<Ptr<T>> operator&()
    { return details::PtrRef<Ptr<T>>(this); }

    const details::PtrRef<const Ptr<T>> operator&() const
    { return details::PtrRef<const Ptr<T>>(this); }

    T* const* GetAddressOf() const
    { return &ptr; }

    T** GetAddressOf()
    { return &ptr; }

    T** ReleaseAndGetAddressOf()
    {
        InternalRelease();
        return &ptr;
    }

    T* Detach()
    {
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void Attach(InterfaceType* _other)
    {
        if (ptr != nullptr)
        {
            auto ref = ptr->Release();
            (ref);
            //assert(ref != 0 || ptr != _other);
        }

        ptr = _other;
    }

    uint32_t Reset()
    { return InternalRelease(); }

    bool CopyTo(InterfaceType** _ptr) const
    {
        InternalAddRef();
        *_ptr = ptr;
        return true;
    }

    template<typename U>
    bool CopyTo(U** _ptr) const
    {
        U* result = ptr->DynamicCastFromThis<U>();
        if (result)
        {
            InternalAddRef();
            *_ptr = result;
            return true;
        }
        else
        {
            *_ptr = nullptr;
            return false;
        }
    }

    // query for U interface
    template<typename U, bool IsDynamicCast, std::enable_if_t<IsDynamicCast, int> = 0>
    Ptr<U> As() const
    { return Ptr<U>(ptr->DynamicCastFromThis<U>()); }

    // query for U interface
    template<typename U, bool IsDynamicCast = false, std::enable_if_t<!IsDynamicCast, int> = 0>
    Ptr<U> As() const
    { return Ptr<U>(ptr->StaticCastFromThis<U>()); }

protected:
    void InternalAddRef() const
    {
        if (ptr)
            ptr->AddRef();
    }

    uint32_t InternalRelease()
    {
        if (ptr == nullptr)
            return 0;

        uint32_t ref = ptr->Release();
        ptr = nullptr;
        return ref;
    }

protected:
    InterfaceType* ptr;

};

template<typename T, typename U>
bool operator==(const Ptr<T>& _a, const Ptr<U>& _b)
{
    static_assert(std::is_base_of_v<T, U> || std::is_base_of_v<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.Get() == _b.Get();
}

template<typename T>
bool operator==(const Ptr<T>& _a, std::nullptr_t)
{ return _a.Get() == nullptr; }

template<typename T>
bool operator==(std::nullptr_t, const Ptr<T>& _a)
{ return _a.Get() == nullptr; }

template<typename T, typename U>
bool operator!=(const Ptr<T>& _a, const Ptr<U>& _b)
{
    static_assert(std::is_base_of_v<T, U> || std::is_base_of_v<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.Get() != _b.Get();
}

template<typename T>
bool operator!=(const Ptr<T>& _a, std::nullptr_t)
{ return _a.Get() != nullptr; }

template<typename T>
bool operator!=(std::nullptr_t, const Ptr<T>& _a)
{ return _a.Get() != nullptr; }

template<typename T, typename U>
bool operator<(const Ptr<T>& _a, const Ptr<U>& _b)
{
    static_assert(std::is_base_of<T, U> || std::is_base_of<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.Get() < _b.Get();
}

template<typename T, typename U>
bool operator==(const details::PtrRef<Ptr<T>>& _a, const details::PtrRef<Ptr<U>>& _b)
{
    static_assert(std::is_base_of<T, U> || std::is_base_of<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.GetAddressOf() == _b.GetAddressOf();
}

template<typename T>
bool operator==(const details::PtrRef<Ptr<T>>& _a, std::nullptr_t)
{ return _a.GetAddressOf() == nullptr; }

template<typename T>
bool operator==(std::nullptr_t, const details::PtrRef<Ptr<T>>& _a)
{ return _a.GetAddressOf() == nullptr; }

template<typename T>
bool operator==(const details::PtrRef<Ptr<T>>& _a, void* _b)
{ return _a.GetAddressOf() == _b; }

template<typename T>
bool operator==(void* _b, const details::PtrRef<Ptr<T>>& _a)
{ return _a.GetAddressOf() == _b; }

template<typename T, typename U>
bool operator!=(const details::PtrRef<Ptr<T>>& _a, const details::PtrRef<Ptr<U>>& _b)
{
    static_assert(std::is_base_of<T, U> || std::is_base_of<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.GetAddressOf() != _b.GetAddressOf();
}

template<typename T>
bool operator!=(const details::PtrRef<Ptr<T>>& _a, std::nullptr_t)
{ return _a.GetAddressOf() != nullptr; }

template<typename T>
bool operator!=(std::nullptr_t, const details::PtrRef<Ptr<T>>& _a)
{ return _a.GetAddressOf() != nullptr; }

template<typename T>
bool operator!=(const details::PtrRef<Ptr<T>>& _a, void* _b)
{ return _a.GetAddressOf() != _b; }

template<typename T>
bool operator!=(void* _b, const details::PtrRef<Ptr<T>>& _a)
{ return _a.GetAddressOf() != _b; }

template<typename T, typename U>
bool operator<(const details::PtrRef<Ptr<T>>& _a, const details::PtrRef<Ptr<U>>& _b)
{
    static_assert(std::is_base_of<T, U> || std::is_base_of<U, T>, "'T' and 'U' pointers must be comparable");
    return _a.GetAddressOf() < _b.GetAddressOf();
}

}// namespace util
}// namespace buma3d
