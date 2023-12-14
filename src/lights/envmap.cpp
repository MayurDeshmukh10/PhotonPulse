#include <lightwave.hpp>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    ref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

public:
    EnvironmentMap(const Properties &properties) {
        m_texture   = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
    }

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector local_direction = direction;

        if (m_transform) {
            local_direction = m_transform->inverse(direction).normalized();
        }

        local_direction = Vector(local_direction.x(), local_direction.y(), -local_direction.z());

        float theta = std::atan2(local_direction.z(), local_direction.x());
        float tex_x = (theta + Pi)/(2*Pi);//phi / (2 * Pi);

        float phi = std::acos(local_direction.y());
        if (phi < 0) {
            phi += 2 * Pi;
        }
        float tex_y = phi / Pi;

        return {
            .value = m_texture->evaluate(Vector2(tex_x, tex_y)),
        };

        // hints:
        // * if (m_transform) { transform direction vector from world to local
        // coordinates }
        // * find the corresponding pixel coordinate for the given local
        // direction
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = squareToUniformSphere(rng.next2D());
        auto E           = evaluate(direction);

        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)

        return {
            .wi     = direction,
            .weight = E.value / Inv4Pi,
            .distance = Infinity,
        };
    }

    std::string toString() const override {
        return tfm::format("EnvironmentMap[\n"
                           "  texture = %s,\n"
                           "  transform = %s\n"
                           "]",
                           indent(m_texture), indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
