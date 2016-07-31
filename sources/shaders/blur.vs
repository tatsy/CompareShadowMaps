#version 330

layout(location = 0) in vec3 v_position;

out vec2 f_mapUv;

void main(void) {
    gl_Position = vec4(v_position, 1.0);
    f_mapUv = v_position.xy * 0.5 + 0.5;
}
