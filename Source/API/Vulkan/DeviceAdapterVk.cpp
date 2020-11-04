#include "Buma3DPCH.h"
#include "DeviceAdapterVk.h"

namespace buma3d
{

B3D_APIENTRY DeviceAdapterVk::DeviceAdapterVk()
    : ref_count             { 1 }
    , name                  {}
    , desc                  {}
    , factory               {}
    , physical_device       {}
    , api_version           {}
    , pd_data               {}
    , inspfn                {}
    , mem_props             {}
    , phys_dev_group_props  {}
    , queue_properties_map  {}
{
}

B3D_APIENTRY DeviceAdapterVk::~DeviceAdapterVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::Init(DeviceFactoryVk* _factory, VkPhysicalDevice _phys_device)
{
    (factory = _factory)->AddRef();
    physical_device = _phys_device;
    inspfn = &factory->GetInstancePFN();

    // メモリプロパティを取得
    mem_props.Init(physical_device);

    pd_data = B3DMakeUnique(util::PHYSICAL_DEVICE_DATA);
    B3D_RET_IF_FAILED(GetFeatures());
    B3D_RET_IF_FAILED(GetProperties());

    InitDesc();

    uint32_t num_qf_props = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &num_qf_props, nullptr);
    util::DyArray<VkQueueFamilyProperties2> qf_props(num_qf_props, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &num_qf_props, qf_props.data());

