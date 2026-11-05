export module UserInterface.Widget:Button;

import std;
import :Pressable;
import :SlicedBrush;
import :WidgetResources;
import Contract.Render.IOverlayView;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export class Button : public Core::TypeNode<Button, Pressable>
{
public:
    explicit Button(Core::Rect localRect, ButtonBrushes brushes = {})
        : TypeNode(localRect), m_brushes(brushes) {}

    void SetOnClick(std::function<void()> onClick) { m_onClick = std::move(onClick); }

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Button.cpp
    void OnClick(const PointerEvent&) override { if (m_onClick) m_onClick(); }

    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawImage(Pick(m_brushes), screenRect);
    }

private:
    ButtonBrushes m_brushes;
    std::function<void()> m_onClick;
};