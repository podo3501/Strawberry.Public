//module;
//
//#include <d3d12.h>
//
//export module Runtime.Render.Resource:TransientMesh;
//
//import :Mesh;
//import Runtime.Render.Allocator;
//
//export class TransientMeshResource : public MeshResource
//{
//public:
//    TransientMeshResource() = default;
//    virtual ~TransientMeshResource() override = default;
//
//    virtual UINT GetVertexHeapIndex() const noexcept override { return m_vertexHeapIndex; }
//    virtual UINT GetIndexHeapIndex() const noexcept override { return m_indexHeapIndex; }
//    virtual UINT GetVertexCount() const noexcept override { return m_vertexCount; }
//    virtual UINT GetIndexCount() const noexcept override { return m_indexCount; }
//
//    void Initialize(
//        const UploadAllocation& vertex,
//        const UploadAllocation& index,
//        UINT vertexCount,
//        UINT indexCount,
//        UINT vertexHeapIndex,
//        UINT indexHeapIndex) noexcept
//    {
//        m_vertexAllocation = vertex;
//        m_indexAllocation = index;
//
//        m_vertexCount = vertexCount;
//        m_indexCount = indexCount;
//
//        m_vertexHeapIndex = vertexHeapIndex;
//        m_indexHeapIndex = indexHeapIndex;
//
//        MarkReady();
//    }
//
//private:
//    UploadAllocation m_vertexAllocation{};
//    UploadAllocation m_indexAllocation{};
//
//    UINT m_vertexHeapIndex{};
//    UINT m_indexHeapIndex{};
//
//    UINT m_vertexCount{};
//    UINT m_indexCount{};
//};