#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        BsdfSample sample;
        Vector normal = { 0, 0, 1 };
        float ior = m_ior->scalar(uv);
        float cosThetaI = Frame::cosTheta(wo);
        bool entering = cosThetaI > 0.f;
        if(!entering) {
            ior = 1 / ior;
            normal = -normal;
        }
        float F = fresnelDielectric(cosThetaI, ior);

        float random_number = rng.next();

        if(random_number <= F) {
            Vector normal = { 0, 0, 1 };
            sample.wi = reflect(wo, Vector(0, 0, 1)).normalized();
            sample.weight = m_reflectance->evaluate(uv);
        } else {
            sample.wi = refract(wo, normal, ior).normalized();
            sample.weight = m_transmittance->evaluate(uv) / (ior * ior);
        }
        return sample;
    }

    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