    uint32_t qf_index = 0;
    for (auto& i : qf_props)
    {
        auto type = util::GetB3DCommandType(i.queueFamilyProperties.queueFlags);
        if (type != -1)
        {
            auto&& prop = *(queue_properties_map[type] = B3DMakeUnique(QUEUE_PROPERTIES_MAP));
            prop.queue_family_index = qf_index;
            prop.queue_flags        = i.queueFamilyProperties.queueFlags;
        }
        qf_index++;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::Init(DeviceFactoryVk* _factory, const VkPhysicalDeviceGroupProperties& _pd_props)
{
    phys_dev_group_props = B3DMakeUniqueArgs(VkPhysicalDeviceGroupProperties, _pd_props);
    B3D_RET_IF_FAILED(Init(_factory, _pd_props.physicalDevices[0]));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::GetFeatures()
{
    auto ins_ver = factory->GetInstanceAPIVersion();

#define MAKECHAIN(x,stype) x = B3DMakeUniqueArgs(decltype(x)::element_type, decltype(x)::element_type stype)

    pd_data->features2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };

    // 取得したいpNextチェーン機能を作成
    auto&& features_chain = pd_data->features_chain;
    auto last_pnext = util::ConnectPNextChains
    (
        pd_data->features2
        , *(MAKECHAIN(features_chain.astc_decode_features_ext                        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT                        }))
        , *(MAKECHAIN(features_chain.blend_operation_advanced_features_ext           , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT           }))
        , *(MAKECHAIN(features_chain.buffer_device_address_features_ext              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT              }))
        , *(MAKECHAIN(features_chain.coherent_memory_features_amd                    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD                    }))
        , *(MAKECHAIN(features_chain.compute_shader_derivatives_features_nv          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV          }))
        , *(MAKECHAIN(features_chain.conditional_rendering_features_ext              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT              }))
        , *(MAKECHAIN(features_chain.cooperative_matrix_features_nv                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV                  }))
        , *(MAKECHAIN(features_chain.corner_sampled_image_features_nv                , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV                }))
        , *(MAKECHAIN(features_chain.coverage_reduction_mode_features_nv             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV             }))
        , *(MAKECHAIN(features_chain.dedicated_allocation_image_aliasing_features_nv , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV }))
        , *(MAKECHAIN(features_chain.depth_clip_enable_features_ext                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT                  }))
        , *(MAKECHAIN(features_chain.device_generated_commands_features_nv           , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV           }))
    //  , *(MAKECHAIN(features_chain.diagnostics_config_features_nv                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV                  }))
        , *(MAKECHAIN(features_chain.exclusive_scissor_features_nv                   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV                   }))
        , *(MAKECHAIN(features_chain.fragment_density_map_features_ext               , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT               }))
        , *(MAKECHAIN(features_chain.fragment_shader_barycentric_features_nv         , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV         }))
        , *(MAKECHAIN(features_chain.fragment_shader_interlock_features_ext          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT          }))
        , *(MAKECHAIN(features_chain.index_type_uint8_features_ext                   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT                   }))
        , *(MAKECHAIN(features_chain.inline_uniform_block_features_ext               , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT               }))
        , *(MAKECHAIN(features_chain.line_rasterization_features_ext                 , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT                 }))
        , *(MAKECHAIN(features_chain.memory_priority_features_ext                    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT                    }))
        , *(MAKECHAIN(features_chain.mesh_shader_features_nv                         , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV                         }))
        , *(MAKECHAIN(features_chain.performance_query_features_khr                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR                  }))
        , *(MAKECHAIN(features_chain.pipeline_creation_cache_control_features_ext    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT    }))
        , *(MAKECHAIN(features_chain.pipeline_executable_properties_features_khr     , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR     }))
    //  , *(MAKECHAIN(features_chain.ray_tracing_features_khr                        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR                        }))
        , *(MAKECHAIN(features_chain.representative_fragment_test_features_nv        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV        }))
        , *(MAKECHAIN(features_chain.shader_clock_features_khr                       , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR                       }))
        , *(MAKECHAIN(features_chain.shader_demote_to_helper_invocation_features_ext , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT }))
        , *(MAKECHAIN(features_chain.shader_image_footprint_features_nv              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV              }))
        , *(MAKECHAIN(features_chain.shader_integer_functions2_features_intel        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL       }))
        , *(MAKECHAIN(features_chain.shader_sm_builtins_features_nv                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV                  }))
        , *(MAKECHAIN(features_chain.shading_rate_image_features_nv                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV                  }))
        , *(MAKECHAIN(features_chain.subgroup_size_control_features_ext              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT              }))
        , *(MAKECHAIN(features_chain.texel_buffer_alignment_features_ext             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT             }))
        , *(MAKECHAIN(features_chain.texture_compression_astc_hdr_features_ext       , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT       }))
        , *(MAKECHAIN(features_chain.transform_feedback_features_ext                 , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT                 }))
        , *(MAKECHAIN(features_chain.vertex_attribute_divisor_features_ext           , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT           }))
        , *(MAKECHAIN(features_chain.ycbcr_image_arrays_features_ext                 , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT                 }))
    );

    if (ins_ver >= VK_API_VERSION_1_2)
    {
        last_pnext = util::ConnectPNextChains
        (
            last_pnext
            , *(MAKECHAIN(features_chain.vulkan11_features, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES }))
            , *(MAKECHAIN(features_chain.vulkan12_features, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES }))
        );
    }
    else
    {
        auto&& features11 = *(features_chain.features11 = B3DMakeUnique(util::PHYSICAL_DEVICE_FEATURES_CHAIN::FEATURES_VK11));
        auto&& features12 = *(features_chain.features12 = B3DMakeUnique(util::PHYSICAL_DEVICE_FEATURES_CHAIN::FEATURES_VK12));
        last_pnext = util::ConnectPNextChains
        (
            last_pnext
            , *(MAKECHAIN(features11.bit16_storage_features                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES                  }))
            , *(MAKECHAIN(features11.multiview_features                      , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES                      }))
            , *(MAKECHAIN(features11.variable_pointers_features              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES              }))
            , *(MAKECHAIN(features11.protected_memory_features               , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES               }))
            , *(MAKECHAIN(features11.sampler_ycbcr_conversion_features       , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES       }))
            , *(MAKECHAIN(features11.shader_draw_parameters_features         , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES         }))
            , *(MAKECHAIN(features12.bit8_storage_features                   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES                   }))
            , *(MAKECHAIN(features12.shader_atomic_int64_features            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES            }))
            , *(MAKECHAIN(features12.shader_float16_int8_features            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES            }))
            , *(MAKECHAIN(features12.descriptor_indexing_features            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES            }))
            , *(MAKECHAIN(features12.scalar_block_layout_features            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES            }))
            , *(MAKECHAIN(features12.imageless_framebuffer_features          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES          }))
            , *(MAKECHAIN(features12.uniform_buffer_standard_layout_features , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES }))
            , *(MAKECHAIN(features12.shader_subgroup_extended_types_features , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES }))
            , *(MAKECHAIN(features12.separate_depth_stencil_layouts_features , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES }))
            , *(MAKECHAIN(features12.host_query_reset_features               , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES               }))
            , *(MAKECHAIN(features12.timeline_semaphore_features             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES             }))
            , *(MAKECHAIN(features12.buffer_device_address_features          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES          }))
            , *(MAKECHAIN(features12.vulkan_memory_model_features            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES            }))
        );
    }
    // VkDevice作成時に使用可能な有効にする機能の取得
    vkGetPhysicalDeviceFeatures2(physical_device, &pd_data->features2);

