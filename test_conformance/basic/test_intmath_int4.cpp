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
#include "harness/compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "procs.h"

const char *int_add4_kernel_code =
"__kernel void test_int_add4(__global int4 *srcA, __global int4 *srcB, __global int4 *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = srcA[tid] + srcB[tid];\n"
"}\n";

const char *int_sub4_kernel_code =
"__kernel void test_int_sub4(__global int4 *srcA, __global int4 *srcB, __global int4 *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = srcA[tid] - srcB[tid];\n"
"}\n";

const char *int_mul4_kernel_code =
"__kernel void test_int_mul4(__global int4 *srcA, __global int4 *srcB, __global int4 *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = srcA[tid] * srcB[tid];\n"
"}\n";

const char *int_mad4_kernel_code =
"__kernel void test_int_mad4(__global int4 *srcA, __global int4 *srcB, __global int4 *srcC, __global int4 *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = srcA[tid] * srcB[tid] + srcC[tid];\n"
"}\n";

int
verify_int_add4(int *inptrA, int *inptrB, int *outptr, int n)
{
    int            r;
    int         i;

    for (i=0; i<n; i++)
    {
        r = inptrA[i] + inptrB[i];
        if (r != outptr[i])
        {
            log_error("INT_ADD int4 test failed\n");
            return -1;
        }
    }

    log_info("INT_ADD int4 test passed\n");
    return 0;
}

int
verify_int_sub4(int *inptrA, int *inptrB, int *outptr, int n)
{
    int            r;
    int         i;

    for (i=0; i<n; i++)
    {
        r = inptrA[i] - inptrB[i];
        if (r != outptr[i])
        {
            log_error("INT_SUB int4 test failed\n");
            return -1;
        }
    }

    log_info("INT_SUB int4 test passed\n");
    return 0;
}

int
verify_int_mul4(int *inptrA, int *inptrB, int *outptr, int n)
{
    int            r;
    int         i;

    for (i=0; i<n; i++)
    {
        r = inptrA[i] * inptrB[i];
        if (r != outptr[i])
        {
            log_error("INT_MUL int4 test failed\n");
            return -1;
        }
    }

    log_info("INT_MUL int4 test passed\n");
    return 0;
}

int
verify_int_mad4(int *inptrA, int *inptrB, int *inptrC, int *outptr, int n)
{
    int            r;
    int         i;

    for (i=0; i<n; i++)
    {
        r = inptrA[i] * inptrB[i] + inptrC[i];
        if (r != outptr[i])
        {
            log_error("INT_MAD int4 test failed\n");
            return -1;
        }
    }

    log_info("INT_MAD int4 test passed\n");
    return 0;
}

