#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <engine/util/Logger.h>

#define VERTEX_POS 0
#define VERTEX_NORMAL 2
#define VERTEX_TANGENT 4
#define VERTEX_UV 8
#define VERTEX_COLOR 0x10

using IndexType = GLuint;

using SubMeshIndex = uint16_t;
using Indices = std::vector<IndexType>;
using Vertices = std::vector<glm::vec3>;
using Normals = std::vector<glm::vec3>;
using Tangents = std::vector<glm::vec3>;
using UVs = std::vector<glm::vec2>;
using Colors = std::vector<glm::vec3>;

class Mesh
{
public:
    struct SubMesh
    {
        Indices indices;
        Vertices vertices;
        Normals normals;
        Tangents tangents;
        UVs uvs;
        Colors colors;
    };

    struct SubMeshRenderData
    {
        GLuint vbo{ 0 };
        GLuint ibo{ 0 };
        GLuint vao{ 0 };
        GLenum renderMode{ GL_TRIANGLES };
    };

    class Builder
    {
        /**
        * Used to queue attributes to deduce offset and stride.
        */
        struct Attr
        {
            Attr() :offset(nullptr) {}
            Attr(GLint size, GLenum type, GLuint divisor, GLboolean normalized)
                :size(size), type(type), divisor(divisor), normalized(normalized), offset(nullptr) {}
            Attr(GLint size, GLenum type, GLuint divisor, GLboolean normalized, const void* offset, size_t stride)
                :size(size), type(type), divisor(divisor), normalized(normalized), offset(offset), stride(stride) {}

            GLint size{ 0 };
            GLenum type{ 0 };
            GLuint divisor{ 0 };
            GLboolean normalized{ false };
            const void* offset;
            size_t stride{ 0 };
        };

    public:
        void reset();

        /**
        * Creates a vertex buffer object with the given size and a pointer to the data.
        * Usage is defined to be GL_STATIC_DRAW per default.
        */
        Builder& createVBO(size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);

        /**
        * Creates an index buffer object with the given index count and a pointer to the data.
        * Specify the index type as a template paramter - e.g. createIBO<GLuint>()
        * Usage is defined to be GL_STATIC_DRAW per default.
        */
        template<class TIndexType>
        Builder& createIBO(size_t indexCount, const void* data, GLenum usage = GL_STATIC_DRAW);

        /**
        * Specify the next vertex attribute - offsets and stride will be deduced when finalize() is called.
        * Attributes in the vertex buffer object are expected to be interleaved.
        * Order of attribute() calls should correspond to the order of attributes in the interleaved buffer object.
        */
        Builder& attribute(GLint size, GLenum type, GLuint attribDivisor = 0, GLboolean normalized = false);

        /**
        * Specify the next vertex attribute with offset and stride.
        * Order of attribute() calls should correspond to the order of attributes in the buffer object.
        */
        Builder& attribute(GLint size, GLenum type, const void* offset, GLsizei stride, GLuint attribDivisor = 0, GLboolean normalized = false);
        
        /**
        * Creates a vertex array object and binds attributes.
        */
        void finalize(SubMesh& mesh, SubMeshRenderData& renderData);

    private:
        GLuint m_vbo{ 0 };
        GLuint m_ibo{ 0 };
        GLuint m_vao{ 0 };

        size_t m_indexCount{ 0 };
        size_t m_vertexCount{ 0 };
        bool m_deducedOffset{ true };
        size_t m_vboSize{ 0 };
        std::vector<Attr> m_attributes;
    };

    friend Builder;
    friend class MeshRenderer;
	friend class MeshRenderSystem;

    Mesh() {}
    ~Mesh();

    void setIndices(Indices indices, SubMeshIndex subMeshIdx);
    void setVertices(Vertices vertices, SubMeshIndex subMeshIdx);
    void setNormals(Normals normals, SubMeshIndex subMeshIdx);
    void setTangents(Tangents tangents, SubMeshIndex subMeshIdx);
    void setUVs(UVs uvs, SubMeshIndex subMeshIdx);
    void setColors(Colors colors, SubMeshIndex subMeshIdx);
    void setRenderMode(GLenum renderMode, SubMeshIndex subMeshIdx);
    void setSubMesh(const SubMesh& subMesh, SubMeshIndex subMeshIdx);
    void finalize();

	void setSubMeshes(const std::vector<SubMesh>& subMeshes);
    const std::vector<SubMesh>& getSubMeshes() const { return m_subMeshes; }
    const SubMesh& getSubMesh(size_t idx) const { return m_subMeshes[idx]; }

    void scale(const glm::vec3& s);

    /**
    * @fn  void Builder::mapToUnitCube();
    *
    * @brief   Maps the mesh vertices to a unit unit cube.
    */

    void mapToUnitCube();

    void bindAndRender();
private:
    void ensureCapacity(SubMeshIndex subMeshIdx);
    void ensureIntegrity();
    void freeGLResources() const;

private:
    std::vector<SubMesh> m_subMeshes;
    std::vector<SubMeshRenderData> m_subMeshRenderData;
};

template <class TIndexType>
Mesh::Builder& Mesh::Builder::createIBO(size_t indexCount, const void* data, GLenum usage)
{
    m_indexCount = indexCount;
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(TIndexType), data, usage);

    GL_ERROR_CHECK();
    return *this;
}
