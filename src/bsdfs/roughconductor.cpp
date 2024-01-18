#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        BsdfEval eval;
        Vector wh = (wi.normalized() + wo.normalized()).normalized();

        // 0.25 * R * D * G1(wi) * G1(wo) / |cosTheta(wo)| 
        eval.value = (0.25 *  m_reflectance->evaluate(uv) * microfacet::evaluateGGX(alpha, wh) * microfacet::smithG1(alpha, wh, wi) * microfacet::smithG1(alpha, wh, wo)) / Frame::absCosTheta(wo);

        return eval;

        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                    Sampler &rng) const override {
        BsdfSample sample;
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        Point2 samplePoint = rng.next2D();
        Vector sampledNormal = microfacet::sampleGGXVNDF(alpha, wo, samplePoint);
        sample.wi = reflect(wo, sampledNormal);
        Vector wh = (sample.wi.normalized() + wo.normalized()).normalized();
        sample.weight = m_reflectance->evaluate(uv) * microfacet::smithG1(alpha, wh, sample.wi);

        return sample;
    }

    Color albedo(const Point2 &uv, const Vector &wo, Sampler &rng) const override {
        return m_reflectance->evaluate(uv);
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
