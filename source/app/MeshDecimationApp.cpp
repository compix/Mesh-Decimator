#include "MeshDecimationApp.h"
#include <engine/util/Timer.h>
#include <engine/input/Input.h>
#include <engine/Engine.h>
#include <engine/rendering/Screen.h>
#include <engine/rendering/util/GLUtil.h>
#include <engine/resource/AssetImporter.h>
#include <imgui/imgui.h>

void MeshDecimationApp::initUpdate()
{
    Input::subscribe(this);

    m_modelCamera.setPerspective(45.0f, float(Screen::getWidth()), float(Screen::getHeight()), 0.3f, 100.0f);
    m_modelCamera.setViewport(0.0f, 0.0f, float(Screen::getWidth()), float(Screen::getHeight()));
    m_modelCamera.setPosition(glm::vec3(0.0f, 0.0f, -1.25f));
    m_modelCamera.lookAt(glm::vec3(0.0f));

    m_engine->registerCamera(&m_modelCamera);

    m_shader.load("assets/shaders/lit.vert", "assets/shaders/lit.frag", { "in_pos", "in_normal" });

    addMesh("assets/meshes/Sphere.obj", "Sphere");
    addMesh("assets/meshes/bunny.obj", "Stanford Bunny");
    m_curLoadingMesh = *m_meshes[m_curLoadingMeshIdx].originalEdgeMesh;

    glEnable(GL_SCISSOR_TEST);

    m_initializing = false;
}

void MeshDecimationApp::addMesh(const std::string& path, const std::string& name)
{
    // Assuming the model has only one sub mesh for simplicity
    auto model = AssetImporter::importObjPositions(path);
    model->mapToUnitCube();
    auto reducibleMesh = std::make_shared<ReducibleDirectedEdgeMesh>(model->getSubMesh(0));
    m_meshes.push_back(MeshWrapper(reducibleMesh, name));
}

void MeshDecimationApp::update()
{
    // The full reduction of a mesh is the slowest process and can take a long time on detailed models
    // -> Don't freeze the program and process all models sequentially with a time constraint.
    if (size_t(m_curLoadingMeshIdx) < m_meshes.size() && m_meshes.size() > 0)
        loadCollapsedEdgesCache();

    m_modelCamera.updateViewMatrix();
    updateModelRotation();

    GL::setViewport(m_modelCamera.getViewport());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (m_meshSelection >= 0)
    {
        m_shader.bind();
        glm::mat4 pivotTranslation = glm::translate(glm::vec3(-0.5f));
        m_shader.setModel(glm::toMat4(m_modelRotation) * pivotTranslation);
        m_shader.setCamera(m_modelCamera.view(), m_modelCamera.proj());
        m_shader.setColor(glm::vec4(m_meshColor, 1.0f));
        m_activeMesh->bindAndRender();
    }

    handleGUI();
}

void MeshDecimationApp::createFlatShadedMesh()
{
    auto& originalSubMesh = m_phongShadedMesh.getSubMesh(0);
    auto flatShadedSubMesh = m_phongShadedMesh.getSubMesh(0);

    flatShadedSubMesh.tangents.clear();
    flatShadedSubMesh.vertices.resize(flatShadedSubMesh.indices.size());
    flatShadedSubMesh.normals.resize(flatShadedSubMesh.indices.size());
    flatShadedSubMesh.uvs.clear();

    auto triangleCount = flatShadedSubMesh.indices.size() / 3;

    for (size_t i = 0; i < triangleCount; ++i)
    {
        auto idxStart = i * 3;

        auto vi0 = originalSubMesh.indices[idxStart];
        auto vi1 = originalSubMesh.indices[idxStart + 1];
        auto vi2 = originalSubMesh.indices[idxStart + 2];

        auto v0 = originalSubMesh.vertices[vi0];
        auto v1 = originalSubMesh.vertices[vi1];
        auto v2 = originalSubMesh.vertices[vi2];

        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        flatShadedSubMesh.vertices[idxStart] = v0;
        flatShadedSubMesh.vertices[idxStart+1] = v1;
        flatShadedSubMesh.vertices[idxStart+2] = v2;

        flatShadedSubMesh.normals[idxStart] = n;
        flatShadedSubMesh.normals[idxStart + 1] = n;
        flatShadedSubMesh.normals[idxStart + 2] = n;

        flatShadedSubMesh.indices[idxStart] = idxStart;
        flatShadedSubMesh.indices[idxStart+1] = idxStart+1;
        flatShadedSubMesh.indices[idxStart+2] = idxStart+2;
    }

    m_flatShadedMesh.setSubMesh(flatShadedSubMesh, 0);
    m_flatShadedMesh.finalize();
}

