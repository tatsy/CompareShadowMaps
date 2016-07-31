#version 330

layout(location = 0) in vec3 v_position;

out vec4 f_posScreen;

uniform mat4 u_mvpMat;

void main(void) {
    gl_Position = u_mvpMat * vec4(v_position, 1.0);
    f_posScreen = gl_Position;
}
