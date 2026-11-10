// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// cuda_probe.cu
// Stage 0 of the CUDA port (ADR-0013). This translation unit is compiled
// only when BHDS_ENABLE_CUDA=ON and a CUDA compiler was found. It does not
// shade pixels. The lensed image stays on the CPU (renderer.hpp) so a
// machine without a GPU, including CI, never needs this file.
//
// The next stage replaces render_lensed_disk with a one-thread-per-pixel
// kernel of the same Schwarzschild orbit equation, and checks the image
// against the CPU tracer.

#include <cuda_runtime.h>

extern "C" int bhds_cuda_device_count() {
    int n = 0;
    const cudaError_t err = cudaGetDeviceCount(&n);
    if (err != cudaSuccess) {
        return 0;
    }
    return n;
}
