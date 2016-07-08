#include "Mesh.h"
#include <assert.h>
#include <engine/util/convert.h>
#include <engine/util/util.h>

void Mesh::Builder::reset()
{
    *this = Builder();
}

Mesh::Builder& Mesh::Builder::createVBO(size_t size, const void* data, GLenum usage)
{
    m_vboSize = size;
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    GL_ERROR_CHECK();
    return *this;
}

Mesh::Builder& Mesh::Builder::attribute(GLint size, GLenum type, GLuint attribDivisor, GLboolean normalized)
{
    // Cannot deduce offset if user provides custom offsets
    // so make sure it doesn't happen
    assert(m_deducedOffset);

    // Queue attributes to deduce stride
    m_attributes.push_back(Attr(size, type, attribDivisor, normalized));
    return *this;
}

Mesh::Builder& Mesh::Builder::attribute(GLint size, GLenum type, const void* offset, GLsizei stride, GLuint attribDivisor, GLboolean normalized)
{
    m_deducedOffset = false;
    m_attributes.push_back(Attr(size, type, attribDivisor, normalized, offset, stride));
    return *this;
}

void Mesh::Builder::finalize(SubMesh& mesh, SubMeshRenderData& renderData)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    GL_ERROR_CHECK();

    size_t stride = 0;
    if (m_deducedOffset) // Compute stride
        for (Attr& attr : m_attributes)
            stride += attr.size * convert::sizeFromGLType(attr.type);

    GLuint idx = 0;
    size_t offset = 0;

    // Bind attributes
    for (Attr& attr : m_attributes)
    {
        glEnableVertexAttribArray(idx);
        if (m_deducedOffset)
            glVertexAttribPointer(idx, attr.size, attr.type, attr.normalized, GLsizei(stride), reinterpret_cast<void*>(offset));
        else
        {
            stride = attr.stride;
            glVertexAttribPointer(idx, attr.size, attr.type, attr.normalized, GLsizei(stride), attr.offset);
        }

        if (attr.divisor != 0)
            glVertexAttribDivisor(idx, attr.divisor);

        // Compute offset
        offset += convert::sizeFromGLType(attr.type) * attr.size;
        ++idx;
        GL_ERROR_CHECK();
    }

    // Unbind all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GL_ERROR_CHECK();

    renderData.vbo = m_vbo;
    renderData.ibo = m_ibo;
    renderData.vao = m_vao;
}

Mesh::~Mesh()
{
    freeGLResources();
}

void Mesh::setIndices(Indices indices, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].indices = indices;
}

void Mesh::setVertices(Vertices vertices, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].vertices = vertices;
}

void Mesh::setNormals(Normals normals, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].normals = normals;
}

void Mesh::setTangents(Tangents tangents, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].tangents = tangents;
}

void Mesh::setUVs(UVs uvs, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].uvs = uvs;
}

void Mesh::setColors(Colors colors, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx].colors = colors;
}

void Mesh::setRenderMode(GLenum renderMode, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshRenderData[subMeshIdx].renderMode = renderMode;
}

void Mesh::setSubMesh(const SubMesh& subMesh, SubMeshIndex subMeshIdx)
{
    ensureCapacity(subMeshIdx);
    m_subMeshes[subMeshIdx] = subMesh;
}

