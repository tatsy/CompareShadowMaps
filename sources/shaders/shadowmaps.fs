#version 330

#define NORMAL_SM      0
#define PCF            1
#define VARIANCE_SM    2
#define CONVOLUTION_SM 3
#define EXPONENTIAL_SM 4

in vec4 f_posScreen;

out vec4 outColor;

uniform float u_esmCoeff;
uniform int u_smType;

void main(void) {
    float depth = f_posScreen.z / f_posScreen.w;
    if (u_smType == NORMAL_SM ||
        u_smType == PCF) {
        // Do nothing
    } else if (u_smType == EXPONENTIAL_SM) {
       depth = exp(u_esmCoeff * depth);
    }
    outColor = vec4(depth, depth, depth, 1.0);
}
