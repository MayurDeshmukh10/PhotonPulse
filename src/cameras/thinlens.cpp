#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief A thinlens camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
 * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
 * are directed in negative y direction ( @code ray.direction.y < 0 ).
 */
class Thinlens : public Camera {
    const Vector focal_vector = Vector(0.f, 0.f, 1.f);
    Vector spanning_x;
    Vector spanning_y;
    float spanning_length;

    float aperture;
    float focus_distance;

public:
    Thinlens(const Properties &properties): Camera(properties) {
        aperture = properties.get<float>("apertureRadius");
        focus_distance = properties.get<float>("focusDistance"); // distance from camera to focal plane

        const float fov = properties.get<float>("fov");
        const float aspect_ratio = 1.0 * this->m_resolution.x() / this->m_resolution.y();
    
        this->spanning_length = tan((fov*M_PI/180)/2);

        if (properties.get<std::string>("fovAxis") == "x") {            
            this->spanning_x = Vector(spanning_length, 0.f, 0.f);
            this->spanning_y = Vector(0.f, spanning_length / aspect_ratio, 0.f);
        }
        else if (properties.get<std::string>("fovAxis") == "y") {            
            this->spanning_x = Vector(spanning_length*aspect_ratio, 0.f, 0.f);
            this->spanning_y = Vector(0.f, spanning_length, 0.f);
        }
        else {
            throw std::runtime_error("Invalid fovAxis");
        }

    }
        
    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        const Vector initial_direction = this->focal_vector + this->spanning_x * normalized.x() + this->spanning_y * normalized.y();

        Point focus_point = Point(0.f, 0.f, 0.f) + initial_direction * focus_distance;

        Point2 starting_point_2d = squareToUniformDiskConcentric(rng.next2D());
        starting_point_2d.x() *= aperture;
        starting_point_2d.y() *= aperture;
        Point starting_point = Point(starting_point_2d.x(), starting_point_2d.y(), 0.f);

        const Vector ray_direction = focus_point - starting_point;
        
        auto ray = Ray(starting_point, ray_direction.normalized());
        ray = this->m_transform->apply(ray).normalized();   

        return CameraSample{
            .ray=ray,
            .weight = Color(1.0f)
        };
    }

    std::string toString() const override {
        return tfm::format(
            "Thinlens[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "  aperture = %f,\n"
            "  focus_distance = %f,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            aperture,
            focus_distance,
            indent(m_transform)
        );
    }
};

}

REGISTER_CAMERA(Thinlens, "thinlens")
