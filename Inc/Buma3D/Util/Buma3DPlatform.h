#pragma once

// プラットフォーム

#ifndef B3D_PLATFORM_USING

#   define B3D_PLATFORM_WINDOWS (1)
#   define B3D_PLATFORM_ANDROID (2)

#   ifdef _WIN64
#       define B3D_PLATFORM_USING (B3D_PLATFORM_WINDOWS)
#   endif // WIN64

#   ifdef __ANDROID__
#       define B3D_PLATFORM_USING (B3D_PLATFORM_ANDROID)
#   endif // __ANDROID__


    // プラットフォーム識別定数
#   if B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS
#       define B3D_PLATFORM_IS_USED_WINDOWS (1)
#       define B3D_PLATFORM_IS_USED_ANDROID (0)
#   endif // B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS

#   if B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID
#       define B3D_PLATFORM_IS_USED_WINDOWS (0)
#       define B3D_PLATFORM_IS_USED_ANDROID (1)
#   endif // B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID


    // APIシンボル宣言の属性を定義
#   if defined(B3D_DLLEXPORT) && B3D_PLATFORM_IS_USED_WINDOWS
#       define B3D_DLL_API extern "C" __declspec(dllexport)
#       define B3D_API 

#   elif defined(B3D_DLLIMPORT) && B3D_PLATFORM_IS_USED_WINDOWS
#       define B3D_DLL_API extern "C" __declspec(dllimport)
#       define B3D_API 

#   else
#       define B3D_DLL_API 
#       define B3D_API 

#   endif // defined(B3D_DLLEXPORT) && B3D_PLATFORM_IS_USED_WINDOWS

#   if B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS
        /* __declspec(novtable)
        この形式の__declspecは、任意のクラス宣言に適用できます。
        ただし、純粋なインターフェースクラス、つまり、それ自体ではインスタンス化されないクラスにのみ適用する必要があります。
        __declspecは、コンパイラーがコードを生成して、クラスのコンストラクターおよびデストラクターでvfptrを初期化するのを停止します。
        多くの場合、これにより、クラスに関連付けられているvtableへの参照のみが削除され、リンカによって削除されます。
        この形式の__declspecを使用すると、コードサイズを大幅に削減できます。
        novtableでマークされたクラスをインスタンス化してからクラスメンバーにアクセスしようとすると、アクセス違反が発生します。*/
#       define B3D_INTERFACE struct B3D_API __declspec(novtable)

#   else
#       define B3D_INTERFACE struct B3D_API 
#   endif

    // 呼び出し規約
#   if B3D_PLATFORM_IS_USED_WINDOWS
#       define B3D_APIENTRY __stdcall
#   else
#       define B3D_APIENTRY
#   endif // B3D_PLATFORM_IS_USED_WINDOWS


    // 1度も参照されない場合を防ぐ
#   define B3D_UNREFERENCED(...) (__VA_ARGS__)

#endif // !B3D_PLATFORM_USING
