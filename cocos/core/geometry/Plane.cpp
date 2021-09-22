#include "cocos/core/geometry/Plane.h"

namespace cc {
namespace geometry {
Plane* Plane::create(float nx, float ny, float nz, float d) {
    return new Plane{nx, ny, nz, d};
}

Plane* Plane::clone(const Plane& p) {
    return new Plane{p.n.x, p.n.y, p.n.z, p.d};
}

Plane* Plane::copy(Plane* out, const Plane& p) {
    out->n = p.n;
    out->d = p.d;
    return out;
}

Plane* Plane::fromPoints(Plane*      out,
                         const Vec3& a,
                         const Vec3& b,
                         const Vec3& c) {
    Vec3::cross(b - a, c - a, &out->n);
    out->n.normalize();
    out->d = Vec3::dot(out->n, a);
    return out;
}

Plane* Plane::fromNormalAndPoint(Plane* out, const Vec3& normal, const Vec3& point) {
    out->n = normal;
    out->d = Vec3::dot(normal, point);
    return out;
}

Plane* Plane::normalize(Plane* out, const Plane& a) {
    const auto len = a.n.length();
    out->n         = a.n.getNormalized();
    if (len > 0) {
        out->d = a.d / len;
    }
    return out;
}

Plane* Plane::set(Plane* out, float nx, float ny, float nz, float d) {
    out->n = {nx, ny, nz};
    out->d = d;
    return out;
}

Plane::Plane(float nx, float ny, float nz, float d) {
    setType(ShapeEnum::SHAPE_PLANE);
    n       = {nx, ny, nz};
    this->d = d;
}

void Plane::transform(const Mat4& mat) {
    Mat4 tempMat = mat.getInversed();
    tempMat.transpose();
    Vec4 tempVec4 = {n.x, n.y, n.z, d};
    tempMat.transformVector(&tempVec4);
    n.set(tempVec4.x, tempVec4.y, tempVec4.z);
    d = tempVec4.w;
}

} // namespace geometry
} // namespace cc