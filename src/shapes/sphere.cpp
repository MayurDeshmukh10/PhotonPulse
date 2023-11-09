#include <lightwave.hpp>


namespace lightwave{
    class Sphere : public Shape{
    public:
        Sphere(const Properties &properties){
            NOT_IMPLEMENTED
        }
        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override{
            NOT_IMPLEMENTED
        } 
        Bounds getBoundingBox() const override{NOT_IMPLEMENTED} Point getCentroid() const override{
            NOT_IMPLEMENTED
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