#undef MAKECHAIN

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::GetProperties()
{
    auto ins_ver = factory->GetInstanceAPIVersion();

#define MAKECHAIN(x,stype) x = B3DMakeUniqueArgs(decltype(x)::element_type, decltype(x)::element_type stype)

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceProperties2.html#_description
    // 物理デバイスプロパティ
    auto&& pd_props = pd_data->properties2;
    pd_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };

    // 取得したいpNextチェーンプロパティを作成
    auto&& props_chain = pd_data->properties_chain;
    auto last_pnext = util::ConnectPNextChains
    (
        pd_props
        , *(MAKECHAIN(props_chain.blend_operation_advanced_props_ext      , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT      }))
        , *(MAKECHAIN(props_chain.conservative_rasterization_props_ext    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT    }))
        , *(MAKECHAIN(props_chain.cooperative_matrix_props_nv             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV             }))
        , *(MAKECHAIN(props_chain.device_generated_commands_props_nv      , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV      }))
        , *(MAKECHAIN(props_chain.discard_rectangle_props_ext             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT             }))
        , *(MAKECHAIN(props_chain.external_memory_host_props_ext          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT          }))
        , *(MAKECHAIN(props_chain.fragment_density_map_props_ext          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT          }))
        , *(MAKECHAIN(props_chain.inline_uniform_block_props_ext          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES_EXT          }))
        , *(MAKECHAIN(props_chain.line_rasterization_props_ext            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT            }))
        , *(MAKECHAIN(props_chain.mesh_shader_props_nv                    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV                    }))
        , *(MAKECHAIN(props_chain.multiview_per_view_attributes_props_nvx , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX }))
        , *(MAKECHAIN(props_chain.pci_bus_info_props_ext                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT                  }))
        , *(MAKECHAIN(props_chain.performance_query_props_khr             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR             }))
        , *(MAKECHAIN(props_chain.push_descriptor_props_khr               , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR               }))
    //  , *(MAKECHAIN(props_chain.ray_tracing_props_khr                   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR                   }))
        , *(MAKECHAIN(props_chain.ray_tracing_props_nv                    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV                    }))
        , *(MAKECHAIN(props_chain.sample_locations_props_ext              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT              }))
        , *(MAKECHAIN(props_chain.shader_core_props2_amd                  , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD                 }))
        , *(MAKECHAIN(props_chain.shader_core_props_amd                   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD                   }))
        , *(MAKECHAIN(props_chain.shader_sm_builtins_props_nv             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV             }))
        , *(MAKECHAIN(props_chain.shading_rate_image_props_nv             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV             }))
        , *(MAKECHAIN(props_chain.subgroup_size_control_props_ext         , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT         }))
        , *(MAKECHAIN(props_chain.texel_buffer_alignment_props_ext        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT        }))
        , *(MAKECHAIN(props_chain.transform_feedback_props_ext            , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT            }))
        , *(MAKECHAIN(props_chain.vertex_attribute_divisor_props_ext      , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT      }))
    );
    if (ins_ver >= VK_API_VERSION_1_2)
    {
        // pNextをセット
        last_pnext = util::ConnectPNextChains
        (
            last_pnext
            , *(MAKECHAIN(props_chain.vulkan11_props, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES }))
            , *(MAKECHAIN(props_chain.vulkan12_props, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES }))
        );
    }
    else
    {  
        auto&& props11 = *(props_chain.props11 = B3DMakeUnique(util::PHYSICAL_DEVICE_PROPERTIES_CHAIN::PROPS_11));
        auto&& props12 = *(props_chain.props12 = B3DMakeUnique(util::PHYSICAL_DEVICE_PROPERTIES_CHAIN::PROPS_12));
        // pNextをセット
        last_pnext = util::ConnectPNextChains
        (
            last_pnext
            , *(MAKECHAIN(props11.id_props                    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES                    }))
            , *(MAKECHAIN(props11.subgroup_props              , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES              }))
            , *(MAKECHAIN(props11.point_clipping_props        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES        }))
            , *(MAKECHAIN(props11.multiview_props             , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES             }))
            , *(MAKECHAIN(props11.protected_memory_props      , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES      }))
            , *(MAKECHAIN(props11.maintenance3_props          , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES         }))
            , *(MAKECHAIN(props12.driver_props                , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES                }))
            , *(MAKECHAIN(props12.float_controls_props        , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES        }))
            , *(MAKECHAIN(props12.descriptor_indexing_props   , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES   }))
            , *(MAKECHAIN(props12.depth_stencil_resolve_props , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES }))
            , *(MAKECHAIN(props12.sampler_filter_minmax_props , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES }))
            , *(MAKECHAIN(props12.timeline_semaphore_props    , { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES    }))
        );
    }
    // TODO: 2020/05/19: プロパティ取得時に検証エラーが発行されるが検証レイヤー自体のバグの模様。https://www.reddit.com/r/vulkan/comments/gil48u/validation_layer_reporting_an_error_on/
    vkGetPhysicalDeviceProperties2(physical_device, &pd_props);

    // 物理デバイスのVulkanAPIバージョン
    // デバイスレベルの機能サポート用分岐等に使用
    api_version = pd_props.properties.apiVersion;

