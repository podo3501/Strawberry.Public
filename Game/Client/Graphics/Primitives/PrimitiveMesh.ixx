export module Graphics.Primitives:Mesh;

import std;
import Client.Render.Definition;
import Client.Asset.Data;
import Core.Math;

export class PrimitiveMesh
{
public:
    static std::shared_ptr<MeshAsset> CreateSphere(
        float radius = 0.5f,
        std::uint32_t sliceCount = 32,
        std::uint32_t stackCount = 32
    );

    static std::shared_ptr<MeshAsset> CreateCube(float size = 1.f);

    static std::shared_ptr<MeshAsset> CreateTorus(
        float radius = 1.0f,      // 전체 반지름
        float tubeRadius = 0.3f,  // 도넛 두께
        std::uint32_t radialSegments = 32,
        std::uint32_t tubularSegments = 16
    );

    static std::shared_ptr<MeshAsset> CreateGrid(
        float cellSize = 1.0f,
        std::uint32_t halfExtent = 10,
        const Core::Color& color = { 0.5f, 0.5f, 0.5f, 1.f }
    );
};

std::shared_ptr<MeshAsset> PrimitiveMesh::CreateSphere(float radius, std::uint32_t sliceCount, std::uint32_t stackCount)
{
    auto mesh = std::make_shared<MeshAsset>();
    mesh->format = VertexFormat::Mesh;

    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;

    // sliceCount: 세로 분할 수 (경도, 축을 중심으로 도는 회전 분할)
    // stackCount: 가로 분할 수 (위도, 북극에서 남극으로 내려가는 층 분할)

    std::uint32_t vertexCount = (stackCount + 1) * (sliceCount + 1);
    std::uint32_t indexCount = stackCount * sliceCount * 6;
    vertices.reserve(vertexCount);
    indices.reserve(indexCount);

    constexpr float PI = 3.14159265359f;
    constexpr float TWO_PI = 6.28318530718f;

    // 1. Vertex (정점) 생성
    for (std::uint32_t i = 0; i <= stackCount; ++i)
    {
        // 북극(0)에서 남극(PI)까지의 각도 (위도)
        float phi = (static_cast<float>(i) / stackCount) * PI;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (std::uint32_t j = 0; j <= sliceCount; ++j)
        {
            // 한 바퀴 회전하는 각도 (경도)
            float theta = (static_cast<float>(j) / sliceCount) * TWO_PI;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            MeshVertex vert;

            // 법선 벡터 (Normal) : 원점에서 정점을 향하는 방향 단위 벡터
            vert.normal = {
                sinPhi * cosTheta,
                cosPhi, // Y축을 위(Up)로 잡는 일반적인 3D 공간 기준
                sinPhi * sinTheta
            };

            // 위치 좌표 (Position) : 법선 벡터에 반지름을 곱함
            vert.position = vert.normal * radius;

            // UV 좌표
            vert.uv = {
                static_cast<float>(j) / sliceCount,
                static_cast<float>(i) / stackCount
            };

            // 접선 벡터 (Tangent) : U(경도 theta)가 증가하는 방향으로의 편미분 벡터
            vert.tangent = {
                -sinPhi * sinTheta,
                0.0f,
                sinPhi * cosTheta
            };

            // Tangent 정규화
            vert.tangent.NormalizeOr(Core::Vector3::Right());

            vertices.push_back(vert);
        }
    }

    // 2. Index (인덱스) 생성 - DirectX 표준 (CW 시계 방향)
    std::uint32_t ringVertexCount = sliceCount + 1;

    for (std::uint32_t i = 0; i < stackCount; ++i)
    {
        for (std::uint32_t j = 0; j < sliceCount; ++j)
        {
            std::uint32_t current_row_left = i * ringVertexCount + j;
            std::uint32_t current_row_right = current_row_left + 1;
            std::uint32_t next_row_left = (i + 1) * ringVertexCount + j;
            std::uint32_t next_row_right = next_row_left + 1;

            // 사각형 면을 2개의 삼각형으로 쪼개서 CW 순서로 조립
            indices.push_back(current_row_left);
            indices.push_back(current_row_right);
            indices.push_back(next_row_left);

            indices.push_back(current_row_right);
            indices.push_back(next_row_right);
            indices.push_back(next_row_left);
        }
    }

    mesh->SetVertices(vertices);
    mesh->indices = std::move(indices);

    return mesh;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::CreateCube(float size)
{
    auto mesh = std::make_shared<MeshAsset>();
    mesh->format = VertexFormat::Mesh;

    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;

    float h = size * 0.5f;

    struct FaceInfo
    {
        Core::Vector3 normal;
        Core::Vector3 tangent;
        Core::Vector3 points[4];
    };

    FaceInfo faces[6] =
    {
        // 1. 앞면 (Front Face: +Z 방향)
        {
            {  0.f,  0.f,  1.f },
            {  1.f,  0.f,  0.f },
            {
                { -h, -h,  h },
                { -h,  h,  h },
                {  h,  h,  h },
                {  h, -h,  h }
            }
        },

        // 2. 뒷면 (Back Face: -Z 방향)
        {
            {  0.f,  0.f, -1.f },
            { -1.f,  0.f,  0.f },
            {
                {  h, -h, -h },
                {  h,  h, -h },
                { -h,  h, -h },
                { -h, -h, -h }
            }
        },

        // 3. 윗면 (Top Face: +Y 방향)
        {
            {  0.f,  1.f,  0.f },
            {  1.f,  0.f,  0.f },
            {
                { -h,  h,  h },
                { -h,  h, -h },
                {  h,  h, -h },
                {  h,  h,  h }
            }
        },

        // 4. 아랫면 (Bottom Face: -Y 방향)
        {
            {  0.f, -1.f,  0.f },
            { -1.f,  0.f,  0.f },
            {
                { -h, -h, -h },
                { -h, -h,  h },
                {  h, -h,  h },
                {  h, -h, -h }
            }
        },

        // 5. 왼쪽면 (Left Face: -X 방향)
        {
            { -1.f,  0.f,  0.f },
            {  0.f,  0.f,  1.f },
            {
                { -h, -h, -h },
                { -h,  h, -h },
                { -h,  h,  h },
                { -h, -h,  h }
            }
        },

        // 6. 오른쪽면 (Right Face: +X 방향)
        {
            {  1.f,  0.f,  0.f },
            {  0.f,  0.f, -1.f },
            {
                {  h, -h,  h },
                {  h,  h,  h },
                {  h,  h, -h },
                {  h, -h, -h }
            }
        }
    };

    vertices.reserve(24);
    indices.reserve(36);

    Core::Vector2 uvs[4] =
    {
        { 0.f, 1.f }, // 좌측 하단
        { 0.f, 0.f }, // 좌측 상단
        { 1.f, 0.f }, // 우측 상단
        { 1.f, 1.f }  // 우측 하단
    };

    for (std::uint32_t f = 0; f < 6; ++f)
    {
        auto startIndex = static_cast<std::uint32_t>(vertices.size());

        for (std::uint32_t i = 0; i < 4; ++i)
        {
            vertices.push_back({
                faces[f].points[i],
                faces[f].normal,
                uvs[i],
                faces[f].tangent
                });
        }

        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 1);

        indices.push_back(startIndex + 0);
        indices.push_back(startIndex + 3);
        indices.push_back(startIndex + 2);
    }

    mesh->SetVertices(vertices);
    mesh->indices = std::move(indices);

    return mesh;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::CreateTorus(
    float radius,
    float tubeRadius,
    std::uint32_t radialSegments,
    std::uint32_t tubularSegments)
{
    auto mesh = std::make_shared<MeshAsset>();
    mesh->format = VertexFormat::Mesh;

    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;

    vertices.reserve((radialSegments + 1) * (tubularSegments + 1));

    constexpr float TWO_PI = 6.28318530718f;

    // 1. Vertex 생성
    for (std::uint32_t j = 0; j <= radialSegments; ++j)
    {
        float v = static_cast<float>(j) / radialSegments;
        float phi = v * TWO_PI;

        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        for (std::uint32_t i = 0; i <= tubularSegments; ++i)
        {
            float u = static_cast<float>(i) / tubularSegments;
            float theta = u * TWO_PI;

            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            // 중심 원(Major Circle)의 중심
            Core::Vector3 center{
                radius * cosPhi,
                radius * sinPhi,
                0.0f
            };

            MeshVertex vert;

            // Position
            vert.position = {
                (radius + tubeRadius * cosTheta) * cosPhi,
                (radius + tubeRadius * cosTheta) * sinPhi,
                tubeRadius * sinTheta
            };

            // Normal
            vert.normal = (vert.position - center).Normalized();

            // Tangent (U(theta) 방향)
            vert.tangent = {
                -sinTheta * cosPhi,
                -sinTheta * sinPhi,
                    cosTheta
            };
            vert.tangent.NormalizeOr(Core::Vector3::Right());

            // UV
            vert.uv = { u, v };

            vertices.push_back(vert);
        }
    }

    // 2. Index 생성
    const std::uint32_t ring = tubularSegments + 1;

    for (std::uint32_t j = 0; j < radialSegments; ++j)
    {
        for (std::uint32_t i = 0; i < tubularSegments; ++i)
        {
            std::uint32_t a = j * ring + i;
            std::uint32_t b = (j + 1) * ring + i;
            std::uint32_t c = (j + 1) * ring + (i + 1);
            std::uint32_t d = j * ring + (i + 1);

            // triangle 1
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(d);

            // triangle 2
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }

    mesh->SetVertices(vertices);
    mesh->indices = std::move(indices);

    return mesh;
}

std::shared_ptr<MeshAsset> PrimitiveMesh::CreateGrid(
    float cellSize,
    std::uint32_t halfExtent,
    const Core::Color& color)
{
    auto mesh = std::make_shared<MeshAsset>();
    mesh->format = VertexFormat::Grid;

    std::vector<GridVertex> vertices;
    std::vector<std::uint32_t> indices;

    const std::uint32_t lineCount = halfExtent * 2 + 1;

    vertices.reserve(lineCount * 4); // 라인당 정점 2개
    indices.reserve(lineCount * 4);  // 라인당 인덱스 2개

    std::uint32_t index = 0;

    const float min = -static_cast<float>(halfExtent) * cellSize;
    const float max = static_cast<float>(halfExtent) * cellSize;

    // Z 방향 라인 (세로줄)
    for (std::uint32_t i = 0; i < lineCount; ++i)
    {
        float x = min + static_cast<float>(i) * cellSize;

        vertices.push_back({
            { x, 0.0f, min },
            color
            });

        vertices.push_back({
            { x, 0.0f, max },
            color
            });

        indices.push_back(index++);
        indices.push_back(index++);
    }

    // X 방향 라인 (가로줄)
    for (std::uint32_t i = 0; i < lineCount; ++i)
    {
        float z = min + static_cast<float>(i) * cellSize;

        vertices.push_back({
            { min, 0.0f, z },
            color
            });

        vertices.push_back({
            { max, 0.0f, z },
            color
            });

        indices.push_back(index++);
        indices.push_back(index++);
    }

    mesh->SetVertices(vertices);
    mesh->indices = std::move(indices);

    return mesh;
}