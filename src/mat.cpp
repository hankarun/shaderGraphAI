#include "mat.h"
#include <cmath>
#include <cstring>

namespace mat {
    void identity(float* m) {
        memset(m, 0, 16 * sizeof(float));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    void perspective(float* m, float fov, float aspect, float near, float far) {
        float tanHalfFov = tan(fov / 2.0f);
        memset(m, 0, 16 * sizeof(float));
        m[0] = 1.0f / (aspect * tanHalfFov);
        m[5] = 1.0f / tanHalfFov;
        m[10] = -(far + near) / (far - near);
        m[11] = -1.0f;
        m[14] = -(2.0f * far * near) / (far - near);
    }
    
    void lookAt(float* m, float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ) {
        float fx = centerX - eyeX, fy = centerY - eyeY, fz = centerZ - eyeZ;
        float len = sqrt(fx*fx + fy*fy + fz*fz);
        fx /= len; fy /= len; fz /= len;
        
        float sx = fy * upZ - fz * upY, sy = fz * upX - fx * upZ, sz = fx * upY - fy * upX;
        len = sqrt(sx*sx + sy*sy + sz*sz);
        sx /= len; sy /= len; sz /= len;
        
        float ux = sy * fz - sz * fy, uy = sz * fx - sx * fz, uz = sx * fy - sy * fx;
        
        m[0] = sx;  m[4] = sy;  m[8]  = sz;  m[12] = -(sx*eyeX + sy*eyeY + sz*eyeZ);
        m[1] = ux;  m[5] = uy;  m[9]  = uz;  m[13] = -(ux*eyeX + uy*eyeY + uz*eyeZ);
        m[2] = -fx; m[6] = -fy; m[10] = -fz; m[14] = fx*eyeX + fy*eyeY + fz*eyeZ;
        m[3] = 0;   m[7] = 0;   m[11] = 0;   m[15] = 1.0f;
    }
    
    void rotateY(float* m, float angle) {
        identity(m);
        float c = cos(angle), s = sin(angle);
        m[0] = c;  m[8] = s;
        m[2] = -s; m[10] = c;
    }
    
    void rotateX(float* m, float angle) {
        identity(m);
        float c = cos(angle), s = sin(angle);
        m[5] = c;  m[9] = -s;
        m[6] = s;  m[10] = c;
    }
    
    void multiply(float* result, const float* a, const float* b) {
        float temp[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i * 4 + j] = 0;
                for (int k = 0; k < 4; k++) {
                    temp[i * 4 + j] += a[k * 4 + j] * b[i * 4 + k];
                }
            }
        }
        memcpy(result, temp, 16 * sizeof(float));
    }
}
