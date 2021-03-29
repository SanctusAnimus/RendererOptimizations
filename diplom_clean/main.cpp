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

В планах:
    SSAO (чекнуть HDAO, HBAO)
    Global Illumination
    Shadow mapping
    Reflections
    Ну и оптимизации к этому всему

Упомянуть в возможной дальнейшей работе
    Light Volumes
    Anti-aliasing (MSAA)

Проиграно:
    Тесселляция (для треугольников работает коряво, а квады не отрисовываются вообще, мб надо больше точек опорных, мб выключить элементы)
    Bloom + HDR (хуй пойми как в deferred засунуть, надо немного подумать, есть вроде базовые шаги:
        1. Bind to multisampled framebuffer.
        2. Render game as normal.
        3. Blit multisampled framebuffer to normal framebuffer with texture attachment. (deferred тоже блитует буфер, но глубины, и где-то тут важно не обосраться)
        4. Unbind framebuffer (use default framebuffer).
        5. Use color buffer texture from normal framebuffer in postprocessing shader.
        6. Render quad of screen-size as output of postprocessing shader.
    )

Включить выключенное:
    Parallax Mapping
    Skybox


*/