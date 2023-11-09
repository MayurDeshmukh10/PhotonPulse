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
            Vector d = ray.direction;
            
            // Intersect the ray with a scene object
            Intersection its = scene()->intersect(ray, rng);

            // if no intersection, return the background color
            if (!its) return scene()->evaluateBackground(d).value;
            
            d = its.wo;

            if (remap) d = (d + Vector(1)) / 2; 
            
            return Color(d);
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
