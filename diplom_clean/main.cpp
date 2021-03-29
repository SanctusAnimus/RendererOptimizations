#include "Application.h"

int main()
{
    Application app;
    app.Init();
    return 0;
}

/*

�������:
    Instancing (��� ������)
    Instancing (��� �������)
    Deferred Shading
    UI (ImGui)
    Frustum Culling (��� ���������� ����� � �������, ��������(���������))

� ������:
    SSAO (������� HDAO, HBAO)
    Global Illumination
    Shadow mapping
    Reflections
    �� � ����������� � ����� �����

��������� � ��������� ���������� ������
    Light Volumes
    Anti-aliasing (MSAA)

���������:
    ����������� (��� ������������� �������� ������, � ����� �� �������������� ������, �� ���� ������ ����� �������, �� ��������� ��������)
    Bloom + HDR (��� ����� ��� � deferred ��������, ���� ������� ��������, ���� ����� ������� ����:
        1. Bind to multisampled framebuffer.
        2. Render game as normal.
        3. Blit multisampled framebuffer to normal framebuffer with texture attachment. (deferred ���� ������� �����, �� �������, � ���-�� ��� ����� �� ����������)
        4. Unbind framebuffer (use default framebuffer).
        5. Use color buffer texture from normal framebuffer in postprocessing shader.
        6. Render quad of screen-size as output of postprocessing shader.
    )

�������� �����������:
    Parallax Mapping
    Skybox


*/