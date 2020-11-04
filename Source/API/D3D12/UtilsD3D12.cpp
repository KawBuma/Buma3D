#include "Buma3DPCH.h"
#include "UtilsD3D12.h"

namespace buma3d
{
namespace util
{

BMRESULT GetBMResultFromHR(HRESULT _hr)
{
    if (SUCCEEDED(_hr))
        return BMRESULT_SUCCEED;

    BMRESULT result{};
    switch (_hr)
    {
    case E_INVALIDARG: // An invalid parameter was passed to the returning function.
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;

    case E_OUTOFMEMORY: // Direct3D could not allocate sufficient memory to complete the call.
        result = BMRESULT_FAILED_OUT_OF_SYSTEM_MEMORY;
        break;

    case DXGI_ERROR_DEVICE_REMOVED: // The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason. 
        result = BMRESULT_FAILED_DEVICE_REMOVED;
        break;

    case DXGI_ERROR_WAIT_TIMEOUT: // The time-out interval elapsed before the next desktop frame was available. 
        result = BMRESULT_SUCCEED_TIMEOUT;// エラー版TIMEOUTを用意すべき?
        break;

    case DXGI_ERROR_UNSUPPORTED: // The requested functionality is not supported by the device or the driver. 
        result = BMRESULT_FAILED_NOT_SUPPORTED;
        break;

    // NOTE: 定数が見つからない。エラーの値も定義なし。
    //case D3D12_ERROR_FILE_NOT_FOUND    : // The file was not found.
    //case D3D12_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: // There are too many unique instances of a particular type of state object.
    //case D3D12_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS : // There are too many unique instances of a particular type of view object.

    case D3D12_ERROR_ADAPTER_NOT_FOUND           : // The blob provided does not match the adapter that the device was created on.
    case D3D12_ERROR_DRIVER_VERSION_MISMATCH     : // The blob provided was created for a different version of the driver, and must be re-created.
    case E_FAIL                                  : // Attempted to create a device with the debug layer enabled and the layer is not installed.
    case E_NOTIMPL                               : // The method call isn't implemented with the passed parameter combination.
    case S_FALSE                                 : // Alternate success value, indicating a successful but nonstandard completion (the precise meaning depends on context).
    case DXGI_ERROR_ACCESS_DENIED                : // You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access. 
    case DXGI_ERROR_ACCESS_LOST                  : // The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop. 
    case DXGI_ERROR_ALREADY_EXISTS               : // The desired element already exists. This is returned by DXGIDeclareAdapterRemovalSupport if it is not the first time that the function is called. 
    case DXGI_ERROR_CANNOT_PROTECT_CONTENT       : // DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection. 
    case DXGI_ERROR_DEVICE_HUNG                  : // The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed. 
    case DXGI_ERROR_DEVICE_RESET                 : // The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device. 
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR        : // The driver encountered a problem and was put into the device removed state. 
    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT    : // An event (for example, a power cycle) interrupted the gathering of presentation statistics. 
    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE : // The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership. 
    case DXGI_ERROR_INVALID_CALL                 : // The application provided invalid parameter data; this must be debugged and fixed before the application is released. 
    case DXGI_ERROR_MORE_DATA                    : // The buffer supplied by the application is not big enough to hold the requested data. 
    case DXGI_ERROR_NAME_ALREADY_EXISTS          : // The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource. 
    case DXGI_ERROR_NONEXCLUSIVE                 : // A global counter resource is in use, and the Direct3D device can't currently use the counter resource. 
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE      : // The resource or request is not currently available, but it might become available later. 
    case DXGI_ERROR_NOT_FOUND                    : // When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFactory::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range. 
    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED   : // Reserved 
    case DXGI_ERROR_REMOTE_OUTOFMEMORY           : // Reserved 
    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE     : // The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed. 
    case DXGI_ERROR_SDK_COMPONENT_MISSING        : // The operation depends on an SDK component that is missing or mismatched. 
    case DXGI_ERROR_SESSION_DISCONNECTED         : // The Remote Desktop Services session is currently disconnected. 
    case DXGI_ERROR_WAS_STILL_DRAWING            : // The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation.
    default:
        result = BMRESULT_FAILED;
        break;
    }

    return result;
}


}// namespace util
}// namespace buma3d
