#include "draw.hpp"

#include "app.hpp"

#include "generation.hpp"

#include "imgui.h"
#include "raylib.h"
#include "raymath.h"

#include "modele3D.hpp"

void draw3DScene(AppContext& context) {
    ClearBackground(RAYWHITE);
    
    BeginMode3D(context.camera);

    Matrix const terrainCentering { getTerrainCenteringMatrix(context) };
    Vector3 const terrainCenterOffset { terrainCentering.m12, terrainCentering.m13, terrainCentering.m14 };

    DrawModel(context.model, terrainCenterOffset, 1.0f, WHITE);
    drawCubes(context, terrainCentering);
    DrawGrid(20, 1.0f);

    EndMode3D();
}

/*void drawCubes(AppContext const& context, Matrix const& terrainCentering)
{
    if (context.objectPositions.empty()) {
        return;
    }

    float const cubeHalfHeight { 0.5f * context.cubeScale };

    for (glm::vec3 const& pos : context.objectPositions) {
        Matrix const objectTranslation { MatrixTranslate(
            pos.x * context.terrainSize.x,
            pos.z * context.terrainSize.y + cubeHalfHeight,
            pos.y * context.terrainSize.z
        )};
        Matrix const centeredTranslation { MatrixMultiply(objectTranslation, terrainCentering) };
        Matrix const scale { MatrixScale(context.cubeScale, context.cubeScale, context.cubeScale) };
        Matrix const transform { MatrixMultiply(scale, centeredTranslation) };
        DrawMesh(context.cube, context.cubeMaterial, transform);
    }
}*/

void drawCubes(AppContext const& context, Matrix const& terrainCentering)
{
    if (context.objectPositions.empty())
        return;


    for (glm::vec3 const& pos : context.objectPositions){
        float cubeHalfHeight = 0.5f * context.cubeScale;

        Vector3 objectTranslation = {
            pos.x * context.terrainSize.x,
            pos.z * context.terrainSize.y + cubeHalfHeight,
            pos.y * context.terrainSize.z
        };

        //Forme de la matrice dans RayLib
        /*  m0  m4  m8  m12
            m1  m5  m9  m13
            m2  m6  m10 m14
            m3  m7  m11 m15*/
        //Pour une translation, on veut m12, m13 et m14

        Vector3 CentrerTerrain = {
            terrainCentering.m12, 
            terrainCentering.m13,
            terrainCentering.m14
        };

        objectTranslation.x += CentrerTerrain.x;
        objectTranslation.y += CentrerTerrain.y;
        objectTranslation.z += CentrerTerrain.z;

        float rotation = GetTime() * 200.0f;

        if (context.changementBiome == 0){
            DrawModelEx(
                context.modelBiome, //Model
                objectTranslation, //Position de l'objet
                {0,1,0},
                rotation,
                {2.0f,2.0f,2.0f}, //Homothétie
                WHITE //Garde la texture de base, si aucune en blanc
            );
        }
        else if (context.changementBiome == 1){
            DrawModelEx(
                context.modelBiome, //Model
                objectTranslation, //Position de l'objet
                {1,0,0},
                rotation,
                {0.1f,0.1f,0.1f}, //Homothétie
                WHITE //Garde la texture de base, si aucune en blanc
            );
        }
        else if (context.changementBiome == 2){
            DrawModelEx(
                context.modelBiome, //Model
                objectTranslation, //Position de l'objet
                {0,0,1},
                rotation,
                {0.3, 0.3, 0.3}, //Homothétie
                WHITE //Garde la texture de base, si aucune en blanc
            );
        }
        else if (context.changementBiome == 3) {
             DrawModelEx(
                context.modelBiome, //Model
                objectTranslation, //Position de l'objet
                {1,0,0},
                rotation,
                {0.3, 0.3, 0.3},//Homothétie
                WHITE //Garde la texture de base, si aucune en blanc
            );
        }
        else {
             DrawModel(
                context.modelBiome, //Model
                objectTranslation, //Position de l'objet
                0.5, //Homothétie
                WHITE //Garde la texture de base, si aucune en blanc
            );
        }
    }
}

