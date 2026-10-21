export module Graphics.SceneObject:CameraInputMapper;

import std;
import :CameraInputState;
import Contract.Input.Service;

export CameraInputState BuildCameraInput(const IInputService& input)
{
    CameraInputState state;

	auto axis = [&](KeyCode positive, KeyCode negative)
		{
			return (input.GetKey(positive).IsHeld() ? 1.0f : 0.0f)
				- (input.GetKey(negative).IsHeld() ? 1.0f : 0.0f);
		};

	state.moveForward = axis(KeyCode::W, KeyCode::S);
	state.moveRight = axis(KeyCode::D, KeyCode::A);
	state.moveUp = axis(KeyCode::E, KeyCode::Q);
	state.fastMove = input.GetKey(KeyCode::LeftShift).IsHeld();

    if (input.GetMouseButton(MouseButton::Left).IsHeld())
    {
        auto mouse = input.GetMouseState();
        int deltaX = mouse.position.x - mouse.prevPosition.x;
        int deltaY = mouse.position.y - mouse.prevPosition.y;

        state.yawDelta = static_cast<float>(deltaX);
        state.pitchDelta = static_cast<float>(-deltaY);
    }

    return state;
}