int
test_intmath_int4(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements)
{
    cl_mem streams[4];
    cl_program program[4];
    cl_kernel kernel[4];

    cl_int *input_ptr[3], *output_ptr, *p;
    size_t threads[1];
    int err, i;
    MTdata d = init_genrand( gRandomSeed );

    size_t length = sizeof(cl_int) * 4 * num_elements;

    input_ptr[0] = (cl_int*)malloc(length);
    input_ptr[1] = (cl_int*)malloc(length);
    input_ptr[2] = (cl_int*)malloc(length);
    output_ptr   = (cl_int*)malloc(length);

    streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[0])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }
    streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[1])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }
    streams[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[2])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }
    streams[3] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[3])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }

    p = input_ptr[0];
    for (i=0; i<num_elements*4; i++)
        p[i] = (int)genrand_int32(d);
    p = input_ptr[1];
    for (i=0; i<num_elements*4; i++)
        p[i] = (int)genrand_int32(d);
    p = input_ptr[2];
    for (i=0; i<num_elements*4; i++)
        p[i] = (int)genrand_int32(d);

    free_mtdata(d); d = NULL;

    err = clEnqueueWriteBuffer(queue, streams[0], CL_TRUE, 0, length, input_ptr[0], 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        log_error("clEnqueueWriteBuffer failed\n");
        return -1;
    }
    err = clEnqueueWriteBuffer(queue, streams[1], CL_TRUE, 0, length, input_ptr[1], 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        log_error("clEnqueueWriteBuffer failed\n");
        return -1;
    }
    err = clEnqueueWriteBuffer(queue, streams[2], CL_TRUE, 0, length, input_ptr[2], 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        log_error("clEnqueueWriteBuffer failed\n");
        return -1;
    }

    err = create_single_kernel_helper(context, &program[0], &kernel[0], 1, &int_add4_kernel_code, "test_int_add4");
    if (err != CL_SUCCESS)
    {
        log_error("create_single_kernel_helper failed\n");
        return -1;
    }

    err = create_single_kernel_helper(context, &program[1], &kernel[1], 1, &int_sub4_kernel_code, "test_int_sub4");
    if (err != CL_SUCCESS)
    {
        log_error("create_single_kernel_helper failed\n");
        return -1;
    }

    err = create_single_kernel_helper(context, &program[2], &kernel[2], 1, &int_mul4_kernel_code, "test_int_mul4");
    if (err != CL_SUCCESS)
    {
        log_error("create_single_kernel_helper failed\n");
        return -1;
    }

    err = create_single_kernel_helper(context, &program[3], &kernel[3], 1, &int_mad4_kernel_code, "test_int_mad4");
    if (err != CL_SUCCESS)
    {
        log_error("create_single_kernel_helper failed\n");
        return -1;
    }

  err  = clSetKernelArg(kernel[0], 0, sizeof streams[0], &streams[0]);
  err |= clSetKernelArg(kernel[0], 1, sizeof streams[1], &streams[1]);
  err |= clSetKernelArg(kernel[0], 2, sizeof streams[3], &streams[3]);
    if (err != CL_SUCCESS)
    {
        log_error("clSetKernelArgs failed\n");
        return -1;
    }

  err  = clSetKernelArg(kernel[1], 0, sizeof streams[0], &streams[0]);
  err |= clSetKernelArg(kernel[1], 1, sizeof streams[1], &streams[1]);
  err |= clSetKernelArg(kernel[1], 2, sizeof streams[3], &streams[3]);
    if (err != CL_SUCCESS)
    {
        log_error("clSetKernelArgs failed\n");
        return -1;
    }

  err  = clSetKernelArg(kernel[2], 0, sizeof streams[0], &streams[0]);
  err |= clSetKernelArg(kernel[2], 1, sizeof streams[1], &streams[1]);
  err |= clSetKernelArg(kernel[2], 2, sizeof streams[3], &streams[3]);
    if (err != CL_SUCCESS)
    {
        log_error("clSetKernelArgs failed\n");
        return -1;
    }

  err  = clSetKernelArg(kernel[3], 0, sizeof streams[0], &streams[0]);
  err |= clSetKernelArg(kernel[3], 1, sizeof streams[1], &streams[1]);
  err |= clSetKernelArg(kernel[3], 2, sizeof streams[2], &streams[2]);
  err |= clSetKernelArg(kernel[3], 3, sizeof streams[3], &streams[3]);
    if (err != CL_SUCCESS)
    {
        log_error("clSetKernelArgs failed\n");
        return -1;
    }

  threads[0] = (unsigned int)num_elements;
  for (i=0; i<4; i++)
  {
    err = clEnqueueNDRangeKernel(queue, kernel[i], 1, NULL, threads, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
      log_error("clEnqueueNDRangeKernel failed\n");
      return -1;
    }

    err = clEnqueueReadBuffer(queue, streams[3], CL_TRUE, 0, length, output_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
      log_error("clEnqueueReadBuffer failed\n");
      return -1;
    }

    switch (i)
    {
      case 0:
        err = verify_int_add4(input_ptr[0], input_ptr[1], output_ptr, num_elements);
        break;
      case 1:
        err = verify_int_sub4(input_ptr[0], input_ptr[1], output_ptr, num_elements);
        break;
      case 2:
        err = verify_int_mul4(input_ptr[0], input_ptr[1], output_ptr, num_elements);
        break;
      case 3:
        err = verify_int_mad4(input_ptr[0], input_ptr[1], input_ptr[2], output_ptr, num_elements);
        break;
    }
    if (err)
      break;
    }

    // cleanup
    clReleaseMemObject(streams[0]);
    clReleaseMemObject(streams[1]);
    clReleaseMemObject(streams[2]);
    clReleaseMemObject(streams[3]);
    for (i=0; i<4; i++)
    {
        clReleaseKernel(kernel[i]);
        clReleaseProgram(program[i]);
    }
    free(input_ptr[0]);
    free(input_ptr[1]);
    free(input_ptr[2]);
    free(output_ptr);
    return err;
}


