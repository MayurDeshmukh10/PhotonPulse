#include <lightwave.hpp>

namespace lightwave {
    class DirectIntegrator : public SamplingIntegrator {
        bool remap; // remap the direction from [-1,+1]^3 to [0,+1]^3 so that colors channels are never negative   

    public:
        DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
            remap = properties.get<bool>("remap", true);
        }

        Color sampleNextEventLight(Intersection its, Sampler &rng) const {
            // Sample a light source
            LightSample light_sample = m_scene->sampleLight(rng);

            if (light_sample.light->canBeIntersected()) {
                return Color(0.0f, 0.0f, 0.0f);
            }

            // Compute the light's contribution
            DirectLightSample direct_light_sample = light_sample.light->sampleDirect(its.position, rng);
            
            // Check if the light is visible from the intersection point
            Ray shadow_ray = Ray(its.position, direct_light_sample.wi);
            Intersection shadow_its = m_scene->intersect(shadow_ray, rng);

            if (not shadow_its or shadow_its.t > direct_light_sample.distance) {
                // Compute the light's contribution
                return  direct_light_sample.weight * its.evaluateBsdf(direct_light_sample.wi).value / light_sample.probability;
            }

            return Color(0.0f, 0.0f, 0.0f);
        }
            
        Color Li(const Ray &ray, Sampler &rng) override {
            Color weight(1.0f, 1.0f, 1.0f);
            Color color = Color(0.0f, 0.0f, 0.0f);

            
            // Intersect the ray with a scene object
            Intersection its = m_scene->intersect(ray, rng);

            if (not its) {
                // If no intersection was found, return the background color
                return m_scene->evaluateBackground(ray.direction).value;
            }

            color = color + weight * its.evaluateEmission();

            //generate a new direction based on the BSDF and update the ray’s weight by multiplying it by the sampled BSDF weight
            if (not its.instance->bsdf()) {
                return color;
            }

            // // Light handling
            if (m_scene->hasLights()) {
                color = color + weight * sampleNextEventLight(its, rng);
            }

            // 1. sample a direction from the BSDF≤
            BsdfSample first_bsdf_sample = its.sampleBsdf(rng);
            weight *= first_bsdf_sample.weight;

            // remap the sample direction to global coordinates
            Ray secondary_ray = Ray(its.position, first_bsdf_sample.wi.normalized());
            its = m_scene->intersect(secondary_ray, rng);

            if (not its) {
                return color + weight * m_scene->evaluateBackground(secondary_ray.direction).value;
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