#undef MAKECHAIN

    ///* Nvidia Nsight Aftermath SDK for Vulkanを使用してデバイスクラッシュダンプをエラー報告メカニズムに統合するアプリケーションは、
    //この拡張機能を使用して、デバイスクラッシュダンプの作成に関連するオプションを構成できます。*/
    //VkDeviceDiagnosticsConfigCreateInfoNV diagnostics_config_create_info_nv = { VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV };
    //if (pd_data->features_chain.diagnostics_config_features_nv->diagnosticsConfig)
    //{
    // diagnostics_config_create_info_nv.flags =
    //  VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV |
    //  VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |
    //  VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV;
    //
    // if (last_pnext)
    //  *last_pnext = &diagnostics_config_create_info_nv;
    //}

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceAdapterVk::InitDesc()
{
    auto&& pd_props = pd_data->properties2;
    VkPhysicalDeviceIDProperties id_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
    if (pd_data->properties_chain.vulkan11_props)
    {
        auto&& idp = *pd_data->properties_chain.vulkan11_props;
        util::MemCopyArray(id_props.deviceUUID, idp.deviceUUID, VK_UUID_SIZE);
        util::MemCopyArray(id_props.driverUUID, idp.driverUUID, VK_UUID_SIZE);
        util::MemCopyArray(id_props.deviceLUID, idp.deviceLUID, VK_LUID_SIZE);
        id_props.deviceNodeMask = idp.deviceNodeMask;
        id_props.deviceLUIDValid = idp.deviceLUIDValid;
    }
    else
    {
        id_props = *pd_data->properties_chain.props11->id_props;
    }
    
    auto&& props = pd_props.properties;
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        static const char* PHYSICAL_DEVICE_TYPE_NAMES[] = 
        {
              "VK_PHYSICAL_DEVICE_TYPE_OTHER"
            , "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"
            , "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"
            , "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"
            , "VK_PHYSICAL_DEVICE_TYPE_CPU"
        };
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            ss << "VkPhysicalDeviceProperties"                                                                     << "\n";
            ss << "\tapiVersion    : "   << util::GetVulkanVersionString(pd_props.properties.apiVersion)           << "\n";
            ss << "\tdriverVersion : "   << util::GetVulkanVersionString(pd_props.properties.driverVersion)        << "\n";
            ss << "\tvendorID      : 0x" << std::hex << pd_props.properties.vendorID                               << "\n";
            ss << "\tdeviceID      : 0x" <<             pd_props.properties.deviceID << std::dec                   << "\n";
            ss << "\tdeviceType    : "   <<             PHYSICAL_DEVICE_TYPE_NAMES[pd_props.properties.deviceType] << "\n";
            ss << "\tdeviceName    : "   <<             pd_props.properties.deviceName                             << "\n";
            ss << "\n";                                                                                            
            ss << "VkPhysicalDeviceIDProperties"                                                                   << "\n";
            ss << "\tdeviceUUID      : " << hlp::GetUUIDString(id_props.deviceUUID)                                << "\n";
            ss << "\tdriverUUID      : " << hlp::GetUUIDString(id_props.driverUUID)                                << "\n";
            ss << "\tdeviceLUIDValid : " << (id_props.deviceLUIDValid ? "VK_TRUE" : "VK_FALSE")                    << "\n";
            if (id_props.deviceLUIDValid == VK_TRUE)
            {
                /* 特定の外部オブジェクトをドライバーコンポーネント間で共有できるかどうかを決定するには、
                deviceUUIDやdriverUUIDを使用する必要があります。
                このような制限は、特定のオブジェクトタイプの互換性テーブルで定義されているとおりに存在します。
                deviceLUIDValidがVK_FALSEの場合、deviceLUIDおよびdeviceNodeMaskの値は未定義です。
                deviceLUIDValidがVK_TRUEで、VulkanがWindowsオペレーティングシステムで実行されている場合、
                deviceLUIDのコンテンツはLUIDオブジェクトにキャストでき、
                physicalDeviceに対応するIDXGIAdapter1オブジェクトのローカルで一意の識別子と等しい必要があります。*/
                ss << "\tdeviceLUID      : " << hlp::GetLUIDString(id_props.deviceLUID) << "\n";
                /* VulkanがDirect3D12 APIをサポートするオペレーティングシステムで実行されていて、
                physicalDeviceがリンクされたデバイスアダプターの個々のデバイスに対応している場合、
                deviceNodeMaskはphysicalDeviceに対応するDirect3D 12ノードを識別します。それ以外の場合、deviceNodeMaskは1でなければなりません。*/
                ss << "\tdeviceNodeMask  : "<< std::hex << std::hex << id_props.deviceNodeMask << std::dec << "\n";
            }
            util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                  , ss.str().c_str());
        }
    }

    util::MemCopyArray(desc.device_name, props.deviceName, std::strlen(props.deviceName));
    std::memcpy(desc.adapter_luid, id_props.deviceLUID, sizeof(desc.adapter_luid));
    desc.vendor_id = props.vendorID;
    desc.device_id = props.deviceID;

    // デバイスのメモリサイズの取得
    auto&& mp = mem_props.GetVkMemoryProperties2();
    for (size_t i = 0; i < mp.memoryProperties.memoryHeapCount; i++)
    {
        auto&& i_heap = mp.memoryProperties.memoryHeaps[i];
        if (desc.dedicated_video_memory < i_heap.size 
            && (i_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            desc.dedicated_video_memory = i_heap.size;
        }
        else if (desc.shared_system_memory < i_heap.size && i_heap.flags == 0)
        {
            desc.shared_system_memory = i_heap.size;
        }
    }

    // このデバイスアダプタが所有する、抽象化可能な物理デバイスの数
    desc.node_count = phys_dev_group_props ? phys_dev_group_props->physicalDeviceCount : 1;
}

