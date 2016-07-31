#version 330

in vec2 f_mapUv;

out vec4 outColor;

uniform sampler2D u_depthMap;
uniform float u_pixelSize;
uniform int u_isHorizontal;

float EPS = 1.0e-8;

float w[10] = float[]( 0.00879839,  0.02714932,  0.06510809,  0.1216189,  0.17702363,
                       0.17702363,  0.1216189,  0.06510809,  0.02714932,  0.00879839 );
float o[10] = float[]( -1.0, -0.8, -0.6, -0.4, -0.2, 0.2, 0.4, 0.6, 0.8, 1.0 );

vec3 blurPS(sampler2D texmap, vec2 texCoord, vec2 dir) {
    vec3 colorM = texture(texmap, texCoord).rgb;
    vec2 finalStep = u_pixelSize * dir;

    vec3 ret = colorM * 0.20060332;
    for (int i = 0; i < 10; i++) {
        vec2 offset = texCoord + o[i] * finalStep;
        vec3 color = texture(texmap, offset).rgb;
        ret += w[i] * color;
    }

    return ret;
}

void main(void) {
    vec2 dir = u_isHorizontal * vec2(1.0, 0.0) +
               (1.0 - u_isHorizontal) * vec2(0.0, 1.0);
    outColor.rgb = blurPS(u_depthMap, f_mapUv, dir);
    outColor.a   = 1.0;
}
