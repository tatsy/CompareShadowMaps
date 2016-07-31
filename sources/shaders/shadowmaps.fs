#version 330

in vec4 f_posScreen;

out vec4 outColor;

uniform float u_esmCoeff;

void main(void) {
    float depth = exp(u_esmCoeff * (f_posScreen.z / f_posScreen.w));
    outColor = vec4(depth, depth, depth, 1.0);
}
