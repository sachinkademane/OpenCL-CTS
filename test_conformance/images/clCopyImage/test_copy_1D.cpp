//
// Copyright (c) 2017 The Khronos Group Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "../testBase.h"

#define MAX_ERR 0.005f
#define MAX_HALF_LINEAR_ERR 0.3f

extern bool            gDebugTrace, gDisableOffsets, gTestSmallImages, gEnablePitch, gTestMaxImages, gTestRounding, gTestMipmaps;
extern cl_filter_mode    gFilterModeToUse;
extern cl_addressing_mode    gAddressModeToUse;
extern uint64_t gRoundingStartValue;
extern cl_command_queue queue;
extern cl_context context;

extern int test_copy_image_generic( cl_device_id device, image_descriptor *srcImageInfo, image_descriptor *dstImageInfo,
                                   const size_t sourcePos[], const size_t destPos[], const size_t regionSize[], MTdata d );

int test_copy_image_size_1D( cl_device_id device, image_descriptor *imageInfo, MTdata d )
{
  size_t sourcePos[ 3 ], destPos[ 3 ], regionSize[ 3 ];
  int ret = 0, retCode;
    size_t src_lod = 0, src_width_lod = imageInfo->width, src_row_pitch_lod;
    size_t dst_lod = 0, dst_width_lod = imageInfo->width, dst_row_pitch_lod;
    size_t width_lod = imageInfo->width;
    size_t max_mip_level;

    if( gTestMipmaps )
    {
        max_mip_level = imageInfo->num_mip_levels;
        // Work at a random mip level
        src_lod = (size_t)random_in_range( 0, max_mip_level ? max_mip_level - 1 : 0, d );
        dst_lod = (size_t)random_in_range( 0, max_mip_level ? max_mip_level - 1 : 0, d );
        src_width_lod = ( imageInfo->width >> src_lod )? ( imageInfo->width >> src_lod ) : 1;
        dst_width_lod = ( imageInfo->width >> dst_lod )? ( imageInfo->width >> dst_lod ) : 1;
        width_lod  = ( src_width_lod > dst_width_lod ) ? dst_width_lod : src_width_lod;
        src_row_pitch_lod = src_width_lod * get_pixel_size( imageInfo->format );
        dst_row_pitch_lod = dst_width_lod * get_pixel_size( imageInfo->format );
    }

    // First, try just a full covering region
    sourcePos[ 0 ] = sourcePos[ 1 ] = sourcePos[ 2 ] = 0;
    destPos[ 0 ] = destPos[ 1 ] = destPos[ 2 ] = 0;
    regionSize[ 0 ] = imageInfo->width;
    regionSize[ 1 ] = 1;
    regionSize[ 2 ] = 1;

    if(gTestMipmaps)
    {
        sourcePos[ 1 ] = src_lod;
        destPos[ 1 ] = dst_lod;
        regionSize[ 0 ] = width_lod;
    }

    retCode = test_copy_image_generic( device, imageInfo, imageInfo, sourcePos, destPos, regionSize, d );
    if( retCode < 0 )
      return retCode;
    else
      ret += retCode;

    // Now try a sampling of different random regions
    for( int i = 0; i < 8; i++ )
    {
      if( gTestMipmaps )
      {
        // Work at a random mip level
        src_lod = (size_t)random_in_range( 0, max_mip_level ? max_mip_level - 1 : 0, d );
        dst_lod = (size_t)random_in_range( 0, max_mip_level ? max_mip_level - 1 : 0, d );
        src_width_lod = ( imageInfo->width >> src_lod )? ( imageInfo->width >> src_lod ) : 1;
        dst_width_lod = ( imageInfo->width >> dst_lod )? ( imageInfo->width >> dst_lod ) : 1;
        width_lod  = ( src_width_lod > dst_width_lod ) ? dst_width_lod : src_width_lod;
        sourcePos[ 1 ] = src_lod;
        destPos[ 1 ] = dst_lod;
      }
      // Pick a random size
      regionSize[ 0 ] = ( width_lod > 8 ) ? (size_t)random_in_range( 8, (int)width_lod - 1, d ) : width_lod;

      // Now pick positions within valid ranges
      sourcePos[ 0 ] = ( width_lod > regionSize[ 0 ] ) ? (size_t)random_in_range( 0, (int)( width_lod - regionSize[ 0 ] - 1 ), d ) : 0;
      destPos[ 0 ] = ( width_lod > regionSize[ 0 ] ) ? (size_t)random_in_range( 0, (int)( width_lod - regionSize[ 0 ] - 1 ), d ) : 0;


      // Go for it!
      retCode = test_copy_image_generic( device, imageInfo, imageInfo, sourcePos, destPos, regionSize, d );
      if( retCode < 0 )
        return retCode;
      else
        ret += retCode;
    }

      return ret;
}

