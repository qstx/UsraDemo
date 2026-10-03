#include <Math/Math.hpp>

int main()
{
	Ursa::Camera camera;
	camera.position = {0.0f, 12.0f, 18.0f};
	camera.target = {0.0f, 0.0f, 0.0f};
	const Ursa::Mat4 viewProjection = camera.ViewProjection(16.0f / 9.0f);
	const Ursa::Mat4 model = Ursa::Transform{{2.0f, 0.5f, 3.0f}, {1.0f, 1.0f, 1.0f}}.Matrix();
	const Ursa::Mat4 clipFromModel = viewProjection * model;
	return clipFromModel.At(3, 3) > 0.0f ? 0 : 1;
}
