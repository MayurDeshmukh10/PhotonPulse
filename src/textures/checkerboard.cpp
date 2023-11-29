#include <lightwave.hpp>

namespace lightwave {

class CheckerboardTexture : public Texture {
    Color m_color_0, m_color_1;
    Vector2 m_scale;

public:
    CheckerboardTexture(const Properties &properties) {
        m_color_0 = properties.get<Color>("color0");
        m_color_1 = properties.get<Color>("color1");
        m_scale = properties.get<Vector2>("scale");
    }


    bool isOddTile(float x) const {
        return int(x) % 2;
    }

    Color evaluate(const Point2 &uv) const override { 
        if (isOddTile(uv.x() * m_scale[0]) == isOddTile(uv.y() * m_scale[1]))
            return m_color_0;
        else
            return m_color_1;
    }

    std::string toString() const override {
        return tfm::format("CheckerboardTexture[\n"
                           "  m_color_0 = %s\n"
                           "  m_color_1 = %s\n"
                            "  m_scale = %s\n"
                           "]",
                           indent(m_color_0),
                           indent(m_color_1),
                           indent(m_scale));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