void 
B3D_APIENTRY DeviceAdapterVk::Uninit()
{
    name.reset();
    hlp::SafeRelease(factory);
    physical_device = VK_NULL_HANDLE;
    pd_data.reset();
    api_version = 0;
    queue_properties_map = {};
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::Create(DeviceFactoryVk* _factory, VkPhysicalDevice _phys_device, DeviceAdapterVk** _dst)
{
    util::Ptr<DeviceAdapterVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceAdapterVk));
    B3D_RET_IF_FAILED(ptr->Init(_factory, _phys_device));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceAdapterVk::CreateForDeviceGroup(DeviceFactoryVk* _factory, const VkPhysicalDeviceGroupProperties& _pd_props, DeviceAdapterVk** _dst)
{
    // デバイスグループを使用するマルチノードアダプタとして作成します
    util::Ptr<DeviceAdapterVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceAdapterVk));
    B3D_RET_IF_FAILED(ptr->Init(_factory, _pd_props));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceAdapterVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceAdapterVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceAdapterVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceAdapterVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DeviceAdapterVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_ADAPTER_DESC&
B3D_APIENTRY DeviceAdapterVk::GetDesc() const
{
    return desc;
}

uint32_t 
B3D_APIENTRY DeviceAdapterVk::GetCommandQueueProperties(COMMAND_QUEUE_PROPERTIES* _properties)
{
    uint32_t num_qf_props = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &num_qf_props, nullptr);

    if (_properties)
    {
        util::DyArray<VkQueueFamilyProperties2> qf_props(num_qf_props, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &num_qf_props, qf_props.data());

        auto qf_data = qf_props.data();
        for (uint32_t i = 0; i < num_qf_props; i++)
        {
            auto&& qf = qf_data[i].queueFamilyProperties;
            auto&& dst = _properties[i];
            dst.type           = util::GetB3DCommandType(qf.queueFlags);
            dst.num_max_queues = qf.queueCount;
        }
    }

    return num_qf_props;
}

