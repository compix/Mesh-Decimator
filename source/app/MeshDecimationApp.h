#pragma once
#include <engine/Application.h>
#include <engine/camera/Camera.h>
#include <engine/rendering/geometry/Mesh.h>
#include <engine/rendering/shader/Shader.h>
#include <engine/input/Input.h>
#include "ReducibleDirectedEdgeMesh.h"
#include <engine/resource/Model.h>
#include <engine/util/Timer.h>

// Used to cache mesh related state
struct MeshWrapper
{
    MeshWrapper() {}
    MeshWrapper(std::shared_ptr<ReducibleDirectedEdgeMesh> originalEdgeMesh, const std::string& name)
        :originalEdgeMesh(originalEdgeMesh), name(name) {}

    std::shared_ptr<ReducibleDirectedEdgeMesh> originalEdgeMesh;
    std::vector<EdgeID> collapsedEdges;
    bool initialized{ false };
    std::string name;
};

class MeshDecimationApp : public Application, InputHandler
{
protected:
    void update() override;
    void initUpdate() override;

    void createFlatShadedMesh();

    void onMouseDown(const SDL_MouseButtonEvent& e) override;
    void onMousewheel(float delta) override;

private:
    void addMesh(const std::string& path, const std::string& name);
    void selectMesh(int meshIdx);
    void selectShading(int selection);
    void loadCollapsedEdgesCache();

    void handleGUI();
    void guiMeshSelection();
    void guiMeshRadioButton(int meshIdx);
    void guiShadingSelection();
    void guiVertexCountSlider();
    void guiShowStats();
    void MeshDecimationApp::guiShowFPS() const;

    void updateModelRotation();
    glm::vec3 computeArcballVector(glm::vec2 pos) const;
private:
    Camera m_modelCamera;

    std::vector<MeshWrapper> m_meshes;
    MeshWrapper* m_curMesh{ nullptr };

    ReducibleDirectedEdgeMesh m_curLoadingMesh;
    ReducibleDirectedEdgeMesh m_reducibleMesh;
    Mesh* m_activeMesh{ nullptr };
    Mesh m_phongShadedMesh;
    Mesh m_flatShadedMesh;

    Shader m_shader;

    glm::vec3 m_meshColor{ 1.f };

    int m_curVertexCount{ 0 };
    int m_shadingSelection{ 0 };
    int m_meshSelection{ -1 };
    int m_curLoadingMeshIdx{ 0 };

    glm::quat m_modelRotation;
    glm::quat m_modelRotationBeforeDrag;
    bool m_guiFocus{ false };
    float m_zoomInc{ 0.1f };
};
