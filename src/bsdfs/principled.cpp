#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"
#include <stdexcept>

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        BsdfEval eval;

        eval.value = color * abs(Frame::cosTheta(wi)) / Pi;

        return eval;

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        BsdfSample sample;
        Point2 samplePoint = rng.next2D();

        sample.wi = squareToCosineHemisphere(samplePoint).normalized();

        sample.weight = color;
        
        return sample;

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        BsdfEval eval;
        Vector wh = (wi.normalized() + wo.normalized()).normalized();
        eval.value = (0.25 * color * microfacet::evaluateGGX(alpha, wh) * microfacet::smithG1(alpha, wh, wi) * microfacet::smithG1(alpha, wh, wo)) / Frame::absCosTheta(wo);

        return eval;

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        BsdfSample sample;
        Point2 samplePoint = rng.next2D();
         
        Vector sampledNormal = microfacet::sampleGGXVNDF(alpha, wo, samplePoint);
        sample.wi = reflect(wo, sampledNormal);
        Vector wh = (sample.wi.normalized() + wo.normalized()).normalized();
        sample.weight = color * microfacet::smithG1(alpha, wh, sample.wi);

        return sample;

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto combination = combine(uv, wo);

        BsdfEval eval;

        eval.value = combination.diffuse.evaluate(wo, wi).value + combination.metallic.evaluate(wo, wi).value;

        return eval;

        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        BsdfSample sample;
        const auto combination = combine(uv, wo);
        float diffuseSelectionProb = combination.diffuseSelectionProb;
        float roughConductorSelectionProb = 1 - diffuseSelectionProb;
        float random_number = rng.next();

        if(random_number < diffuseSelectionProb) {
            sample = combination.diffuse.sample(wo, rng);
            sample.weight = sample.weight / diffuseSelectionProb;
        } else {
            sample = combination.metallic.sample(wo, rng);
            sample.weight = sample.weight / roughConductorSelectionProb;
        }

        return sample;

        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`
    }

    Color albedo(const Point2 &uv, const Vector &wo, Sampler &rng) const override {
        return m_baseColor->evaluate(uv);
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
