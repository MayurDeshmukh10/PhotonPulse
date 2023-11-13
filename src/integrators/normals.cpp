#include <lightwave.hpp>

namespace lightwave {

    class NormalsIntegrator : public SamplingIntegrator {
        bool remap; // remap the direction from [-1,+1]^3 to [0,+1]^3 so that colors channels are never negative   

    public:
        NormalsIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
            remap = properties.get<bool>("remap", true);
        }
            
        Color Li(const Ray &ray, Sampler &rng) override {
            
            // Intersect the ray with a scene object
            Intersection its = scene()->intersect(ray, rng);

            Vector normal_vector = its ? its.wo : Vector(0.0, 0.0, 0.0);

            if (remap) normal_vector = (normal_vector + Vector(1)) / 2; 
            
            return Color(normal_vector);
        }

        std::string toString() const override {
            return tfm::format(
                "NormalsIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image)
            );
        }
    };
}

// this informs lightwave to use our class NormalsIntegrator whenever a <integrator type="normals" /> is found in a scene file
REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
