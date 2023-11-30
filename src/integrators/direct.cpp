#include <lightwave.hpp>

namespace lightwave {

    class DirectIntegrator : public SamplingIntegrator {
        bool remap; // remap the direction from [-1,+1]^3 to [0,+1]^3 so that colors channels are never negative   

    public:
        DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
            remap = properties.get<bool>("remap", true);
        }
            
        Color Li(const Ray &ray, Sampler &rng) override {
            Color weight(1.0f, 1.0f, 1.0f);
            Color color = Color(0.0f, 0.0f, 0.0f);

            
            // Intersect the ray with a scene object
            Intersection its = scene()->intersect(ray, rng);

            if (not its) {
                // If no intersection was found, return the background color
                return scene()->evaluateBackground(ray.direction).value;
            }

            color = color + weight * its.evaluateEmission();

            //generate a new direction based on the BSDF and update the ray’s weight by multiplying it by the sampled BSDF weight
            if (not its.instance->bsdf()) {
                return color;
            }

            // 1. sample a direction from the BSDF
            BsdfSample first_bsdf_sample = its.instance->bsdf()->sample(its.uv, its.wo, rng);
            weight *= first_bsdf_sample.weight;

            // remap the sample direction to global coordinates
            Ray secondary_ray = Ray(its.position, its.frame.toWorld(first_bsdf_sample.wi).normalized());
            its = scene()->intersect(secondary_ray, rng);

            if (not its) {
                return color + weight * scene()->evaluateBackground(secondary_ray.direction).value;
            }
            
            return color + weight * its.evaluateEmission();
        }

        std::string toString() const override {
            return tfm::format(
                "DirectIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image)
            );
        }
    };
}

// this informs lightwave to use our class DirectIntegrator whenever a <integrator type="normals" /> is found in a scene file
REGISTER_INTEGRATOR(DirectIntegrator, "direct")
