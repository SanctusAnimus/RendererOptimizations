#include "Application.h"

int main()
{
    Application app;
    app.Init();
    return 0;
}

/*

Сделано:
    Instancing (для квадов)
    Instancing (для моделей)
    Deferred Shading
    UI (ImGui)
    Frustum Culling (для источников света и моделей, точечный(дерьмовый))
    Bloom + HDR

В планах:
    Имгуи логгер (по возможности)
    Компрессия текстур (https://www.informit.com/articles/article.aspx?p=770639&seqNum=3)
    EnTT
    SSAO (чекнуть HDAO, HBAO)
    Global Illumination
    Shadow mapping
    Reflections
    Ну и оптимизации к этому всему

Упомянуть в возможной дальнейшей работе
    Light Volumes
    Anti-aliasing (MSAA и т.д.)

Проиграно:
    Тесселляция (для треугольников работает коряво, а квады не отрисовываются вообще, мб надо больше точек опорных, мб выключить элементы)
    

Включить выключенное:
    Parallax Mapping
    Skybox


*/