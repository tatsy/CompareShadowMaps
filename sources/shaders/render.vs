#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

out vec3 f_position;
out vec3 f_normal;
out vec4 f_posLightSpace;

uniform mat4 u_mvpMat;
uniform mat4 u_depthMvp;

void main(void) {
    gl_Position = u_mvpMat * vec4(v_position, 1.0);

    f_position = v_position;
    f_normal = v_normal;

    f_posLightSpace = u_depthMvp * vec4(v_position, 1.0);
}
