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
    
    // apply normal mapping
    if(m_normal) {
        Color normal_mapping = m_normal->evaluate(surf.uv);

        // converting from [0,1]^3 to [-1,1]^3
        Color M = Color(2 * normal_mapping.r() - 1, 2 * normal_mapping.g() - 1, 2 * normal_mapping.b() - 1);

        // (Mr * tangent + Mg * bitangent + 5 * Mb * normal)
        Vector mappedNormal = (M.r() * surf.frame.tangent + M.g() * surf.frame.bitangent +  M.b() * surf.frame.normal).normalized();
        surf.frame = Frame(m_transform->applyNormal(mappedNormal).normalized());
    } else {
        surf.frame.tangent = m_transform->apply(surf.frame.tangent).normalized();
        surf.frame.bitangent = m_transform->apply(surf.frame.bitangent).normalized();
        surf.frame.normal = surf.frame.tangent.cross(surf.frame.bitangent).normalized();
    }
    
    surf.position = m_transform->apply(surf.position);

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
        } else {
            return false;
        }
    }

    const float previousT = its.t;
    Ray localRay;

    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does *its.t need to change?
    // * how does its.position need to change?

    localRay = m_transform->inverse(worldRay);
    const float local_ray_scale = localRay.direction.length();
    localRay.direction = localRay.direction.normalized();

    its.t *= local_ray_scale;

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);

    if (wasIntersected) {
        its.instance = this;

        its.t /= local_ray_scale;
        this->transformFrame(its);

        return true;

    } else {
        its.t = previousT;
        return false;
    }
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
