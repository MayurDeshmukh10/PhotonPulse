#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {

        BsdfSample sample;
        Point2 samplePoint = rng.next2D();

        sample.wi = squareToCosineHemisphere(samplePoint).normalized();//squareToUniformHemisphere(samplePoint); squareToCosineHemisphere

        sample.weight = m_albedo->evaluate(uv); // * cosTheta / cosineHemispherePdf(sample.wi); // everything else simplifies to 1
        
        return sample;
    }

    std::string toString() const override {
        return tfm::format("Diffuse[\n"
                           "  albedo = %s\n"
                           "]",
                           indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
