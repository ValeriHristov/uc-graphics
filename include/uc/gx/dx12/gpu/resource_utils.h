#pragma once

#include <d3d12.h>
#include <cstdint>
#include <wrl/client.h>

#include <uc/mem/alloc.h>
#include <uc/sys/memcpy.h>

//#include "helpers.h"

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
			//------------------------------------------------------------------------------------------------
			struct CD3DX12_TEXTURE_COPY_LOCATION : public D3D12_TEXTURE_COPY_LOCATION
			{
				CD3DX12_TEXTURE_COPY_LOCATION() = default;
				explicit CD3DX12_TEXTURE_COPY_LOCATION(const D3D12_TEXTURE_COPY_LOCATION& o) :
					D3D12_TEXTURE_COPY_LOCATION(o)
				{}
				CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes) { pResource = pRes; }
				CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, D3D12_PLACED_SUBRESOURCE_FOOTPRINT const& Footprint)
				{
					pResource = pRes;
					Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					PlacedFootprint = Footprint;
				}
				CD3DX12_TEXTURE_COPY_LOCATION(ID3D12Resource* pRes, UINT Sub)
				{
					pResource = pRes;
					Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					SubresourceIndex = Sub;
				}
			};

			//------------------------------------------------------------------------------------------------
			// Row-by-row memcpy
			inline void MemcpySubresource(
				_In_ const D3D12_MEMCPY_DEST* pDest,
				_In_ const D3D12_SUBRESOURCE_DATA* pSrc,
				SIZE_T RowSizeInBytes,
				UINT NumRows,
				UINT NumSlices)
			{
				for (UINT z = 0; z < NumSlices; ++z)
				{
					BYTE* pDestSlice = reinterpret_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
					const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * z;
					for (UINT y = 0; y < NumRows; ++y)
					{
						memcpy(pDestSlice + pDest->RowPitch * y, pSrcSlice + pSrc->RowPitch * y, RowSizeInBytes);
					}
				}
			}

			//------------------------------------------------------------------------------------------------
			// Returns required size of a buffer to be used for data upload
			inline UINT64 GetRequiredIntermediateSize(
				_In_ ID3D12Resource* pDestinationResource,
				_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
				_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources)
			{
				D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
				UINT64 RequiredSize = 0;

				ID3D12Device* pDevice;
				pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
				pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, nullptr, nullptr, nullptr, &RequiredSize);
				pDevice->Release();

				return RequiredSize;
			}

			//------------------------------------------------------------------------------------------------
			// All arrays must be populated (e.g. by calling GetCopyableFootprints)
			inline UINT64 UpdateSubresources(
				_In_ ID3D12GraphicsCommandList* pCmdList,
				_In_ ID3D12Resource* pDestinationResource,
				_In_ ID3D12Resource* pIntermediate,
				_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
				_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
				UINT64 RequiredSize,
				_In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
				_In_reads_(NumSubresources) const UINT* pNumRows,
				_In_reads_(NumSubresources) const UINT64* pRowSizesInBytes,
				_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData)
			{
				// Minor validation
				D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
				D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc();
				if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
					IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
					RequiredSize >(SIZE_T) - 1 ||
					(DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
					(FirstSubresource != 0 || NumSubresources != 1)))
				{
					return 0;
				}

				BYTE* pData;
				HRESULT hr = pIntermediate->Map(0, NULL, reinterpret_cast<void**>(&pData));
				if (FAILED(hr))
				{
					return 0;
				}

				for (UINT i = 0; i < NumSubresources; ++i)
				{
					if (pRowSizesInBytes[i] > (SIZE_T)-1) return 0;
					D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, SIZE_T(pLayouts[i].Footprint.RowPitch) * SIZE_T(pNumRows[i]) };
					MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
				}
				pIntermediate->Unmap(0, NULL);

				if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
				{
					pCmdList->CopyBufferRegion(
						pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
				}
				else
				{
					for (UINT i = 0; i < NumSubresources; ++i)
					{
						CD3DX12_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
						CD3DX12_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
						pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
					}
				}
				return RequiredSize;
			}

			//------------------------------------------------------------------------------------------------
			// Heap-allocating UpdateSubresources implementation
			inline UINT64 UpdateSubresources(
				_In_ ID3D12GraphicsCommandList* pCmdList,
				_In_ ID3D12Resource* pDestinationResource,
				_In_ ID3D12Resource* pIntermediate,
				UINT64 IntermediateOffset,
				_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
				_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
				_In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData)
			{
				UINT64 RequiredSize = 0;
				UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * NumSubresources;
				if (MemToAlloc > SIZE_MAX)
				{
					return 0;
				}
				void* pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
				if (pMem == NULL)
				{
					return 0;
				}
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pMem);
				UINT64* pRowSizesInBytes = reinterpret_cast<UINT64*>(pLayouts + NumSubresources);
				UINT* pNumRows = reinterpret_cast<UINT*>(pRowSizesInBytes + NumSubresources);

				D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
				ID3D12Device* pDevice;
				pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
				pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
				pDevice->Release();

				UINT64 Result = UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
				HeapFree(GetProcessHeap(), 0, pMem);
				return Result;
			}

			//------------------------------------------------------------------------------------------------
			// Stack-allocating UpdateSubresources implementation
			template <UINT MaxSubresources>
			inline UINT64 UpdateSubresources(
				_In_ ID3D12GraphicsCommandList* pCmdList,
				_In_ ID3D12Resource* pDestinationResource,
				_In_ ID3D12Resource* pIntermediate,
				UINT64 IntermediateOffset,
				_In_range_(0, MaxSubresources) UINT FirstSubresource,
				_In_range_(1, MaxSubresources - FirstSubresource) UINT NumSubresources,
				_In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData)
			{
				UINT64 RequiredSize = 0;
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[MaxSubresources];
				UINT NumRows[MaxSubresources];
				UINT64 RowSizesInBytes[MaxSubresources];

				D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
				ID3D12Device* pDevice;
				pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
				pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
				pDevice->Release();

				return UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, Layouts, NumRows, RowSizesInBytes, pSrcData);
			}

			/*
            //copies subresources row by row
            inline void memcpy_subresource(const D3D12_MEMCPY_DEST* __restrict destination, const D3D12_SUBRESOURCE_DATA* __restrict source, uint64_t row_size_in_bytes, uint32_t row_count, uint32_t slice_count)
            {
                auto source_address      = reinterpret_cast<uintptr_t> (source->pData);
                auto destination_address = reinterpret_cast<uintptr_t> (destination->pData);

                for (auto z = 0U; z < slice_count; ++z)
                {
                    auto destination_slice = destination_address + destination->SlicePitch * z;
                    auto source_slice      = source_address + source->SlicePitch * z;

                    for (auto y = 0U; y < row_count; ++y)
                    {
                        auto destination_row = destination_slice + destination->RowPitch * y;
                        auto source_row      = source_slice + source->RowPitch * y;

                        sys::memcpy(reinterpret_cast<void*>(destination_row), reinterpret_cast<const void*>(source_row), static_cast<size_t>(row_size_in_bytes));
                    }
                }
            }

            //------------------------------------------------------------------------------------------------
            // Returns required size of a buffer to be used for data upload
            inline uint64_t resource_size( ID3D12Resource* resource, _In_range_(0, D3D12_REQ_SUBRESOURCES) uint32_t first_sub_resource, _In_range_(0, D3D12_REQ_SUBRESOURCES - first_sub_resource) uint32_t sub_resource_count)
            {
                D3D12_RESOURCE_DESC desc = resource->GetDesc();
                uint64_t r = 0;

                Microsoft::WRL::ComPtr<ID3D12Device> d;

                throw_if_failed(resource->GetDevice(IID_PPV_ARGS(&d)));
                d->GetCopyableFootprints(&desc, first_sub_resource, sub_resource_count, 0, nullptr, nullptr, nullptr, &r);

                return r;
            }

            //------------------------------------------------------------------------------------------------
            // All arrays must be populated (e.g. by calling GetCopyableFootprints)
            inline UINT64 UpdateSubresources(
                _In_ ID3D12GraphicsCommandList* __restrict      cmd_list,
                _In_ ID3D12Resource* __restrict                 destination,
                _In_ ID3D12Resource* __restrict                 intermediate,
                _In_range_(0, D3D12_REQ_SUBRESOURCES) uint32_t  first_sub_resource,
                _In_range_(0, D3D12_REQ_SUBRESOURCES - first_sub_resource) uint32_t sub_resource_count,
                UINT64 required_size,
                _In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* __restrict layouts,
                _In_reads_(NumSubresources) const uint32_t* __restrict row_count,
                _In_reads_(NumSubresources) const uint64_t* __restrict row_sizes_in_bytes,
                _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* __restrict source)
            {
                // Minor validation
                D3D12_RESOURCE_DESC desc0 = intermediate->GetDesc();
                D3D12_RESOURCE_DESC desc1 = destination->GetDesc();

                if ( desc0.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || desc0.Width < required_size + layouts[0].Offset || required_size >(SIZE_T) - 1 ||
                    (desc1.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&  (first_sub_resource != 0 || sub_resource_count != 1)))
                {
                    throw_if_failed(E_FAIL);
                }

                uint8_t* pData;
                throw_if_failed( intermediate->Map(0, NULL, reinterpret_cast<void**>(&pData)) );

                //1. copy row by row, subresource by subresource to the upload data
                for (UINT i = 0; i < sub_resource_count; ++i)
                {
                    if ( row_sizes_in_bytes[i] >(SIZE_T)-1 )
                    {
                        intermediate->Unmap(0, NULL);
                        throw_if_failed(E_FAIL);
                    }

                    D3D12_MEMCPY_DEST DestData = { pData + layouts[i].Offset, layouts[i].Footprint.RowPitch, layouts[i].Footprint.RowPitch * row_count[i] };
                    
                    memcpy_subresource(&DestData, &source[i], (SIZE_T) row_sizes_in_bytes[i], row_count[i], layouts[i].Footprint.Depth);
                }

                intermediate->Unmap(0, NULL);

                //copy form the upload heap to the gpu
                if (desc1.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
                {
                    D3D12_BOX SrcBox = { UINT(layouts[0].Offset), UINT(layouts[0].Offset + layouts[0].Footprint.Width) };
                    cmd_list->CopyBufferRegion( destination, 0, intermediate, layouts[0].Offset, layouts[0].Footprint.Width);
                }
                else
                {
                    for (UINT i = 0; i < sub_resource_count; ++i)
                    {
                        D3D12_TEXTURE_COPY_LOCATION Dst = { destination,   D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, i + first_sub_resource };
                        D3D12_TEXTURE_COPY_LOCATION Src = { intermediate,  D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,  layouts[i] };
                        cmd_list->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
                    }
                }

                return required_size;
            }


            //------------------------------------------------------------------------------------------------
            // Stack-allocating UpdateSubresources implementation
            template <UINT max_sub_resources>
            inline UINT64 UpdateSubresources(
                _In_ ID3D12GraphicsCommandList* __restrict cmd_list,
                _In_ ID3D12Resource* __restrict destination,
                _In_ ID3D12Resource* __restrict intermediate,
                uint64_t intermediate_offset,
                _In_range_(0, max_sub_resources) uint32_t first_sub_resource,
                _In_range_(1, max_sub_resources - first_sub_resource) uint32_t sub_resource_count,
                _In_reads_(sub_resource_count) D3D12_SUBRESOURCE_DATA*__restrict source )
            {
                uint64_t                           required_size = 0;
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[max_sub_resources];
                uint32_t                           row_count[max_sub_resources];
                uint64_t                           row_sizes_in_bytes[max_sub_resources];

                {
                    D3D12_RESOURCE_DESC desc = destination->GetDesc();
                    Microsoft::WRL::ComPtr<ID3D12Device> d;
                    throw_if_failed(destination->GetDevice(IID_PPV_ARGS(&d)));
                    d->GetCopyableFootprints(&desc, first_sub_resource, sub_resource_count, intermediate_offset, layouts, row_count, row_sizes_in_bytes, &required_size);
                }

                return UpdateSubresources(cmd_list, destination, intermediate, first_sub_resource, sub_resource_count, required_size, layouts, row_count, row_sizes_in_bytes, source);
            }

            //------------------------------------------------------------------------------------------------
            // Stack-allocating UpdateSubresources implementation
            template <UINT max_sub_resources>
            inline UINT64 UpdateSubresources(
                _In_ ID3D12GraphicsCommandList* __restrict cmd_list,
                _In_ ID3D12Resource* __restrict destination,
                _In_ ID3D12Resource* __restrict intermediate,
                _In_range_(0, max_sub_resources) uint32_t first_sub_resource,
                _In_range_(1, max_sub_resources - first_sub_resource) uint32_t sub_resource_count,
                _In_reads_(sub_resource_count) D3D12_SUBRESOURCE_DATA*__restrict source)
            {
                return UpdateSubresources<max_sub_resources>(cmd_list, destination, intermediate, 0, first_sub_resource, sub_resource_count, source);
            }
			*/
        }
    }
}
