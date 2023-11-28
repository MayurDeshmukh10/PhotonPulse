#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
    bool m_smoothNormals;

    inline void populate(SurfaceEvent &surf, const Point &position, Vector normal) const {
        surf.position = position;
        surf.frame.normal = normal.normalized();

        Vector tangent;
        if ((normal - Vector(0,0,1)).length() < Epsilon) {
            tangent = Vector(1, 0, 0);
        }
        else{
            tangent = normal.cross(Vector(0, 0, 1)).normalized();
        }
        Vector bitangent = normal.cross(tangent);

        surf.frame.tangent = tangent.normalized();
        surf.frame.bitangent = bitangent.normalized();
    }

protected:
    int numberOfPrimitives() const override {
        return int(m_triangles.size());
    }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its, Sampler &rng) const override {
        float inv_det, u, v;
        Vector3i indexes = m_triangles[primitiveIndex];
        Vertex v0 = m_vertices[indexes.x()];
        Vertex v1 = m_vertices[indexes.y()];
        Vertex v2 = m_vertices[indexes.z()];

        Vector edge1 = {v1.position.x() - v0.position.x(), v1.position.y() - v0.position.y(), v1.position.z() - v0.position.z()};
        Vector edge2 = {v2.position.x() - v0.position.x(), v2.position.y() - v0.position.y(), v2.position.z() - v0.position.z()};
        Vector rayXedge2 = ray.direction.cross(edge2);
        float det = edge1.dot(rayXedge2);

        // This ray is parallel to the triangle
        if(det > -Epsilon && det < Epsilon) {
            return false;
        }

        inv_det = 1.0f / det;

        // displacement from the origin of the ray to the first vertex of the triangle.
        Vector s = { ray.origin.x() - v0.position.x(), ray.origin.y() - v0.position.y(), ray.origin.z() - v0.position.z() };

        // barycentric coordinate
        u = inv_det * s.dot(rayXedge2);

        // check if u is outside the triangle
        if(u < 0.0f || u > 1.0f) {
            return false;
        }

        // Calculate vector perpendicular to both s and edge1
        Vector sXedge1 = s.cross(edge1);

        // barycentric coordinate
        v = inv_det * ray.direction.dot(sXedge1);

        // check if v is outside the triangle
        if(v < 0.0f || u + v > 1.0f) {
            return false;
        }

        const float t = inv_det * edge2.dot(sXedge1);

        // Check if the intersection point is in front of the ray origin
        if(t > Epsilon and t < its.t) {
            its.t = t;
            const Point position = ray(t);
            Vector normal;
            if(m_smoothNormals) {
                Vertex interpolated_vertex = Vertex::interpolate(Vector2(u, v), v0, v1, v2);
                normal = interpolated_vertex.normal.normalized();
            } else {
                normal = edge1.cross(edge2).normalized();
            }
            
            populate(its, position, normal);

            return true;

        } else {
            return false;
        }
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        Vector3i indexes = m_triangles[primitiveIndex];
        Vertex v0 = m_vertices[indexes.x()];
        Vertex v1 = m_vertices[indexes.y()];
        Vertex v2 = m_vertices[indexes.z()];

        float min_v_x = std::min(std::min(v0.position.x(), v1.position.x()), v2.position.x());
        float min_v_y = std::min(std::min(v0.position.y(), v1.position.y()), v2.position.y());
        float min_v_z = std::min(std::min(v0.position.z(), v1.position.z()), v2.position.z());


        float max_v_x = std::max(std::max(v0.position.x(), v1.position.x()), v2.position.x());
        float max_v_y = std::max(std::max(v0.position.y(), v1.position.y()), v2.position.y());
        float max_v_z = std::max(std::max(v0.position.z(), v1.position.z()), v2.position.z());
        

		return Bounds(Point(min_v_x, min_v_y, min_v_z), Point(max_v_x, max_v_y, max_v_z));
    }

    Point getCentroid(int primitiveIndex) const override {
        Vector3i indexes = m_triangles[primitiveIndex];
        Vertex v0 = m_vertices[indexes.x()];
        Vertex v1 = m_vertices[indexes.y()];
        Vertex v2 = m_vertices[indexes.z()];

        float centroid_x = (v0.position.x() + v1.position.x() + v2.position.x()) / 3;
        float centroid_y = (v0.position.y() + v1.position.y() + v2.position.y()) / 3;
        float centroid_z = (v0.position.z() + v1.position.z() + v2.position.z()) / 3;
        
        return Point(centroid_x, centroid_y, centroid_z);
    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath.string(), m_triangles, m_vertices);
        logger(EInfo, "loaded ply with %d triangles, %d vertices",
            m_triangles.size(),
            m_vertices.size()
        );
        buildAccelerationStructure();
    }

    AreaSample sampleArea(Sampler &rng) const override {
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string()
        );
    }
};

}

REGISTER_SHAPE(TriangleMesh, "mesh")
