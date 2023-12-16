#include <lightwave.hpp>

namespace lightwave {
    class PathtracerIntegrator : public SamplingIntegrator {
        bool remap; // remap the direction from [-1,+1]^3 to [0,+1]^3 so that colors channels are never negative   
        int depth;

    public:
        PathtracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
            remap = properties.get<bool>("remap", true);
            depth = properties.get<int>("depth", 2);
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

            if (shadow_its and shadow_its.t < direct_light_sample.distance) {
                return Color(0.0f, 0.0f, 0.0f);
            }

            return  direct_light_sample.weight * its.evaluateBsdf(direct_light_sample.wi).value / light_sample.probability;
        }
            
        Color Li(const Ray &ray, Sampler &rng) override {
            Color weight(1.0f, 1.0f, 1.0f);
            Color color = Color(0.0f, 0.0f, 0.0f);

            Ray current_ray = ray;
            BsdfSample bsdf_sample;


            int bounces = 0;
            while (true) {
                // Intersect the ray with a scene object
                Intersection its = m_scene->intersect(current_ray, rng);

                if (not its) {
                    // If no intersection was found, return the background color
                    return color + weight * m_scene->evaluateBackground(ray.direction).value;
                }

                color += weight * its.evaluateEmission();

                if (not its.instance->bsdf()) {
                    return color;
                }   

                if (bounces >= depth - 1) {
                    return color;
                }

                // Light handling
                if (m_scene->hasLights()) {
                    color += weight * sampleNextEventLight(its, rng);
                }

                bsdf_sample = its.sampleBsdf(rng); // Sample a direction from the BSDF
                weight *= bsdf_sample.weight; 

                current_ray = Ray(its.position, bsdf_sample.wi.normalized());
                bounces++;
            }

            return color;
        }

        std::string toString() const override {
            return tfm::format(
                "PathtracerIntegrator[\n"
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
REGISTER_INTEGRATOR(PathtracerIntegrator, "pathtracer")
