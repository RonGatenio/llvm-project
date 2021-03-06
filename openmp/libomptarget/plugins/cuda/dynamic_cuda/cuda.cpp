//===--- cuda/dynamic_cuda/cuda.pp ------------------------------- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implement subset of cuda api by calling into cuda library via dlopen
// Does the dlopen/dlsym calls as part of the call to cuInit
//
//===----------------------------------------------------------------------===//

#include "cuda.h"
#include "Debug.h"
#include "dlwrap.h"

#include <dlfcn.h>

DLWRAP_INTERNAL(cuInit, 1);

DLWRAP(cuCtxGetDevice, 1);
DLWRAP(cuDeviceGet, 2);
DLWRAP(cuDeviceGetAttribute, 3);
DLWRAP(cuDeviceGetCount, 1);
DLWRAP(cuFuncGetAttribute, 3);

DLWRAP(cuGetErrorString, 2);
DLWRAP(cuLaunchKernel, 11);

DLWRAP(cuMemAlloc_v2, 2);
DLWRAP(cuMemcpyDtoDAsync_v2, 4);

DLWRAP(cuMemcpyDtoH_v2, 3);
DLWRAP(cuMemcpyDtoHAsync_v2, 4);
DLWRAP(cuMemcpyHtoD_v2, 3);
DLWRAP(cuMemcpyHtoDAsync_v2, 4);

DLWRAP(cuMemFree_v2, 1);
DLWRAP(cuModuleGetFunction, 3);
DLWRAP(cuModuleGetGlobal_v2, 4);

DLWRAP(cuModuleUnload, 1);
DLWRAP(cuStreamCreate, 2);
DLWRAP(cuStreamDestroy_v2, 1);
DLWRAP(cuStreamSynchronize, 1);
DLWRAP(cuCtxSetCurrent, 1);
DLWRAP(cuDevicePrimaryCtxRelease_v2, 1);
DLWRAP(cuDevicePrimaryCtxGetState, 3);
DLWRAP(cuDevicePrimaryCtxSetFlags_v2, 2);
DLWRAP(cuDevicePrimaryCtxRetain, 2);
DLWRAP(cuModuleLoadDataEx, 5);

DLWRAP(cuDeviceCanAccessPeer, 3);
DLWRAP(cuCtxEnablePeerAccess, 2);
DLWRAP(cuMemcpyPeerAsync, 6);

DLWRAP_FINALIZE();

#ifndef DYNAMIC_CUDA_PATH
#define DYNAMIC_CUDA_PATH "libcuda.so"
#endif

#define TARGET_NAME CUDA
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

static bool checkForCUDA() {
  // return true if dlopen succeeded and all functions found

  const char *CudaLib = DYNAMIC_CUDA_PATH;
  void *DynlibHandle = dlopen(CudaLib, RTLD_NOW);
  if (!DynlibHandle) {
    DP("Unable to load library '%s': %s!\n", CudaLib, dlerror());
    return false;
  }

  for (size_t I = 0; I < dlwrap::size(); I++) {
    const char *Sym = dlwrap::symbol(I);

    void *P = dlsym(DynlibHandle, Sym);
    if (P == nullptr) {
      DP("Unable to find '%s' in '%s'!\n", Sym, CudaLib);
      return false;
    }

    *dlwrap::pointer(I) = P;
  }

  return true;
}

CUresult cuInit(unsigned X) {
  // Note: Called exactly once from cuda rtl.cpp in a global constructor so
  // does not need to handle being called repeatedly or concurrently
  if (!checkForCUDA()) {
    return CUDA_ERROR_INVALID_VALUE;
  }
  return dlwrap_cuInit(X);
}
