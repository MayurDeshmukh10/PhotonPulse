#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {
    class RoughDielectric : public Bsdf {
        ref<Texture> m_ior;
        ref<Texture> m_reflectance;
        ref<Texture> m_transmittance;
        ref<Texture> m_roughness;

        public:
            RoughDielectric(const Properties &properties) {
                m_ior = properties.get<Texture>("ior");
                m_reflectance = properties.get<Texture>("reflectance");
                m_transmittance = properties.get<Texture>("transmittance");
                m_roughness = properties.get<Texture>("roughness");
            }

            BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                             const Vector &wi) const override {
                return BsdfEval::invalid();
            }

            BsdfSample sample(const Point2 &uv, const Vector &wo, Sampler &rng) const override {
                BsdfSample sample;
                float ior = m_ior->scalar(uv);
                float cosThetaI = Frame::cosTheta(wo);

                bool entering = cosThetaI > 0.f;
                if(!entering) {    
                    ior = 1 / ior;
                }
                const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
                float F = fresnelDielectric(abs(cosThetaI), ior);
                float random_number = rng.next();
                Point2 samplePoint = rng.next2D();
                Vector sampledNormal = microfacet::sampleGGXVNDF(alpha, wo, samplePoint);

                if(random_number <= F) {
                    sample.wi = reflect(wo, sampledNormal);
                    if(!Frame::sameHemisphere(wo, sample.wi)) {
                        return sample.invalid();
                    }
                    Vector wh = (sample.wi.normalized() + wo.normalized()).normalized();
                    sample.weight = m_reflectance->evaluate(uv) * microfacet::smithG1(alpha, wh, sample.wi);
                    return sample;
                } else {
                    if(!entering) {
                        sampledNormal = -sampledNormal;
                    }
                    sample.wi = refract(wo, sampledNormal, ior);
                    Vector wh = (sample.wi.normalized() + wo.normalized()).normalized();
                    sample.weight = m_transmittance->evaluate(uv) * microfacet::smithG1(alpha, wh, sample.wi);
                    return sample;
                }

            }

            std::string toString() const override {
                return tfm::format("RoughDielectric[\n"
                                "  ior           = %s,\n"
                                "  reflectance   = %s,\n"
                                "  transmittance = %s,\n"
                                "  roughness = %s\n"
                                "]",
                                indent(m_ior), indent(m_reflectance),
                                indent(m_transmittance), indent(m_roughness));
            }
    };
}

REGISTER_BSDF(RoughDielectric, "roughdielectric")
