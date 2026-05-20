#include "generation.hpp"

#include "noise.hpp"
#include "raylib.h"

#include "utils/raylibUtils.hpp"
#include <bits/stdc++.h>
#include <algorithm> // for std::clamp
#include <cmath>

#include "degradeCouleur.hpp"


std::vector<glm::vec2> generate2DPositions([[maybe_unused]] PointsGenerationParameters const& params) {
    

    // TODO(student): implement Poisson disk sampling to replace the above naive random generation
    // points output should be in [0..1] range, where (0,0) is onoat r = 0.e corner of the terrain and (1,1) is the opposite corner, so they can be easily scaled to terrain size and sampled from heightmap.
    std::vector<glm::vec2> positions {};
    std::list<glm::vec2> active_list {};


    float r = params.r;
    const int k = 30;


    std::default_random_engine gen;
    std::uniform_real_distribution<double> distribution(r, 2*r);

    glm::vec2 x0 {
        static_cast<float>(GetRandomValue(0, INT_MAX)) / static_cast<float>(INT_MAX),
        static_cast<float>(GetRandomValue(0, INT_MAX)) / static_cast<float>(INT_MAX)};

    active_list.push_back(x0);
    positions.push_back(x0);

    while (!active_list.empty() && positions.size() < 1000)
    {
        int index = GetRandomValue(0, active_list.size() - 1);
        auto it = active_list.begin();
        std::advance(it, index);

        glm::vec2 current = *it;

        bool is_alive = false;

        for (int i = 0; i < k; i++)
        {
            int sign_x = (GetRandomValue(0, 1) == 1) ? -1 : 1;
            int sign_y = (GetRandomValue(0, 1) == 1) ? -1 : 1;

            glm::vec2 x_candidat {
                current.x + sign_x * static_cast<float>(distribution(gen)),
                current.y + sign_y * static_cast<float>(distribution(gen))};

            if (x_candidat.x >= 0 && x_candidat.x <= 1 && x_candidat.y >= 0 && x_candidat.y <= 1)
            {
                bool is_active = true;

                for (glm::vec2 x : positions)
                {
                    float dist = glm::distance(x_candidat, x);
                    if ( dist < r)
                    {
                        is_active = false;  
                    }
                }

                if (is_active)
                {
                    active_list.push_front(x_candidat);
                    positions.push_back(x_candidat);
                    is_alive = true;
                }   
            } 
        }
        if(!is_alive) {
            active_list.erase(it);
        }
    }
    return positions;
}

void generateObjectsPositions(AppContext& context) {
    std::vector<glm::vec2> const positions {generate2DPositions(context.pointsGenerationParameters)};

    context.objectPositions.clear();
    context.objectPositions.reserve(positions.size());
    for (glm::vec2 const& p : positions)
    {
        context.objectPositions.emplace_back(
            p.x, // x
            p.y, // y
            // sample height from heightmap for each point (asuming positions are normalized in [0..1] range)
            sampleHeightmap(context, p.x, p.y)
        );
    }
    // TODO(student): extension - filter positions by sampled height range.
}

float sampleHeightmap(AppContext const& context, float u, float v)
{
    if (!context.heightmapImage.data || context.heightmapImage.width <= 0 || context.heightmapImage.height <= 0) return 0.0f;

    int const px = std::clamp(static_cast<int>(u * static_cast<float>(context.heightmapImage.width - 1)), 0, context.heightmapImage.width - 1);
    int const py = std::clamp(static_cast<int>(v * static_cast<float>(context.heightmapImage.height - 1)), 0, context.heightmapImage.height - 1);

    // If the heightmap is in R32 format, we can directly read the height value as a float. 
    if (context.heightmapImage.format == PIXELFORMAT_UNCOMPRESSED_R32)
    {
        float const* heightData = static_cast<float const*>(context.heightmapImage.data);
        int const idx = py * context.heightmapImage.width + px;
        return std::clamp(heightData[idx], 0.0f, 1.0f);
    }

    // Otherwise, we assume it's in a color format and we read the red channel as height (with normalization from [0..255] to [0..1]).
    Color const c = GetImageColor(context.heightmapImage, px, py);
    return static_cast<float>(c.r)/255.0f;
}

