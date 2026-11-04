module Game.UserInterface.Widget;

import std;
import :TextBox;
import Contract.Input.Shared;

namespace
{
    bool IsContinuationByte(char c) { return (static_cast<unsigned char>(c) & 0xC0) == 0x80; }

    std::size_t CountCharacters(std::string_view s)
    {
        return static_cast<std::size_t>(std::ranges::count_if(s, [](char c) { return !IsContinuationByte(c); }));
    }

    // 앞에서 count 글자까지의 바이트 수 (글자가 모자라면 전체)
    std::size_t BytesForCharacters(std::string_view s, std::size_t count)
    {
        std::size_t bytes = 0;
        while (bytes < s.size() && count > 0)
        {
            do { ++bytes; } while (bytes < s.size() && IsContinuationByte(s[bytes]));
            --count;
        }
        return bytes;
    }

    TextLayout SingleLineLayout() { return { .verticalAlign = TextVerticalAlign::Middle, .wordWrap = false }; }   // 그리기와 측정이 같은 규칙을 써야 캐럿이 글자와 어긋나지 않는다
}

TextBox::TextBox(ITextMeasurer& measurer, Core::Rect localRect, TextBoxSkin skin, std::string text)
    : Widget(localRect), m_measurer(measurer), m_skin(skin), m_text(std::move(text)), m_caret(m_text.size())
{}

void TextBox::SetText(std::string text)
{
    m_text = std::move(text);
    ClampToMaxLength();
    m_textWidth.reset();
    m_caret = m_text.size();
    UpdateScroll();
}

void TextBox::SetMaxLength(std::size_t maxChars)
{
    m_maxLength = maxChars;
    ClampToMaxLength();
    m_textWidth.reset();
    m_caret = std::min(m_caret, m_text.size());
    UpdateScroll();
}

void TextBox::OnTextInput(std::string_view utf8)
{
    if (m_maxLength > 0)
    {
        const std::size_t used = CountCharacters(m_text);
        const std::size_t remaining = used < m_maxLength ? m_maxLength - used : 0;
        utf8 = utf8.substr(0, BytesForCharacters(utf8, remaining));   // 남은 자리만큼만 받는다 (글자 한가운데는 자르지 않는다)
        if (utf8.empty())
            return;
    }

    m_text.insert(m_caret, utf8);
    m_textWidth.reset();
    m_caret += utf8.size();
    UpdateScroll();
    ResetBlink();
    NotifyChanged();   // 콜백이 위젯을 파괴할 수 있으니 마지막에 부른다
}

bool TextBox::OnPointerDown(const PointerEvent& e)
{
    m_caret = CaretFromOffset(e.screenPos.x - (ScreenRect().x + m_skin.padding) + m_scrollX);   // 글자는 m_scrollX만큼 밀려 있다
    UpdateScroll();
    ResetBlink();
    return true;
}

bool TextBox::OnKeyDown(const KeyEvent& e)
{
    bool changed = false;
    bool submitted = false;
    switch (e.key)
    {
    case KeyCode::Enter:
        if (!m_onSubmit)
            return false;   // 받을 곳이 없으면 부모(대화상자 등)가 처리하도록 넘긴다
        submitted = true;
        break;
    case KeyCode::Left:  m_caret = PrevBoundary(m_caret); break;
    case KeyCode::Right: m_caret = NextBoundary(m_caret); break;
    case KeyCode::Home:  m_caret = 0; break;
    case KeyCode::End:   m_caret = m_text.size(); break;
    case KeyCode::Backspace:
        if (m_caret > 0)
        {
            Erase(PrevBoundary(m_caret), m_caret);
            changed = true;
        }
        break;
    case KeyCode::Delete:
        if (m_caret < m_text.size())
        {
            Erase(m_caret, NextBoundary(m_caret));
            changed = true;
        }
        break;
    default:
        return false;
    }

    UpdateScroll();
    ResetBlink();
    if (changed)
        NotifyChanged();   // 콜백이 위젯을 파괴할 수 있으니 마지막에 부른다
    if (submitted)
        NotifySubmit();
    return true;
}

void TextBox::OnTick(float dt)
{
    const float interval = m_skin.caretBlinkInterval;
    if (!m_focused || interval <= 0.f)
        return;

    m_blinkElapsed = std::fmod(m_blinkElapsed + dt, interval * 2.f);   // 보임 interval, 숨김 interval이 번갈아 온다
    m_caretVisible = m_blinkElapsed < interval;
}

