#include <lightwave.hpp>
#ifdef LW_WITH_OIDN
#include <OpenImageDenoise/oidn.hpp>

namespace lightwave {

  class Denoise : public Postprocess {
      ref<Image> input;
      ref<Image> output;
      ref<Image> normal;
      ref<Image> albedo;

  public:
      Denoise(const Properties &properties): Postprocess(properties) {
        input = m_input;
        output = m_output;
        normal = m_normals;
        albedo = m_albedo;
      }

      void execute() override {
        auto width = input->resolution().x();
        auto height = input->resolution().y();
        output->initialize(input->resolution());

        oidn::DeviceRef device = oidn::newDevice(oidn::DeviceType::CPU);
        device.commit();

        oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
        filter.setImage("color", input->data(), oidn::Format::Float3, width, height); // beauty
        filter.setImage("albedo", albedo->data(), oidn::Format::Float3, width, height);
        filter.setImage("normal", normal->data(), oidn::Format::Float3, width, height);
        filter.setImage("output", output->data(), oidn::Format::Float3, width, height); // denoised beauty
        filter.set("hdr", true);
        filter.commit();
        filter.execute();

        output->save();

        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None) {
          std::cout << "Error: " << errorMessage << std::endl;
        }
      }

      std::string toString() const override {
        return "Denoise[]";
    }
  };

} // namespace lightwave

REGISTER_POSTPROCESS(Denoise, "denoising")
#endif