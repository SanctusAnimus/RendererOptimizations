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
// ����                 |         |        |        |         |         |
// ����������           |         |        |        |         |         |
// Deferred Shading     |         |        |        |         |         |
// Frustum Culling      |         |        |        |         |         |
// ���������� �������   |         |        |        |         |         |

�������:
    Instancing
    Deferred Shading (+ Light radius approximation)
    UI (ImGui)
    Frustum Culling (��� ���������� ����� � �������, �� ������ ������������� �������)
    Bloom + HDR
    ����� ������
    Skybox

� ������:
    ���������� ������� (https://www.informit.com/articles/article.aspx?p=770639&seqNum=3)
    ����� � ����������� ����������� ����������� (�� ������ �� �������� � ������ ������)
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



���� �����������:
    ������� ��� �� ���������� (TransformComponent, RenderComponent, AnimationComponent, AIComponent?)
    �������� �� �� ���������, �� ����� �� �������� ����� ��� �� ����������� ���������� �����
    �������� �������� ����, �������� �����������, ���� ����� ��� ��������� ������� ��������� �������
    �������� �������� ����� ((��)������������ �����)
    ����������� �������� ������ �������, ������� (std::async)
    ��������� ��� ������
    ����?
    ��������� ����� ���������???
    Compute ������� ��� ������???
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