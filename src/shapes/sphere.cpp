#include <lightwave.hpp>


namespace lightwave{
    class Sphere : public Shape {
        const float radius = 1.f;
        const Point center = Vector(0, 0, 0);

        inline void populate(SurfaceEvent &surf, const Point &position) const {
            surf.position = position;

            // surf.uv.x() = (position.x() + 1) / 2;
            // surf.uv.y() = (position.y() + 1) / 2;

            // Vector normal_vector = Vector(position.x(), position.y(), position.z()) - center;
            // Vector tangent = normal_vector.cross(Vector(0, 1, 0));
            // Vector bitangent = normal_vector.cross(tangent);

            // the tangent always points in positive x direction
            // surf.frame.tangent = Vector(0, 0, 0);
            // // the bitagent always points in positive y direction
            // surf.frame.bitangent = Vector(0, 0, 0);
            // // and accordingly, the normal always points in the positive z direction
            // surf.frame.normal = Vector(0, 0, 0);

            surf.pdf = 0;
        }

        inline bool is_quadratic_solution(float a, float b, float c, float *t0, float *t1) const {
            // Find quadratic discriminant
            double discriminant = (double)b * (double)b - 4 * (double)a * (double)c;
            if (discriminant < 0) return false;
            double root_discriminant = std::sqrt(discriminant);

            double q;
            if (b < 0)
                q = -.5 * (b - root_discriminant);
            else
                q = -.5 * (b + root_discriminant);
            *t0 = q / a;
            *t1 = c / q;
            if (*t0 > *t1) std::swap(*t0, *t1);
            return true;
        }

    public:
        Sphere(const Properties &properties){
            // There will probably be center and radius properties, but not for now
        }

        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override {
            float a = ray.direction.x() * ray.direction.x() + ray.direction.y() * ray.direction.y() + ray.direction.z() * ray.direction.z();
            float b = 2 * ( ray.origin.x() * ray.direction.x() + ray.origin.y() * ray.direction.y() + ray.origin.z() * ray.direction.z());
            float c = (ray.origin.x() * ray.origin.x() + ray.origin.y() * ray.origin.y() + ray.origin.z() * ray.origin.z()) - (radius * radius);
            float t0, t1, t;
            
            if(!is_quadratic_solution(a, b, c, &t0, &t1)) {
                its.t = 0.f;
                return false;
            }

            if (t0 < 0) {
                t0 = t1; // if t0 is negative, let's use t1 instead
                if (t0 < 0) {
                    its.t = 0.f;
                    return false; // both t0 and t1 are negative
                }
            }
            t = t0;

            if (t < Epsilon) {
                its.t = 0.f;
                return false;
            }

            const Point position = ray(t);
            its.t = t;
            Vector normal_vector = Vector(position.x(), position.y(), position.z()) - center;
            its.wo = normal_vector.normalized();
            populate(its, position);
            return true;
        }

        Bounds getBoundingBox() const override {
            return Bounds(Point(-radius, -radius, -radius), Point(radius, radius, radius));
        } 
        
        Point getCentroid() const override {
            return Point(0, 0, 0);
        }

        AreaSample sampleArea(Sampler &rng) const override{
            NOT_IMPLEMENTED
        }

        std::string toString() const override
        {
            return "Sphere[]";
        }
    };
}

REGISTER_SHAPE(Sphere, "sphere")