int test_copy_image_set_1D( cl_device_id device, cl_image_format *format )
{
    size_t maxWidth;
    cl_ulong maxAllocSize, memSize;
    image_descriptor imageInfo = { 0 };
    RandomSeed seed(gRandomSeed);
    size_t pixelSize;

    imageInfo.format = format;
    imageInfo.height = imageInfo.depth = imageInfo.arraySize = imageInfo.slicePitch = 0;
    imageInfo.type = CL_MEM_OBJECT_IMAGE1D;
    pixelSize = get_pixel_size( imageInfo.format );

    int error = clGetDeviceInfo( device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof( maxWidth ), &maxWidth, NULL );
    error |= clGetDeviceInfo( device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof( maxAllocSize ), &maxAllocSize, NULL );
    error |= clGetDeviceInfo( device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof( memSize ), &memSize, NULL );
    test_error( error, "Unable to get max image 2D size from device" );

    if (memSize > (cl_ulong)SIZE_MAX) {
        memSize = (cl_ulong)SIZE_MAX;
    }

    if( gTestSmallImages )
    {
        for( imageInfo.width = 1; imageInfo.width < 13; imageInfo.width++ )
        {
      size_t rowPadding = gEnablePitch ? 48 : 0;
            imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;

            if (gTestMipmaps)
                imageInfo.num_mip_levels = (cl_uint) random_log_in_range(2, (int)compute_max_mip_levels(imageInfo.width, 0, 0), seed);

            if (gEnablePitch)
            {
                do {
                    rowPadding++;
                    imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;
                } while ((imageInfo.rowPitch % pixelSize) != 0);
            }

            if( gDebugTrace )
                log_info( "   at size %d\n", (int)imageInfo.width );

            int ret = test_copy_image_size_1D( device, &imageInfo, seed );
            if( ret )
                return -1;
        }
    }
    else if( gTestMaxImages )
    {
        // Try a specific set of maximum sizes
        size_t numbeOfSizes;
        size_t sizes[100][3];

        get_max_sizes(&numbeOfSizes, 100, sizes, maxWidth, 1, 1, 1, maxAllocSize, memSize, CL_MEM_OBJECT_IMAGE1D, imageInfo.format);

        for( size_t idx = 0; idx < numbeOfSizes; idx++ )
        {
      size_t rowPadding = gEnablePitch ? 48 : 0;
            imageInfo.width = sizes[ idx ][ 0 ];
            imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;

            if (gTestMipmaps)
                imageInfo.num_mip_levels = (cl_uint) random_log_in_range(2, (int)compute_max_mip_levels(imageInfo.width, 0, 0), seed);

            if (gEnablePitch)
            {
                do {
                    rowPadding++;
                    imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;
                } while ((imageInfo.rowPitch % pixelSize) != 0);
            }

            log_info( "Testing %d\n", (int)sizes[ idx ][ 0 ] );
            if( gDebugTrace )
                log_info( "   at max size %d\n", (int)sizes[ idx ][ 0 ] );
            if( test_copy_image_size_1D( device, &imageInfo, seed ) )
                return -1;
        }
    }
    else
    {
        for( int i = 0; i < NUM_IMAGE_ITERATIONS; i++ )
        {
            cl_ulong size;
      size_t rowPadding = gEnablePitch ? 48 : 0;
            // Loop until we get a size that a) will fit in the max alloc size and b) that an allocation of that
            // image, the result array, plus offset arrays, will fit in the global ram space
            do
            {
                imageInfo.width = (size_t)random_log_in_range( 16, (int)maxWidth / 32, seed );

        if (gTestMipmaps)
        {
          imageInfo.num_mip_levels = (cl_uint) random_log_in_range(2, (int)compute_max_mip_levels(imageInfo.width, 0, 0), seed);
          imageInfo.rowPitch = imageInfo.width * get_pixel_size( imageInfo.format );
          size = compute_mipmapped_image_size( imageInfo );
          size = size*4;
        }
        else
        {
          imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;

          if (gEnablePitch)
          {
            do {
              rowPadding++;
              imageInfo.rowPitch = imageInfo.width * pixelSize + rowPadding;
            } while ((imageInfo.rowPitch % pixelSize) != 0);
          }

          size = (size_t)imageInfo.rowPitch * 4;
        }
            } while(  size > maxAllocSize || ( size * 3 ) > memSize );

            if( gDebugTrace )
      {
        log_info( "   at size %d (row pitch %d) out of %d\n", (int)imageInfo.width, (int)imageInfo.rowPitch, (int)maxWidth );
        if ( gTestMipmaps )
          log_info( "   and %llu mip levels\n", (size_t) imageInfo.num_mip_levels );
      }

            int ret = test_copy_image_size_1D( device, &imageInfo, seed );
            if( ret )
                return -1;
        }
    }

    return 0;
}