void MeshDecimationApp::onMouseDown(const SDL_MouseButtonEvent& e)
{
    glm::vec2 pos = glm::vec2(Input::mousePosition);
    if (ImGui::IsAnyItemHovered())
        m_guiFocus = true;
    else if (m_modelCamera.getViewport().inside(pos))
        m_guiFocus = false;
}

void MeshDecimationApp::onMousewheel(float delta)
{
    // Value can vary between different operating systems, browsers and mice
    // -> Make it independent
    float nDelta = delta < 0.0f ? -1.0f : 1.0f;
    m_modelCamera.zoom(nDelta * m_zoomInc);
}

void MeshDecimationApp::selectMesh(int meshIdx)
{
    m_phongShadedMesh = Mesh();

    m_curMesh = &m_meshes[meshIdx];

    m_reducibleMesh = *m_curMesh->originalEdgeMesh;

    m_phongShadedMesh.setSubMesh(m_reducibleMesh.getReducedSubMesh(), 0);
    m_phongShadedMesh.finalize();
    createFlatShadedMesh();

    selectShading(m_shadingSelection);

    m_curVertexCount = int(m_curMesh->originalEdgeMesh->getVertices().size());
    // Reset to a 180� rotation
    m_modelRotation = glm::quat(cosf(math::PI_DIV_2), 0.0f, sinf(math::PI_DIV_2), 0.0f);
}

void MeshDecimationApp::selectShading(int selection)
{
    switch (m_shadingSelection)
    {
    case 0:
        m_activeMesh = &m_flatShadedMesh;
        break;
    case 1:
        m_activeMesh = &m_phongShadedMesh;
        break;
    }
}

void MeshDecimationApp::loadCollapsedEdgesCache()
{
    float maxAllowedLoadingTimePerFrame = 0.016f; // 16ms

    Timer timer;
    std::vector<EdgeID>& collapsedEdges = m_meshes[m_curLoadingMeshIdx].collapsedEdges;

    while (timer.totalTime() < maxAllowedLoadingTimePerFrame)
    {
        EdgeID collapsedEdge = m_curLoadingMesh.reduce();
        if (collapsedEdge < 0)
            break;

        collapsedEdges.push_back(collapsedEdge);
        timer.tick();
    }

    if (m_curLoadingMesh.reachedMaxReduction())
    {
        m_meshes[m_curLoadingMeshIdx].initialized = true;

        ++m_curLoadingMeshIdx;
        if (size_t(m_curLoadingMeshIdx) < m_meshes.size())
            m_curLoadingMesh = *m_meshes[m_curLoadingMeshIdx].originalEdgeMesh;

        if (m_meshSelection < 0)
        {
            m_meshSelection = 0;
            selectMesh(0);
        }   
    }
}

void MeshDecimationApp::updateModelRotation()
{
    if (!Input::leftDrag().isDragging() || m_guiFocus)
    {
        m_modelRotationBeforeDrag = m_modelRotation;
        return;
    }

    if (glm::length2(Input::leftDrag().getDragDeltaToStart()) == 0.0f)
        return;

    // Simple arcball model rotation based on: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
    auto vp = m_modelCamera.getViewport();
    float viewportRadius = 0.5f * sqrtf(vp.width() * vp.width() + vp.height() * vp.height());
    glm::vec2 viewportCenter = vp.center();

    glm::vec2 startPos = (Input::leftDrag().getStartDragPos() - viewportCenter) / viewportRadius;
    glm::vec2 curPos = (Input::leftDrag().getCurDragPos() - viewportCenter) / viewportRadius;

    glm::vec3 toStart = glm::vec3(m_modelCamera.viewInverse() * glm::vec4(computeArcballVector(startPos), 0.0f));
    glm::vec3 toEnd = glm::vec3(m_modelCamera.viewInverse() * glm::vec4(computeArcballVector(curPos), 0.0f));

    float angle = 1.5f * std::acosf(std::min(1.0f, glm::dot(toStart, toEnd)));
    glm::vec3 rotationAxis = glm::normalize(glm::cross(toStart, toEnd));
    m_modelRotation = glm::angleAxis(angle, rotationAxis) * m_modelRotationBeforeDrag;
}

glm::vec3 MeshDecimationApp::computeArcballVector(glm::vec2 pos) const
{
    // Radius of the ball
    static float r = 1.0f;

    // Use pythagorean theorem to compute z
    glm::vec3 v = glm::vec3(pos, 0.0f);
    float sq = v.x * v.x + v.y * v.y;
    if (sq <= r * r)
        v.z = -std::sqrtf(r * r - sq);

    return glm::normalize(v);
}

