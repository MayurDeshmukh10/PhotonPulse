#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
    Vector direction;
    Color intensity;

public:
    DirectionalLight(const Properties &properties) {
        direction = properties.get<Vector>("direction");
        direction = direction.normalized();
        
        intensity   = properties.get<Color>("intensity");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
                               
        return {
            .wi     = direction,
            .weight = intensity,
            .distance = Infinity,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("DirectionalLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
