#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {

        if (wo.z() * wi.z() <= 0) {
            return {
                .value = Color(0),
            };
        }

        return { 
            .value = m_albedo->evaluate(uv) * abs(Frame::cosTheta(wi)) / Pi, 
        };
        
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {

        BsdfSample sample;
        sample.weight = m_albedo->evaluate(uv); // * cosTheta / cosineHemispherePdf(sample.wi); // everything else simplifies to 1

        Point2 samplePoint = rng.next2D();

        sample.wi = squareToCosineHemisphere(samplePoint).normalized();

        if (sample.wi.z() * wo.z() < 0) {
            sample.wi[2] = sample.wi[2] * -1;
        }
        
        return sample;
    }

    Color albedo(const Point2 &uv) const override {

        return m_albedo->evaluate(uv);
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
