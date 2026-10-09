export module Contract.Asset.Data:TextureTypes;

export enum class PixelFormat
{
    Unknown,
    RGB8,
    RGBA8,
    R11G11B10F,
    RGBA16F,    // cmgen이 float16으로 뽑을 때
    RGB9E5,     // cmgen RGBM 대신 shared-exponent 옵션 쓸 때
    BC6H_UF16,  // 압축 HDR 옵션 쓸 때
};

export enum class ColorSpace
{
    SRGB,
    Linear,
    Count
};
