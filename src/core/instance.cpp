#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {
    // hints:
    // * transform the hitpoint and frame here
    // * if m_flipNormal is true, flip the direction of the bitangent (which in effect flips the normal)
    // * make sure that the frame is orthonormal (you are free to change the bitangent for this, but keep
    //   the direction of the transformed tangent the same)
    
    surf.position = m_transform->apply(surf.position);
    surf.frame.tangent = m_transform->apply(surf.frame.tangent).normalized();
    surf.frame.bitangent = m_transform->apply(surf.frame.bitangent).normalized();

    surf.frame.normal = surf.frame.tangent.cross(surf.frame.bitangent).normalized();

    if (m_flipNormal) {
        surf.frame.bitangent = -surf.frame.bitangent;
    }
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {
    if (!m_transform) {
        // fast path, if no transform is needed
        Ray localRay = worldRay;
        if (m_shape->intersect(localRay, its, rng)) {
            its.instance = this;
            return true;
        }
        return false;
    }

    const float previousT = its.t;
    Ray localRay;

    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does *its.t need to change?
    // * how does its.position need to change?

    localRay = m_transform->inverse(worldRay);
    localRay.direction = localRay.direction.normalized();

    const Point local_intersection_point = m_transform->inverse(its.position);

    if (previousT != Infinity){
        its.t = Vector(local_intersection_point - localRay.origin).length();
    }

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);

    if (wasIntersected) {
        // hint: how does its.t need to change?

        const Point global_intersection_point = m_transform->apply(its.position);
        const double t_global = Vector(global_intersection_point - worldRay.origin).length();

        its.instance = this;

        its.t = t_global;
        this->transformFrame(its);

        return true;

    } else {
        its.t = previousT;
    }

    return false;
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")
