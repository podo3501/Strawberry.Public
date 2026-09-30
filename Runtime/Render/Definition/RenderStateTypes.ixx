export module Runtime.Render.Definition:RenderStateTypes;

export enum class FillMode
{
    Solid,
    Wireframe
};

export enum class CullMode
{
    None,
    Front,
    Back
};

export enum class PrimitiveTopologyType
{
    Triangle,
    Line
};