/*
 * Stereo Pipeline: stereo reprojection
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <opencv2/gpu/device/common.hpp>
#include <opencv2/gpu/device/vec_traits.hpp>

__constant__ float cq[16];
__constant__ ushort off_x, off_y;

__global__ void reproject_kernel (const cv::gpu::PtrStepSz<unsigned char> disparity, cv::gpu::PtrStepSz<float3> points)
{
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (y >= disparity.rows || x >= disparity.cols) {
        return;
    }

    const float qx = (x + off_x) * cq[ 0] + (y + off_y) * cq[ 1] + cq[ 3];
    const float qy = (x + off_x) * cq[ 4] + (y + off_y) * cq[ 5] + cq[ 7];
    const float qz = (x + off_x) * cq[ 8] + (y + off_y) * cq[ 9] + cq[11];
    const float qw = (x + off_x) * cq[12] + (y + off_y) * cq[13] + cq[15];

    const unsigned char d = disparity(y, x);

    const float iW = 1.f / (qw + cq[14] * d);

    float3 v = cv::gpu::device::VecTraits<float3>::all(1.0f);
    v.x = (qx + cq[2] * d) * iW;
    v.y = (qy + cq[6] * d) * iW;
    v.z = (qz + cq[10] * d) * iW;

    points(y, x) = v;
}


void reprojectDisparityImageGpu (const cv::gpu::PtrStepSz<unsigned char> disparity, cv::gpu::PtrStepSz<float3> points, const float *q, unsigned short offsetX, unsigned short offsetY)
{
    dim3 block(32, 8);
    dim3 grid(cv::gpu::divUp(disparity.cols, block.x), cv::gpu::divUp(disparity.rows, block.y));

    cudaSafeCall(cudaMemcpyToSymbol(cq, q, 16 * sizeof(float)));
    cudaSafeCall(cudaMemcpyToSymbol(off_x, &offsetX, sizeof(offsetX)));
    cudaSafeCall(cudaMemcpyToSymbol(off_y, &offsetY, sizeof(offsetY)));

    reproject_kernel<<<grid, block, 0>>>(disparity, points);
    cudaSafeCall(cudaGetLastError());
}