void drawImGui(AppContext& context) {
    if(ImGui::Button("Generate random positions")) {
        generateObjectsPositions(context);
    }

    if (ImGui::CollapsingHeader("objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Cube Scale", &context.cubeScale, 0.01f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Apparence de l'île", ImGuiTreeNodeFlags_DefaultOpen)) {
        if(ImGui::SliderFloat("Grandeur de l'île", &context.changementMasque, 1, 2)){
            generateHeightmap(context);
            regenerateMeshFromImage(context);
        }
        if(ImGui::RadioButton("Île de base", &context.changementBiome, 0)){
            generateHeightmap(context);
            regenerateMeshFromImage(context);
            context.modelBiome = Model3DChoix(context);
            generateObjectsPositions(context);
        }
        if(ImGui::RadioButton("Mode pastel", &context.changementBiome, 1)){
            generateHeightmap(context);
            regenerateMeshFromImage(context);
            context.modelBiome = Model3DChoix(context);
            generateObjectsPositions(context);
        }
        if(ImGui::RadioButton("Banquise", &context.changementBiome, 2)){
            generateHeightmap(context);
            regenerateMeshFromImage(context);
            context.modelBiome = Model3DChoix(context);
            generateObjectsPositions(context);
        }
        if(ImGui::RadioButton("Volcan", &context.changementBiome, 3)){
            generateHeightmap(context);
            regenerateMeshFromImage(context);
            context.modelBiome = Model3DChoix(context);
            generateObjectsPositions(context);
        }
    }

    if (ImGui::CollapsingHeader("Poisson", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Poisson radius", &context.pointsGenerationParameters.r, 0.01f, 0.5f);
    }

    if (ImGui::CollapsingHeader("Filtre", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Hauteur Minimale", &context.heightMin, 0.0f, context.heightMax);
        ImGui::SliderFloat("Hauteur Maximale", &context.heightMax, context.heightMin, 1.0f);
    }

    if (ImGui::CollapsingHeader("Noise", ImGuiTreeNodeFlags_DefaultOpen)) {
    if(ImGui::SliderInt("Nombre d'octaves", &context.nboctaves, 1, 10)){
        generateHeightmap(context);
        regenerateMeshFromImage(context);
    }
     if(ImGui::SliderFloat("lacunarity", &context.lacunarity, 1, 10)){
        generateHeightmap(context);
        regenerateMeshFromImage(context);
    }
      if(ImGui::SliderFloat("gain", &context.gain, 0, 1)){
        generateHeightmap(context);
        regenerateMeshFromImage(context);
    }
    }
}

void drawRaylibUI(AppContext& context) {
    int screenWidth { GetScreenWidth() };
    
    float wanted_size { 400.f };
    float scale_factor { wanted_size / std::max(context.texture.width, context.texture.height) };
    float const preview_x { screenWidth - wanted_size - 20.f };
    float const preview_y { 20.f };
    float const preview_w { context.texture.width * scale_factor };
    float const preview_h { context.texture.height * scale_factor };
    // DrawTexture(context.texture, screenWidth - context.texture.width - 20, 20, WHITE);
    DrawTextureEx(context.texture, { preview_x, preview_y }, 0.0f, scale_factor, WHITE);
    DrawRectangleLines(screenWidth - wanted_size - 20, 20, wanted_size, wanted_size, GREEN);

    //draw positions on top of the heightmap
    for (auto const& pos : context.objectPositions)
    {
        // Remap normalized coordinates [0..1] to the preview image in screen space.
        float const px { preview_x + Clamp(pos.x, 0.0f, 1.0f) * preview_w };
        float const py { preview_y + Clamp(pos.y, 0.0f, 1.0f) * preview_h };

        DrawCircleV({ px, py }, 2.0f, RED);
    }

    DrawFPS(10, 10);
}