BMRESULT
B3D_APIENTRY DeviceAdapterVk::CreateSurface(const SURFACE_DESC& _desc, ISurface** _dst)
{
    util::Ptr<SurfaceVk> ptr;
    B3D_RET_IF_FAILED(SurfaceVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceAdapterVk::QueryPresentationSupport(COMMAND_TYPE _queue_type, const ISurface* _surface)
{
    BMRESULT result = BMRESULT_FAILED_NOT_SUPPORTED;
    auto&& prop = queue_properties_map[_queue_type];

    if (!inspfn->vkGetPhysicalDeviceSurfaceSupportKHR && !prop)
        return result;

    VkBool32 is_supported = VK_FALSE;
    auto vkr = inspfn->vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, prop->queue_family_index, _surface->As<SurfaceVk>()->GetVkSurface(), &is_supported);
    result = VKR_TRACE_IF_FAILED(vkr);

    if (hlp::IsSucceed(result) && is_supported)
        result = BMRESULT_SUCCEED;

    return result;
}

const util::PHYSICAL_DEVICE_DATA& 
B3D_APIENTRY DeviceAdapterVk::GetPhysicalDeviceData() const
{
    return *pd_data;
}

DeviceFactoryVk*
B3D_APIENTRY DeviceAdapterVk::GetDeviceFactoryVk()
{
    return factory;
}

VkPhysicalDevice 
B3D_APIENTRY DeviceAdapterVk::GetVkPhysicalDevice() const
{
    return physical_device;
}

uint32_t 
B3D_APIENTRY DeviceAdapterVk::GetPhysicalDeviceAPIVersion() const
{
    return api_version;
}

const InstancePFN& 
B3D_APIENTRY DeviceAdapterVk::GetInstancePFN() const
{
    return *inspfn;
}

DeviceFactoryVk* 
B3D_APIENTRY DeviceAdapterVk::GetDeviceFactory() const
{
    return factory;
}

bool
B3D_APIENTRY DeviceAdapterVk::IsEnabledDebug()
{
    return factory->IsEnabledDebug();
}

void
B3D_APIENTRY DeviceAdapterVk::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const char* _str)
{
    factory->AddMessageFromB3D(_severity, _category, _str);
}

const VkPhysicalDeviceGroupProperties* 
B3D_APIENTRY DeviceAdapterVk::GetVkPhysicalDeviceGroupProperties() const
{
    // デバイスグループを使用しない非マルチノードアダプタの場合nullptr
    return phys_dev_group_props.get();
}

util::MemoryProperties& 
B3D_APIENTRY DeviceAdapterVk::GetMemoryProperties()
{
    return mem_props;
}

const util::StArray<util::UniquePtr<DeviceAdapterVk::QUEUE_PROPERTIES_MAP>, COMMAND_TYPE_NUM_TYPES>&
B3D_APIENTRY DeviceAdapterVk::QueuePropertiesMap() const
{
    return queue_properties_map;
}


}// namespace buma3d
