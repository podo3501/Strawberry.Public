export module UIVisualizer.KeyQuery;

import Contract.Input.Service;
import Contract.Input.Shared;

// 모드 중 단축키(되돌리기, 미세 이동)를 판정할 때 함께 쓰는 키 조회
// 왼쪽과 오른쪽이 따로 있는 Ctrl, Shift는 둘 중 하나만 눌려도 눌린 것으로 본다.
export namespace KeyQuery
{
    inline bool IsDown(const IInputService& input, KeyCode left, KeyCode right)
    {
        return input.GetKey(left).IsDown() || input.GetKey(right).IsDown();
    }

    inline bool IsCtrlDown(const IInputService& input) { return IsDown(input, KeyCode::LeftCtrl, KeyCode::RightCtrl); }
    inline bool IsShiftDown(const IInputService& input) { return IsDown(input, KeyCode::LeftShift, KeyCode::RightShift); }

    // 눌린 프레임이거나 누르고 있어서 반복이 일어난 프레임 (키를 누르고 있으면 계속 동작해야 하는 단축키용)
    inline bool IsTriggered(const IInputService& input, KeyCode key)
    {
        const ButtonState& state = input.GetKey(key);
        return state.IsPressed() || state.IsRepeated();
    }
}
