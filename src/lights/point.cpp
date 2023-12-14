#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
    Point position;
    Color intensity;

public:
    PointLight(const Properties &properties) {
        position            = properties.get<Point>("position");

        const Color power   = properties.get<Color>("power");
        assert(std::max(power[0], std::max(power[1], power[2])) != 0);
        assert(std::min(power[0], std::min(power[1], power[2])) >= 0);

        intensity = power / (4 * Pi);
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
                                    
        const Vector direction = (position - origin);
        const float distance = direction.length();

        const Vector direction_norm = direction.normalized();

        return {
            .wi     = direction_norm,
            .weight = intensity / (distance * distance),
            .distance = distance,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
