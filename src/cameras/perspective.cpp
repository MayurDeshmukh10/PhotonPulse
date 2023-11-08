#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
 * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
 * are directed in negative y direction ( @code ray.direction.y < 0 ).
 */
class Perspective : public Camera {
    const Vector focal_vector = Vector(0.f, 0.f, 1.f);
    Vector spanning_x;
    Vector spanning_y;
    float spanning_length;
    

public:
    Perspective(const Properties &properties): Camera(properties) {
        const float fov = properties.get<float>("fov");

        this->spanning_length = tan(fov / 2);

        this->spanning_x = Vector(spanning_length, 0.f, 0.f);
        this->spanning_y = Vector(0.f, spanning_length, 0.f);


        // hints:
        // * precompute any expensive operations here (most importantly trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
    }
        
    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        const Vector ray_direction = this->focal_vector + this->spanning_x * normalized.x() + this->spanning_y * normalized.y();

        auto ray = Ray(Point(0.f, 0.f, 0.f), ray_direction.normalized());

        ray = this->m_transform->apply(ray);   
        // hints:
        //     // * use m_transform to transform the local camera coordinate system into the world coordinate system
    

        return CameraSample{
            .ray=ray,
            .weight = Color(1.0f)
        };
    }

    std::string toString() const override {
        return tfm::format(
            "Perspective[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            indent(m_transform)
        );
    }
};

}

REGISTER_CAMERA(Perspective, "perspective")
