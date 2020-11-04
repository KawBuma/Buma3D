#include "Buma3DPCH.h"
#include "RayTracingPipelineStateVk.h"

/*
A shader binding table is a resource which establishes the relationship between the ray tracing pipeline and the acceleration structures that were built for the ray tracing pipeline.
It indicates the shaders that operate on each geometry in an acceleration structure.
In addition, it contains the resources accessed by each shader, including indices of textures, buffer device addresses, and constants.
The application allocates and manages shader binding tables as VkBuffer objects.
シェーダーバインディングテーブルは、レイトレーシングパイプラインと、レイトレーシングパイプライン用に構築されたアクセラレーション構造の間の関係を確立するリソースです。
アクセラレーション構造の各ジオメトリで動作するシェーダーを示します。
さらに、テクスチャのインデックス、バッファデバイスのアドレス、定数など、各シェーダーがアクセスするリソースが含まれています。
アプリケーションは、シェーダーバインディングテーブルをVkBufferオブジェクトとして割り当て、管理します。

Each entry in the shader binding table consists of shaderGroupHandleSize bytes of data as queried by vkGetRayTracingShaderGroupHandlesKHR to refer to the shader that it invokes.
The remainder of the data specified by the stride is application-visible data that can be referenced by a ShaderRecordBufferKHR block in the shader.
シェーダーバインディングテーブルの各エントリは、vkGetRayTracingShaderGroupHandlesKHRによって呼び出された
シェーダーを参照するためにvkGetRayTracingShaderGroupHandlesKHRによってクエリされた、shaderGroupHandleSizeバイトのデータで構成されます。
ストライドで指定された残りのデータは、アプリケーションから見えるデータであり、シェーダーのShaderRecordBufferKHRブロックで参照できます。

The shader binding tables to use in a ray tracing pipeline are passed to the vkCmdTraceRaysNV, vkCmdTraceRaysKHR, or vkCmdTraceRaysIndirectKHR commands.
Shader binding tables are read-only in shaders that are executing on the ray tracing pipeline.
Accesses to the shader binding table from ray tracing pipelines must be synchronized with the VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR pipeline stage and an access type of VK_ACCESS_SHADER_READ_BIT.
レイトレーシングパイプラインで使用するシェーダーバインディングテーブルは、vkCmdTraceRaysNV、vkCmdTraceRaysKHR、またはvkCmdTraceRaysIndirectKHRコマンドに渡されます。
シェーダーバインディングテーブルは、レイトレーシングパイプラインで実行されているシェーダーでは読み取り専用です。
レイトレーシングパイプラインからシェーダーバインディングテーブルへのアクセスは、VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHRパイプラインステージおよびアクセスタイプVK_ACCESS_SHADER_READ_BITと同期する必要があります。
*/

namespace buma3d
{



}// namespace buma3d
