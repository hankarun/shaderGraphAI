#ifndef MAT_H
#define MAT_H

namespace mat {
    void identity(float* m);
    void perspective(float* m, float fov, float aspect, float near, float far);
    void lookAt(float* m, float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ);
    void rotateY(float* m, float angle);
    void rotateX(float* m, float angle);
    void multiply(float* result, const float* a, const float* b);
}

#endif // MAT_H