void Mesh::finalize()
{
    freeGLResources();

    // Go through all submeshes and create ibos/vbos/vaos
    for (size_t mi = 0; mi < m_subMeshes.size(); ++mi)
    {
        auto& subMesh = m_subMeshes[mi];
        auto& renderData = m_subMeshRenderData[mi];
        std::vector<float> vertices;

        if (subMesh.vertices.size() == 0)
            continue;

        for (size_t i = 0; i < subMesh.vertices.size(); ++i)
        {
            vertices.push_back(subMesh.vertices[i].x);
            vertices.push_back(subMesh.vertices[i].y);
            vertices.push_back(subMesh.vertices[i].z);

            if (subMesh.normals.size() > 0)
            {
                vertices.push_back(subMesh.normals[i].x);
                vertices.push_back(subMesh.normals[i].y);
                vertices.push_back(subMesh.normals[i].z);
            }

            if (subMesh.tangents.size() > 0)
            {
                vertices.push_back(subMesh.tangents[i].x);
                vertices.push_back(subMesh.tangents[i].y);
                vertices.push_back(subMesh.tangents[i].z);
            }

            if (subMesh.uvs.size() > 0)
            {
                vertices.push_back(subMesh.uvs[i].x);
                vertices.push_back(subMesh.uvs[i].y);
            }

            if (subMesh.colors.size() > 0)
            {
                vertices.push_back(subMesh.colors[i].r);
                vertices.push_back(subMesh.colors[i].g);
                vertices.push_back(subMesh.colors[i].b);
            }
        }

        Builder builder;
        builder.createVBO(vertices.size() * sizeof(float), &vertices[0]);

        // Position
        builder.attribute(3, GL_FLOAT);

        if (subMesh.normals.size() > 0)
            builder.attribute(3, GL_FLOAT);

        if (subMesh.tangents.size() > 0)
            builder.attribute(3, GL_FLOAT);

        if (subMesh.uvs.size() > 0)
            builder.attribute(2, GL_FLOAT);

        if (subMesh.colors.size() > 0)
            builder.attribute(3, GL_FLOAT);

        if (subMesh.indices.size() > 0)
            builder.createIBO<GLuint>(subMesh.indices.size(), &subMesh.indices[0]);

        builder.finalize(subMesh, renderData);
    }
}

void Mesh::setSubMeshes(const std::vector<SubMesh>& subMeshes)
{
	m_subMeshes = subMeshes;
	m_subMeshRenderData.resize(m_subMeshes.size());
}

void Mesh::scale(const glm::vec3& s)
{
    for (auto& subMesh : m_subMeshes)
        for (auto& v : subMesh.vertices)
            v *= s;
}

void Mesh::mapToUnitCube()
{
    BBox meshBBox = util::computeBBox(*this);
    float scaleInv = 1.0f / meshBBox.scale()[meshBBox.maximumExtent()];
    glm::vec3 offset = -meshBBox.min();

    for (auto& subMesh : m_subMeshes)
    {
        for (size_t i = 0; i < subMesh.vertices.size(); ++i)
        {
            subMesh.vertices[i] = (subMesh.vertices[i] + offset) * scaleInv;
        }
    }

    finalize();
}

void Mesh::bindAndRender()
{
    for (SubMeshIndex i = 0; i < m_subMeshes.size(); ++i)
    {
        SubMesh& subMesh = m_subMeshes[i];
        SubMeshRenderData& renderData = m_subMeshRenderData[i];

        assert(renderData.vbo != 0 && renderData.vao != 0);

        glBindVertexArray(renderData.vao);

        if (subMesh.indices.size() > 0)
        {
            assert(renderData.ibo != 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.ibo);
        }

        if (subMesh.indices.size() > 0)
            glDrawElements(renderData.renderMode, GLsizei(subMesh.indices.size()), GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(renderData.renderMode, 0, GLsizei(subMesh.vertices.size()));
    }
}

void Mesh::ensureCapacity(SubMeshIndex subMeshIdx)
{
    m_subMeshes.resize(subMeshIdx + 1);
    m_subMeshRenderData.resize(subMeshIdx + 1);
}

void Mesh::ensureIntegrity()
{
#if defined(DEBUG) || defined(_DEBUG)
    for (auto& sm : m_subMeshes)
    {
        assert((sm.normals.size() == 0 || sm.normals.size() == sm.vertices.size()) &&
               (sm.tangents.size() == 0 || sm.tangents.size() == sm.vertices.size()) &&
               (sm.uvs.size() == 0 || sm.uvs.size() == sm.vertices.size()) &&
               (sm.colors.size() == 0 || sm.colors.size() == sm.vertices.size()));
    }
#endif
}

void Mesh::freeGLResources() const
{
    for (auto& renderData : m_subMeshRenderData)
    {
        if (renderData.vao != 0)
            glDeleteVertexArrays(1, &renderData.vao);

        if (renderData.vbo != 0)
            glDeleteBuffers(1, &renderData.vbo);

        if (renderData.ibo != 0)
            glDeleteBuffers(1, &renderData.ibo);
    }
}
