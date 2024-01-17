#include <lightwave.hpp>

namespace lightwave {

    class AlbedoIntegrator : public SamplingIntegrator {

    public:
        AlbedoIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {}
            
        Color Li(const Ray &ray, Sampler &rng) override {
            // Intersect the ray with a scene object
            Intersection its = scene()->intersect(ray, rng);

            if (its) {
                return its.evaluateAlbedo(rng);
            }
            
            return Color(0);
        }

        std::string toString() const override {
            return tfm::format(
                "AlbedoIntegrator[]"
            );
        }
    };
}

// this informs lightwave to use our class NormalsIntegrator whenever a <integrator type="normals" /> is found in a scene file
REGISTER_INTEGRATOR(AlbedoIntegrator, "albedo")
