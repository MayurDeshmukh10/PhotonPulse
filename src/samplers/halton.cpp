#include <lightwave.hpp>
#include "pcg32.h"
#include "prime.h"

namespace lightwave {

    class Halton : public Sampler {
        private:
            int64_t haltonIndex;
            int dimension = 0;
            pcg32 m_pcg;
            float mask;

            float radicalInverse() {
                unsigned int base = Primes[dimension];
                double result = 0.0;
                double f = 1.0 / base;
                int64_t i = haltonIndex;
                while(i > 0) {
                    result += f * (i % base);
                    i = static_cast<int64_t>(i / base);
                    f /= base;
                }
                double maskedNumber = result + mask;
                if(maskedNumber >= 1) {
                    maskedNumber -= 1;
                }
                dimension = dimension + 1;

                return maskedNumber;
            }

        public:
            Halton(const Properties &properties)
            : Sampler(properties) {}

            void seed(int sampleIndex) override {
                haltonIndex = sampleIndex;
                dimension = 0;
            }

            void seed(const Point2i &pixel, int sampleIndex) override {
                const uint64_t a = (uint64_t(pixel.x()) << 32) ^ pixel.y();
                m_pcg.seed(1337, a);
                mask = m_pcg.nextFloat();
                haltonIndex = sampleIndex;
                dimension = 0;
            }

            float next() override {
                return radicalInverse();
            }

            Point2 next2D() override {
                return {radicalInverse(), radicalInverse()};
            }

            ref<Sampler> clone() const override {
                return std::make_shared<Halton>(*this);
            }


            std::string toString() const override {
                return tfm::format(
                    "Halton[\n"
                    "  count = %d\n"
                    "]",
                    m_samplesPerPixel
                );
            }
    };
}

REGISTER_SAMPLER(Halton, "halton")
