#include <lightwave.hpp>

#ifdef LW_WITH_OPENCV
#include <opencv2/opencv.hpp>

namespace lightwave {

    class ImageBloom : public Postprocess {
        ref<Image> input;
        ref<Image> output;

        float m_threshold;
        float m_radius;
        float m_strength;

    public:
        ImageBloom(const Properties &properties) : Postprocess(properties) {
            input = m_input;
            output = m_output;

            m_threshold = properties.get<float>("threshold");
            m_radius = properties.get<float>("radius");
            m_strength = properties.get<float>("strength");
        }

        float get_contribution(Color pixel) {
            float contribution = pixel.luminance() - m_threshold;
            contribution *= m_strength;
            
            return contribution > 0 ? contribution : 0;
        }

        Image get_weighted_highlight_image(const Image& image) {
            auto width = image.resolution().x();
            auto height = image.resolution().y();

            Image highlight_image = Image(image.resolution());

            Point2i pixel_coords;

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++){
                    pixel_coords = Point2i(x, y);

                    Color pixel_value = image.get(pixel_coords);
                    highlight_image(pixel_coords) = pixel_value * get_contribution(pixel_value);
                }
            }

            return highlight_image;
        }

        Image get_blurred_image(const Image & image) {
            cv::Mat image_mat = image.toCvMat();
            cv::Mat blurred_mat;
            cv::GaussianBlur(image_mat, blurred_mat, cv::Size(m_radius, m_radius), 0);

            return Image(blurred_mat);
        }

        Image get_combined_image(const Image& base_image, const Image& highlight_image) {
            auto width = base_image.resolution().x();
            auto height = base_image.resolution().y();

            assert (base_image.resolution() == highlight_image.resolution());
            Image combined_image = Image(base_image.resolution());

            Point2i pixel_coords;

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++){
                    pixel_coords = Point2i(x, y);

                    Color base_pixel_value = base_image.get(pixel_coords);
                    Color highlight_pixel_value = highlight_image.get(pixel_coords);

                    combined_image(pixel_coords) = base_pixel_value + highlight_pixel_value;
                }
            }

            return combined_image;
        }

        void execute() override {
            Image highlight_image = get_weighted_highlight_image(*input);
            Image blurred_image = get_blurred_image(highlight_image);

            Image combined_image = get_combined_image(*input, blurred_image);

            output->copy(combined_image);
            output->save();
        }

        std::string toString() const override {
            return "ImageBloom[]";
        }
    };
};


REGISTER_POSTPROCESS(ImageBloom, "image_bloom")

#endif // LW_WITH_OPENCV
