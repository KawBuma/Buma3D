#pragma once

// �v���b�g�t�H�[��

#ifndef B3D_PLATFORM_USING

#   define B3D_PLATFORM_WINDOWS (1)
#   define B3D_PLATFORM_ANDROID (2)

#   ifdef _WIN64
#       define B3D_PLATFORM_USING (B3D_PLATFORM_WINDOWS)
#   endif // WIN64

#   ifdef __ANDROID__
#       define B3D_PLATFORM_USING (B3D_PLATFORM_ANDROID)
#   endif // __ANDROID__


    // �v���b�g�t�H�[�����ʒ萔
#   if B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS
#       define B3D_PLATFORM_IS_USED_WINDOWS (1)
#       define B3D_PLATFORM_IS_USED_ANDROID (0)
#   endif // B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS

#   if B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID
#       define B3D_PLATFORM_IS_USED_WINDOWS (0)
#       define B3D_PLATFORM_IS_USED_ANDROID (1)
#   endif // B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID


    // API�V���{���錾�̑������`
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
        ���̌`����__declspec�́A�C�ӂ̃N���X�錾�ɓK�p�ł��܂��B
        �������A�����ȃC���^�[�t�F�[�X�N���X�A�܂�A���ꎩ�̂ł̓C���X�^���X������Ȃ��N���X�ɂ̂ݓK�p����K�v������܂��B
        __declspec�́A�R���p�C���[���R�[�h�𐶐����āA�N���X�̃R���X�g���N�^�[����уf�X�g���N�^�[��vfptr������������̂��~���܂��B
        �����̏ꍇ�A����ɂ��A�N���X�Ɋ֘A�t�����Ă���vtable�ւ̎Q�Ƃ݂̂��폜����A�����J�ɂ���č폜����܂��B
        ���̌`����__declspec���g�p����ƁA�R�[�h�T�C�Y��啝�ɍ팸�ł��܂��B
        novtable�Ń}�[�N���ꂽ�N���X���C���X�^���X�����Ă���N���X�����o�[�ɃA�N�Z�X���悤�Ƃ���ƁA�A�N�Z�X�ᔽ���������܂��B*/
#       define B3D_INTERFACE struct B3D_API __declspec(novtable)

#   else
#       define B3D_INTERFACE struct B3D_API 
#   endif

    // �Ăяo���K��
#   if B3D_PLATFORM_IS_USED_WINDOWS
#       define B3D_APIENTRY __stdcall
#   else
#       define B3D_APIENTRY
#   endif // B3D_PLATFORM_IS_USED_WINDOWS


    // 1�x���Q�Ƃ���Ȃ��ꍇ��h��
#   define B3D_UNREFERENCED(...) (__VA_ARGS__)

#endif // !B3D_PLATFORM_USING
