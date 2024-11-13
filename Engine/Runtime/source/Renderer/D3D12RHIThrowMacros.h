#pragma once

// HRESULT hr should exist in the local scope for these macros to work

#define D3D12RHI_EXCEPT_NOINFO(hr) Renderer::D3D12RHI::HrException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Renderer::D3D12RHI::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define D3D12RHI_EXCEPT(hr) Renderer::D3D12RHI::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw D3D12RHI_EXCEPT(hr)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::D3D12RHI::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Renderer::D3D12RHI::InfoException( __LINE__,__FILE__,v);}}
#else
#define D3D12RHI_EXCEPT(hr) Renderer::D3D12RHI::HrException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::D3D12RHI::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_INFO_ONLY(call) (call)
#endif

// macro for importing infomanager into local scope
// this.GetInfoManager(Graphics& gfx) must exist
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; Renderer::DxgiInfoManager& infoManager = GetInfoManager((gfx))
#endif

#ifdef NDEBUG
#define INFOMAN_NOHR(gfx)
#else
#define INFOMAN_NOHR(gfx) DxgiInfoManager& infoManager = GetInfoManager((gfx))
#endif