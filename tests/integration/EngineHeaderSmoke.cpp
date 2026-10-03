#include <math/Math.hpp>

int main() {
    ursa::Camera camera;
    camera.position = {0.0f, 12.0f, 18.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
    const ursa::Mat4 viewProjection = camera.viewProjection(16.0f / 9.0f);
    const ursa::Mat4 model = ursa::Transform{{2.0f, 0.5f, 3.0f}, {1.0f, 1.0f, 1.0f}}.matrix();
    const ursa::Mat4 clipFromModel = viewProjection * model;
    return clipFromModel.at(3, 3) > 0.0f ? 0 : 1;
}
