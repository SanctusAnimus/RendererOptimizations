#include "Application.h"

int main()
{
    Application app;
    app.Init();
    return 0;
}

/*

�������:
    Instancing
    Deferred Shading (+ Light radius approximation)
    UI (ImGui)
    Frustum Culling (��� ���������� ����� � �������, ��������)
    Bloom + HDR
    ����� ������ (�� �����������)

� ������:
    ���������� ������� (https://www.informit.com/articles/article.aspx?p=770639&seqNum=3)
    EnTT
    ���������� �������
    RAII ����� (��� ������������ ����!)

���� �� ��������� �����:
    SSAO
    Global Illumination
    Shadow mapping
    Reflections
    �� � ����������� � ����� �����

��������� � ��������� ���������� ������
    Light Volumes
    Anti-aliasing (MSAA � �.�.)
    Octree sorting

���������:
    ����������� (��� ������������� �������� ������, � ����� �� �������������� ������, �� ���� ������ ����� �������, �� ��������� ��������)
    
�������� �����������:
    Parallax Mapping
    Skybox


*/