void TextBox::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    overlay.DrawImage(m_focused ? m_skin.focused : m_skin.normal, screenRect);

    const Core::Rect inner{
        screenRect.x + m_skin.padding, screenRect.y + m_skin.padding,
        screenRect.width - m_skin.padding * 2.f, screenRect.height - m_skin.padding * 2.f };

    const float textWidth = TextWidth();
    const bool overflows = textWidth + m_skin.caretWidth > inner.width;   // 글자와 캐럿이 글자 영역을 넘으면 밖을 잘라서 그린다
    if (overflows)
        overlay.PushClip(inner);

    if (!m_text.empty())
    {
        const Core::Rect textBounds{ inner.x - m_scrollX, inner.y, std::max(inner.width, textWidth), inner.height };
        overlay.DrawText(m_skin.font, m_skin.mode, m_text, m_skin.fontSize, textBounds, SingleLineLayout(), m_skin.style);
    }

    if (m_focused && m_caretVisible)   // 캐럿 앞 글자들의 폭이 곧 캐럿의 x 위치다
        overlay.DrawImage(m_skin.caret, Core::Rect{ inner.x - m_scrollX + PrefixWidth(m_caret), inner.y, m_skin.caretWidth, inner.height });

    if (overflows)
        overlay.PopClip();
}

float TextBox::TextWidth() const
{
    if (m_text.empty())
        return 0.f;
    if (!m_textWidth)
    {
        const float width = PrefixWidth(m_text.size());
        if (width <= 0.f)
            return width;   // 폰트 미로드 등으로 실패했을 수 있는 측정은 캐시하지 않는다
        m_textWidth = width;
    }
    return *m_textWidth;
}

void TextBox::UpdateScroll()
{
    const float viewWidth = Size().x - m_skin.padding * 2.f;
    const float caretX = PrefixWidth(m_caret);

    if (caretX - m_scrollX < 0.f)
        m_scrollX = caretX;                                          // 캐럿이 왼쪽 밖으로 나감
    else if (caretX + m_skin.caretWidth - m_scrollX > viewWidth)
        m_scrollX = caretX + m_skin.caretWidth - viewWidth;          // 캐럿이 오른쪽 밖으로 나감

    const float maxScroll = std::max(0.f, TextWidth() + m_skin.caretWidth - viewWidth);   // 글자가 줄면 빈 공간이 생기지 않도록 끝에 맞춘다
    m_scrollX = std::clamp(m_scrollX, 0.f, maxScroll);
}

float TextBox::PrefixWidth(std::size_t bytes) const
{
    if (bytes == 0)
        return 0.f;
    return m_measurer.MeasureText(m_skin.font, std::string_view{ m_text }.substr(0, bytes),
        m_skin.fontSize, 0.f, SingleLineLayout()).x;
}

// 글자 경계마다 앞 글자들의 폭을 재서, 누른 자리가 두 경계의 중간보다 왼쪽이면 앞 경계, 아니면 다음 경계로 간다
std::size_t TextBox::CaretFromOffset(float offsetX) const
{
    std::size_t pos = 0;
    float posWidth = 0.f;
    while (pos < m_text.size())
    {
        const std::size_t next = NextBoundary(pos);
        const float nextWidth = PrefixWidth(next);
        if (offsetX < (posWidth + nextWidth) * 0.5f)
            break;
        pos = next;
        posWidth = nextWidth;
    }
    return pos;
}

std::size_t TextBox::PrevBoundary(std::size_t pos) const
{
    if (pos == 0)
        return 0;
    do { --pos; } while (pos > 0 && IsContinuationByte(m_text[pos]));
    return pos;
}

std::size_t TextBox::NextBoundary(std::size_t pos) const
{
    if (pos >= m_text.size())
        return m_text.size();
    do { ++pos; } while (pos < m_text.size() && IsContinuationByte(m_text[pos]));
    return pos;
}

void TextBox::Erase(std::size_t from, std::size_t to)
{
    m_text.erase(from, to - from);
    m_textWidth.reset();
    m_caret = from;
}

// 콜백이 이 위젯을 파괴(Detach)해도 실행 중인 함수 객체와 글자는 살아 있어야 해서 복사해서 부른다
void TextBox::NotifyChanged()
{
    if (!m_onChanged)
        return;
    const auto callback = m_onChanged;
    const std::string text = m_text;
    callback(text);
}

void TextBox::NotifySubmit()
{
    if (!m_onSubmit)
        return;
    const auto callback = m_onSubmit;
    const std::string text = m_text;
    callback(text);
}

void TextBox::ClampToMaxLength()
{
    if (m_maxLength > 0)
        m_text.resize(BytesForCharacters(m_text, m_maxLength));
}
