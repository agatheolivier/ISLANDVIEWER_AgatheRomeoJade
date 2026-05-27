#include "raylib.h"
#include "app.hpp"

Model Model3DChoix(AppContext context){
    if (context.changementBiome == 0){
        return LoadModel("../../resources/poisson-steve/source/steve.glb");
    }
    else if (context.changementBiome == 1){
        return LoadModel("../../resources/fat_rainbow_cat.glb");
    }
    else if (context.changementBiome == 2){
        return LoadModel("../../resources/pingu.glb");
    }
    else if (context.changementBiome == 3){
        return LoadModel("../../resources/peak-bingbong-model.glb");
    }
    else {
        return LoadModel("../../resources/poisson-steve/source/steve.glb");
    }
}