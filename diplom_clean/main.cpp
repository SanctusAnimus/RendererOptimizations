#include "Application.h"

int main()
{
    Application app;
    app.Init();
    return 0;
}

/*

Сделано:
    Instancing
    Deferred Shading (+ Light radius approximation)
    UI (ImGui)
    Frustum Culling (для источников света и моделей, точечный)
    Bloom + HDR
    Имгуи логгер (по возможности)

В планах:
    Компрессия текстур (https://www.informit.com/articles/article.aspx?p=770639&seqNum=3)
    EnTT
    абстракция буферов
    RAII всего (для переключения сцен!)

Было бы прикольно иметь:
    SSAO
    Global Illumination
    Shadow mapping
    Reflections
    Ну и оптимизации к этому всему

Упомянуть в возможной дальнейшей работе
    Light Volumes
    Anti-aliasing (MSAA и т.д.)
    Octree sorting

Проиграно:
    Тесселляция (для треугольников работает коряво, а квады не отрисовываются вообще, мб надо больше точек опорных, мб выключить элементы)
    
Включить выключенное:
    Parallax Mapping
    Skybox


*/