void generateHeightmap(AppContext& context) {

    if (context.texture.id > 0) {
        UnloadTexture(context.texture);
        context.texture = {};
    }

    if(context.image.data) {
        UnloadImage(context.image);
        context.image = {};
    }

    if (context.heightmapImage.data) {
        UnloadImage(context.heightmapImage);
        context.heightmapImage = {};
    }

    int const resolution = std::max(1, context.imageGenerationParameters.resolution);

    context.heightmapImage = GenImageFromNoiseFunction<float>(resolution, resolution, PIXELFORMAT_UNCOMPRESSED_R32,
        [&](glm::vec2 const& p)->float {
            // TODO(student): implement stack based noise and island mask
            //float masqueX = cos(2*M_PI*p.x)*0.5f+0.5f;
            // float masqueY = sin(2*M_PI*p.y+1.6f)*0.5f+0.5f;
            float distance  =  glm::distance(p, glm::vec2(0.5, 0.5));
            float masqueDistance = sin(2*M_PI*distance+1.6f)*0.5f+0.5f;
            if (distance < -0.5 || distance > 0.5 ) {
                masqueDistance = 0;
            }
            return masqueDistance*context.changementMasque*(perlinNoiseSeeded(p * context.imageGenerationParameters.noiseScale, context.imageGenerationParameters.noiseSeed) * 0.5f + 0.5f);
        });
    
    //Teste dégradé de couleur
    std::vector<std::vector<glm::vec3>> couleursIleSRGB {
        {
        //Basique
            {0.27f, 0.23f, 1.0f}, // mer foncé 
            {0.36f, 0.94f, 1.0f}, // mer claire
            {1.0f, 0.66f, 0.26f}, // sable 
            {0.93f, 0.84f, 0.69f}, // sable 2
            {0.99f, 0.79f, 0.72f}, // herbe jaune
            {0.51f, 1.0f, 0.47f},  // herbe verte
            {0.53f, 0.53f, 0.53f}, //montagne
            {0.86f, 1.0f, 0.99f} //montagne 2
        },
        {
            //Pastel
            {89.0f/255.0f, 95.0f/255.0f, 255.0f/255.0f}, // mer foncé 
            {187.0f/255.0f, 242.0f/255.0f, 244.0f/255.0f}, // mer claire
            {0.96f, 0.73f, 0.79f}, // sable rose
            {244.0f/255.0f, 238.0f/255.0f, 187.0f/255.0f}, // sable 2
            {179.0f/255.0f, 255.0f/255.0f, 184.0f/255.0f}, // herbe jaune
            {113.0f/255.0f,220.0f/255.0f,114.0f/255.0f},  // herbe verte
            {0.65f, 0.65f, 0.65f}, //montagne 2
            {1.0f, 1.0f, 1.0f}
        },
        {
            //Banquise
            {41.0f/255.0f, 82.0f/255.0f, 136.0f/255.0f}, // mer foncé 
            {59.0f/255.0f, 134.0f/255.0f, 234.0f/255.0f}, // mer claire
            {69.0f/255.0f, 119.0f/255.0f, 184.0f/255.0f}, 
            {156.0f/255.0f, 198.0f/255.0f, 255.0f/255.0f}, 
            {119.0f/255.0f, 163.0f/255.0f, 209.0f/255.0f}, // herbe jaune
            {183.0f/255.0f,226.0f/255.0f,227.0f/255.0f},  // herbe verte
            {153.0f/255.0f, 189.0f/255.0f, 247.0f/255.0f}, //montagne 2
            {1.0f, 1.0f, 1.0f}
        }
    };

    std::vector<glm::vec3> couleursIleLab;
    for (const auto& couleur : couleursIleSRGB[context.changementBiome])
    {
        //Convertie en linéar
        glm::vec3 couleursIleLin{
            srgb_to_linear(couleur.r),
            srgb_to_linear(couleur.g),
            srgb_to_linear(couleur.b)
        };

        //Convertie de Linéar à Lab et les met dans le tableau
        couleursIleLab.push_back(linear_srgb_to_oklab(couleursIleLin));
    }

    // exemple conversion from heightmap to color image

    context.image = TransformImage<float, Color>(context.heightmapImage, [&](float const& v, int const, int const) {
        if (v < 0.1)
        {
                float v2 = v / 0.1f; // Normalise v entre 0 et 1 dans le segment

                glm::vec3 mer1 = couleursIleLab[0];
                glm::vec3 mer2 = couleursIleLab[1];

                glm::vec3 couleurLab = glm::mix(mer1, mer2, v2);

                // Retour vers sRGB
                glm::vec3 couleurLin = oklab_to_linear_srgb(couleurLab);

                glm::vec3 couleurSRGB{
                    linear_to_srgb(couleurLin.r),
                    linear_to_srgb(couleurLin.g),
                    linear_to_srgb(couleurLin.b)
                };

                // Clamp entre 0 et 1 pour éviter l'erreur de couleur chelou avec du rose et du vert
                couleurSRGB = glm::clamp(couleurSRGB, 0.0f, 1.0f);

                //Utilise la structure RayLibe, donc color ressemble à : 
                /*struct Color {
                    unsigned char r;
                    unsigned char g;
                    unsigned char b;
                    unsigned char a;
                };*/

                //On convertit tout en unsigned char pour éviter l'erreur
                return Color{
                    static_cast<unsigned char>(couleurSRGB.r * 255.f), //Pour inverser le return static_cast<float>(c.r)/255.0f;
                    static_cast<unsigned char>(couleurSRGB.g * 255.f),
                    static_cast<unsigned char>(couleurSRGB.b * 255.f),
                    255 //transparence
                };
        }
        else if (v < 0.2f) //sable multicolore
        {
                float v2 = (v / 0.1f) / (0.2f/0.1f); 
                //Pour normaliser on a v2 = (v-a) / (b-a) avec a et b le début des intervalles

                glm::vec3 sable1 = couleursIleLab[2];
                glm::vec3 sable2 = couleursIleLab[3];

                glm::vec3 couleurLab = glm::mix(sable1, sable2, v2);

                // Retour vers sRGB
                glm::vec3 couleurLin = oklab_to_linear_srgb(couleurLab);

                glm::vec3 couleurSRGB{
                    linear_to_srgb(couleurLin.r),
                    linear_to_srgb(couleurLin.g),
                    linear_to_srgb(couleurLin.b)
                };

                // Clamp entre 0 et 1
                couleurSRGB = glm::clamp(couleurSRGB, 0.0f, 1.0f);

                return Color{
                    static_cast<unsigned char>(couleurSRGB.r * 255.f), //Pour inverser le return static_cast<float>(c.r)/255.0f;
                    static_cast<unsigned char>(couleurSRGB.g * 255.f),
                    static_cast<unsigned char>(couleurSRGB.b * 255.f),
                    255 //transparence
                };
        }
        else if (v < 0.4f) //Herbe
        {
                float v2 = (v / 0.2f) / (0.4f/0.2f); 
                //Pour normaliser on a v2 = (v-a) / (b-a) avec a et b le début des intervalles

                glm::vec3 herbe1 = couleursIleLab[4];
                glm::vec3 herbe2 = couleursIleLab[5];

                glm::vec3 couleurLab = glm::mix(herbe1, herbe2, v2);

                // Retour vers sRGB
                glm::vec3 couleurLin = oklab_to_linear_srgb(couleurLab);

                glm::vec3 couleurSRGB{
                    linear_to_srgb(couleurLin.r),
                    linear_to_srgb(couleurLin.g),
                    linear_to_srgb(couleurLin.b)
                };

                // Clamp entre 0 et 1
                couleurSRGB = glm::clamp(couleurSRGB, 0.0f, 1.0f);

                return Color{
                    static_cast<unsigned char>(couleurSRGB.r * 255.f), //Pour inverser le return static_cast<float>(c.r)/255.0f;
                    static_cast<unsigned char>(couleurSRGB.g * 255.f),
                    static_cast<unsigned char>(couleurSRGB.b * 255.f),
                    255 //transparence
                };
        }
        else //montagne
        {
                float v2 = (v / 0.4f) / (1.0f/0.4f); 
                //Pour normaliser on a v2 = (v-a) / (b-a) avec a et b le début des intervalles

                glm::vec3 montagne1 = couleursIleLab[6];
                glm::vec3 montagne2 = couleursIleLab[7];

                glm::vec3 couleurLab = glm::mix(montagne1, montagne2, v2);

                // Retour vers sRGB
                glm::vec3 couleurLin = oklab_to_linear_srgb(couleurLab);

                glm::vec3 couleurSRGB{
                    linear_to_srgb(couleurLin.r),
                    linear_to_srgb(couleurLin.g),
                    linear_to_srgb(couleurLin.b)
                };

                // Clamp entre 0 et 1
                couleurSRGB = glm::clamp(couleurSRGB, 0.0f, 1.0f);

                return Color{
                    static_cast<unsigned char>(couleurSRGB.r * 255.f), //Pour inverser le return static_cast<float>(c.r)/255.0f;
                    static_cast<unsigned char>(couleurSRGB.g * 255.f),
                    static_cast<unsigned char>(couleurSRGB.b * 255.f),
                    255 //transparence
                };
        }
        
    }, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    context.texture = LoadTextureFromImage(context.image);
    if (context.model.meshCount > 0) {
        context.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = context.texture;
    }
}
