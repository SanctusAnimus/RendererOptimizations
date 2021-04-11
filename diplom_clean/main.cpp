#include "Application.h"

int main()
{
    Application app;
    app.Init();
    return 0;
}

/*

//----------------------------------------------------------------------------------------------
//  Models * Light Sources
//                      |    1    |   16   |   64   |   256   |   512   |   
// База                 |         |        |        |         |         |
// Инстансинг           |         |        |        |         |         |
// Deferred Shading     |         |        |        |         |         |
// Frustum Culling      |         |        |        |         |         |
// Компрессия текстур   |         |        |        |         |         |

Сделано:
    Instancing
    Deferred Shading (+ Light radius approximation)
    UI (ImGui)
    Frustum Culling (для источников света и моделей, по сферам определенного радиуса)
    Bloom + HDR
    Имгуи логгер
    Skybox

В планах:
    Компрессия текстур (https://www.informit.com/articles/article.aspx?p=770639&seqNum=3)
    Сцены с постепенным добавлением оптимизаций (от ничего до нынешней с полным фаршем)
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



План архитектуры:
    Разбить все на компоненты (TransformComponent, RenderComponent, AnimationComponent, AIComponent?)
    Вытащить их из рендерера, он будет их собирать через вью по компонентам конкретной сцены
    Добавить иерархию сцен, редактор компонентов, скан папок для получения списков доступных ассетов
    Добавить манифест сцены ((де)сериализация сцены)
    Асинхронная загрузка файлов моделей, текстур (std::async)
    Устранить все утечки
    Звук?
    Отдельный поток отрисовки???
    Compute шейдеры для частиц???
*/


/*
Raw Scene (no optimizations) [ready to be implemented]
    |
Instancing Scene [ready to be implemented]
    |
Deferred Scene [ready to be implemented]
    | 
Frustum Scene [ready to be implemented]
    |
Compression Scene [ready to be implemented]
    |
Starting Scene (everything and a bit extra) [DONE]
    |
Loadable Scene (previous with (de)serialization support) [TODO]
*/