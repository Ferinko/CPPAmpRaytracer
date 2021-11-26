#define USE_AMP

#include "Scene.hpp"
#include "SampleScenes.hpp"
#include "ComputerInfo.hpp"

#include <iosfwd>

const enum Extensions { bmp };
const char* extensions[] = {".bmp"};

using namespace Smurf;
using namespace Smurf::Utils;

int main() {
    printPCInfo(std::wcout);
    printRayTraceInfo(std::wcout);
    auto scene = Scenes::constructSceneGPU0();
    //scene->renderScene(scene->rayTraceScene());
    scene->renderScene(scene->rayTraceSceneGPU());
    system("PAUSE");
}