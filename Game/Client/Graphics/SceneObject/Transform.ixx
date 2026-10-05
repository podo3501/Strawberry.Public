export module Graphics.SceneObject:Transform;

import std;
import Core.Math;

export class Transform
{
public:
    Transform()
        : position(0, 0, 0)
        , rotation(0, 0, 0)
        , scale(1, 1, 1)
        , m_dirty(true)
    {
    }

    Transform(
        const Core::Vector3& pos,
        const Core::Vector3& rot,
        const Core::Vector3& scale)
        : position(pos)
        , rotation(rot)
        , scale(scale)
        , m_dirty(true)
    {
    }

    void SetPosition(const Core::Vector3& p)
    {
        position = p;
        m_dirty = true;
    }

    void SetRotation(const Core::Vector3& r)
    {
        rotation = r;
        m_dirty = true;
    }

    void SetScale(const Core::Vector3& s)
    {
        scale = s;
        m_dirty = true;
    }

    void SetDirty()
    {
        m_dirty = true;
    }

    const Core::Matrix& GetMatrix() const
    {
        if (m_dirty)
        {
            RebuildMatrix();
        }
        return m_worldMatrix;
    }

private:
    void RebuildMatrix() const
    {
        Core::Matrix T = Core::Matrix::Translation(position.x, position.y, position.z);
        Core::Matrix Rx = Core::Matrix::RotationX(rotation.x);
        Core::Matrix Ry = Core::Matrix::RotationY(rotation.y);
        Core::Matrix Rz = Core::Matrix::RotationZ(rotation.z);
        Core::Matrix S = Core::Matrix::Scale(scale.x, scale.y, scale.z);

        m_worldMatrix = S * Rx * Ry * Rz * T;
        m_dirty = false;
    }

    mutable Core::Matrix m_worldMatrix;
    mutable bool m_dirty{ true };

    Core::Vector3 position;
    Core::Vector3 rotation; // radians (x,y,z)
    Core::Vector3 scale;
};