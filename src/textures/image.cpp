#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                           {
                                               { "clamp", BorderMode::Clamp },
                                               { "repeat", BorderMode::Repeat },
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter", FilterMode::Bilinear,
            {
                { "nearest", FilterMode::Nearest },
                { "bilinear", FilterMode::Bilinear },
            });
    }

    Point2i getBorderedPixelCoords(const Point2 &pixel_xy) const {
        Point2i rounded_coords = Point2i(
            std::round(pixel_xy.x()),
            std::round(pixel_xy.y())
        );

        if (m_border == BorderMode::Clamp) {
            return Point2i(
                std::clamp(rounded_coords.x(), 0, m_image->resolution().x() - 1),
                std::clamp(rounded_coords.y(), 0, m_image->resolution().y() - 1));
        } else {
            return Point2i(
                rounded_coords.x() % m_image->resolution().x(),
                rounded_coords.y() % m_image->resolution().y());
        }
    }

    Color sampleNearestNeighbor(const Point2 &scaled_uv) const {
        const Point2i pixel_xy = getBorderedPixelCoords(scaled_uv);
        return (*m_image)(pixel_xy);
    }

    float getBilinearWeight(const Point2 &scaled_uv, const Point2 &pixel_xy) const {
        float x_distance = std::abs(scaled_uv.x() - pixel_xy.x());
        float y_distance = std::abs(scaled_uv.y() - pixel_xy.y());

        return (1 - x_distance) * (1 - y_distance);
    }

    Color sampleBilinear(const Point2 &scaled_uv) const {
        const Point2 lower_left = Point2(
            std::floor(scaled_uv.x()),
            std::floor(scaled_uv.y())
        );

        const Point2 upper_right = Point2(
            std::ceil(scaled_uv.x()),
            std::ceil(scaled_uv.y())
        );

        const Point2 lower_right = Point2(
            upper_right.x(),
            lower_left.y()
        );

        const Point2 upper_left = Point2(
            lower_left.x(),
            upper_right.y()
        );

        return getBilinearWeight(scaled_uv, lower_left) * (*m_image)(getBorderedPixelCoords(lower_left)) +
               getBilinearWeight(scaled_uv, lower_right) * (*m_image)(getBorderedPixelCoords(lower_right)) +
               getBilinearWeight(scaled_uv, upper_left) * (*m_image)(getBorderedPixelCoords(upper_left)) +
               getBilinearWeight(scaled_uv, upper_right) * (*m_image)(getBorderedPixelCoords(upper_right));
    }

    Color evaluate(const Point2 &uv) const override {
        const Point2 scaled_uv = Point2(
            uv.x() * m_image->resolution().x(),
            uv.y() * m_image->resolution().y()
        );


        if (m_filter == FilterMode::Nearest) {
            return sampleNearestNeighbor(scaled_uv);
        } else {
            // bilinear filtering
            return sampleBilinear(scaled_uv);
        }
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