void MeshDecimationApp::handleGUI()
{
    static bool open = true;
    ImGui::SetNextWindowSize(ImVec2(250.0f, 600.0f));
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0.f, 0.f, 0.f, 0.f));
    ImGui::Begin("Test", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    guiVertexCountSlider();
    guiMeshSelection();
    guiShadingSelection();
    guiShowStats();

    ImGui::End();
    ImGui::PopStyleColor(1);
}

void MeshDecimationApp::guiMeshSelection()
{
    static int lastSelection = 0;

    ImGui::NewLine();
    ImGui::Text("- Mesh Selection -");

    for (size_t i = 0; i < m_meshes.size(); ++i)
        guiMeshRadioButton(int(i));

    if (m_curLoadingMeshIdx == 0)
        return;

    if (m_meshSelection != lastSelection)
    {
        selectMesh(m_meshSelection);
        lastSelection = m_meshSelection;
    }
}

void MeshDecimationApp::guiMeshRadioButton(int meshIdx)
{
    auto& mesh = m_meshes[meshIdx];
    if (mesh.initialized)
        ImGui::RadioButton(mesh.name.c_str(), &m_meshSelection, meshIdx);
    else
    {
        std::string loadingMsg = "Processing " + mesh.name + " ...";
        ImGui::Text(loadingMsg.c_str(), "%s");
    } 
}

void MeshDecimationApp::guiShadingSelection()
{
    static int lastSelection = m_shadingSelection;

    ImGui::NewLine();
    ImGui::Text("- Shading Selection -");
    ImGui::RadioButton("Flat", &m_shadingSelection, 0);
    ImGui::RadioButton("Phong", &m_shadingSelection, 1);

    if (m_shadingSelection != lastSelection)
    {
        selectShading(m_shadingSelection);
        lastSelection = m_shadingSelection;
    }
}

static void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(desc, "%s");
}

void MeshDecimationApp::guiVertexCountSlider()
{
    if (m_curLoadingMeshIdx == 0)
        return;

    ImGui::Text("- Vertex Count -");
    auto& collapsedEdges = m_curMesh->collapsedEdges;
    int maxVertexCount = int(m_curMesh->originalEdgeMesh->getVertices().size());
    int minVertexCount = int(maxVertexCount - m_curMesh->collapsedEdges.size());
    ImGui::SliderInt("", &m_curVertexCount, minVertexCount, maxVertexCount);
    ImGui::SameLine(); ShowHelpMarker("CTRL+click to input value.");
    m_curVertexCount = math::clamp(m_curVertexCount, minVertexCount, maxVertexCount);

    static int lastVertexCount = -1;
    int reductionCount = maxVertexCount - m_curVertexCount;

    // To allow interactive speeds collapse candidates are cached for each mesh during program startup.
    // Upon slider interaction the original mesh is restored and reduced to the desired vertex count.
    // This is still an expensive process and won't work interactively for meshes with millions of vertices.
    if (lastVertexCount != m_curVertexCount)
    {
        m_reducibleMesh = *m_curMesh->originalEdgeMesh;

        for (int i = 0; i < reductionCount; ++i)
            m_reducibleMesh.collapse(collapsedEdges[i]);

        m_phongShadedMesh.setSubMesh(m_reducibleMesh.getReducedSubMesh(), 0);
        m_phongShadedMesh.finalize();
        createFlatShadedMesh();
        lastVertexCount = m_curVertexCount;
    }
}

void MeshDecimationApp::guiShowStats()
{
    ImGui::NewLine();

    if (m_curLoadingMeshIdx != 0)
    {
        std::string triangleStr = "Triangles: " + std::to_string(m_reducibleMesh.getFaceCount());
        ImGui::Text(triangleStr.c_str(), "%s");
    }

    guiShowFPS();
}

void MeshDecimationApp::guiShowFPS() const
{
    static Seconds left = 0.f;
    static uint32_t fps = 0;
    static uint32_t fpsCounter = 0;
    static Seconds frameTime = 0.f;
    left -= Time::deltaTime();

    if (left <= 0.f)
    {
        frameTime = Seconds(1000) / std::max(fpsCounter, 1u);
        left = 1.f;
        fps = fpsCounter;
        fpsCounter = 0;
    }

    std::stringstream ss;
    ss.precision(2);

    std::string fpsStr = "FPS: " + std::to_string(fps);
    ss << "Frame time: " << std::fixed << frameTime << "ms";
    std::string totalTimeStr = "Total time: " + std::to_string(int(Time::totalTime()));

    ImGui::Text(fpsStr.c_str(), "%s");
    ImGui::Text(ss.str().c_str(), "%s");
    ImGui::Text(totalTimeStr.c_str(), "%s");

    fpsCounter++;
}
