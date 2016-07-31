#version 330

in vec3 f_position;
in vec3 f_normal;
in vec4 f_posLightSpace;

out vec4 outColor;

uniform vec3 u_lightPos;
uniform vec3 u_cameraPos;
uniform float u_esmCoeff;
uniform float u_darkness;

uniform sampler2D u_depthMap;

vec3 colorDiff = vec3(0.5, 0.5, 0.5);
vec3 colorSpec = vec3(0.5, 0.5, 0.5);

void main(void) {
    vec3 V = normalize(u_cameraPos - f_position);
    vec3 L = normalize(u_lightPos - f_position);
    vec3 N = normalize(f_normal);
    vec3 H = normalize(V + L);

    float ndotl = dot(N, L);
    float ndoth = dot(N, H);

    float rhoDiff = max(0.0, ndotl);
    float rhoSpec = pow(max(0.0, ndoth), 8.0);

    vec2 depthUv = (f_posLightSpace.xy / f_posLightSpace.w) * 0.5 + 0.5;
    float dvalue = exp(-u_esmCoeff * (f_posLightSpace.z / f_posLightSpace.w));
    float zvalue = texture(u_depthMap, depthUv).x;
    float bias = tan(acos(ndotl)) * 1.0e-3;
    float visibility = 1.0 - u_darkness * (1.0 - clamp(dvalue * zvalue, 0.0, 1.0));

    outColor.rgb = (colorDiff * rhoDiff + colorSpec * rhoSpec) * visibility;
    outColor.a   = 1.0;
}
