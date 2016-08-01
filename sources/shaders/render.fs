#version 330

#define NORMAL_SM      0
#define PCF            1
#define VARIANCE_SM    2
#define CONVOLUTION_SM 3
#define EXPONENTIAL_SM 4

in vec3 f_position;
in vec3 f_normal;
in vec4 f_posLightSpace;

out vec4 outColor;

uniform vec3 u_lightPos;
uniform vec3 u_cameraPos;
uniform float u_esmCoeff;
uniform float u_kernelSize;
uniform float u_darkness;
uniform int u_smType;

uniform sampler2D u_depthMap;

vec3 colorDiff = vec3(0.5, 0.5, 0.5);
vec3 colorSpec = vec3(0.5, 0.5, 0.5);

float shadowMaps() {
    vec3 N = normalize(f_normal);
    vec3 L = normalize(u_lightPos - f_position);
    float ndotl = max(0.0, dot(N, L));
    float bias = tan(acos(ndotl)) * 1.0e-5;

    vec2 depthUv = (f_posLightSpace.xy / f_posLightSpace.w) * 0.5 + 0.5;
    float dvalue = (f_posLightSpace.z / f_posLightSpace.w);
    float zvalue = texture(u_depthMap, depthUv).x;

    float visibility = 1.0;
    if (zvalue < dvalue - bias) {
        visibility = 0.0;
    }
    return visibility;
}

float shadowMapsPCF() {
    const int nSamples = 32;
    vec2 samples[] = vec2[] (
        vec2(-0.613392, 0.617481),
        vec2(0.170019, -0.040254),
        vec2(-0.299417, 0.791925),
        vec2(0.645680, 0.493210),
        vec2(-0.651784, 0.717887),
        vec2(0.421003, 0.027070),
        vec2(-0.817194, -0.271096),
        vec2(-0.705374, -0.668203),
        vec2(0.977050, -0.108615),
        vec2(0.063326, 0.142369),
        vec2(0.203528, 0.214331),
        vec2(-0.667531, 0.326090),
        vec2(-0.098422, -0.295755),
        vec2(-0.885922, 0.215369),
        vec2(0.566637, 0.605213),
        vec2(0.039766, -0.396100),
        vec2(0.751946, 0.453352),
        vec2(0.078707, -0.715323),
        vec2(-0.075838, -0.529344),
        vec2(0.724479, -0.580798),
        vec2(0.222999, -0.215125),
        vec2(-0.467574, -0.405438),
        vec2(-0.248268, -0.814753),
        vec2(0.354411, -0.887570),
        vec2(0.175817, 0.382366),
        vec2(0.487472, -0.063082),
        vec2(-0.084078, 0.898312),
        vec2(0.488876, -0.783441),
        vec2(0.470016, 0.217933),
        vec2(-0.696890, -0.549791),
        vec2(-0.149693, 0.605762),
        vec2(0.034211, 0.979980),
        vec2(0.503098, -0.308878),
        vec2(-0.016205, -0.872921),
        vec2(0.385784, -0.393902),
        vec2(-0.146886, -0.859249),
        vec2(0.643361, 0.164098),
        vec2(0.634388, -0.049471),
        vec2(-0.688894, 0.007843),
        vec2(0.464034, -0.188818),
        vec2(-0.440840, 0.137486),
        vec2(0.364483, 0.511704),
        vec2(0.034028, 0.325968),
        vec2(0.099094, -0.308023),
        vec2(0.693960, -0.366253),
        vec2(0.678884, -0.204688),
        vec2(0.001801, 0.780328),
        vec2(0.145177, -0.898984),
        vec2(0.062655, -0.611866),
        vec2(0.315226, -0.604297),
        vec2(-0.780145, 0.486251),
        vec2(-0.371868, 0.882138),
        vec2(0.200476, 0.494430),
        vec2(-0.494552, -0.711051),
        vec2(0.612476, 0.705252),
        vec2(-0.578845, -0.768792),
        vec2(-0.772454, -0.090976),
        vec2(0.504440, 0.372295),
        vec2(0.155736, 0.065157),
        vec2(0.391522, 0.849605),
        vec2(-0.620106, -0.328104),
        vec2(0.789239, -0.419965),
        vec2(-0.545396, 0.538133),
        vec2(-0.178564, -0.596057)
    );

    vec3 N = normalize(f_normal);
    vec3 L = normalize(u_lightPos - f_position);
    float ndotl = max(0.0, dot(N, L));
    float bias = tan(acos(ndotl)) * 1.0e-4;

    vec2 depthUv = (f_posLightSpace.xy / f_posLightSpace.w) * 0.5 + 0.5;
    float dvalue = f_posLightSpace.z / f_posLightSpace.w;

    float visibility = 1.0;
    float loss = 1.0 / nSamples;
    for (int i = 0; i < nSamples; i++) {
        vec2 jitteredUv = depthUv + samples[i] * u_kernelSize;
        float zvalue = texture(u_depthMap, jitteredUv).x;
        if (zvalue < dvalue - bias) {
            visibility -= loss;
        }
    }
    return visibility;
}

float exponentialShadowMaps() {
    vec2 depthUv = (f_posLightSpace.xy / f_posLightSpace.w) * 0.5 + 0.5;
    float dvalue = (f_posLightSpace.z / f_posLightSpace.w);
    float zvalue = texture(u_depthMap, depthUv).x;
    dvalue = exp(-u_esmCoeff * dvalue);

    float visibility = 1.0 - u_darkness * (1.0 - clamp(dvalue * zvalue, 0.0, 1.0));
    return visibility;
}

void main(void) {
    vec3 V = normalize(u_cameraPos - f_position);
    vec3 L = normalize(u_lightPos - f_position);
    vec3 N = normalize(f_normal);
    vec3 H = normalize(V + L);

    float ndotl = dot(N, L);
    float ndoth = dot(N, H);

    float rhoDiff = max(0.0, ndotl);
    float rhoSpec = pow(max(0.0, ndoth), 8.0);

    float visibility = 1.0;
    if (u_smType == NORMAL_SM) {
        visibility = shadowMaps();
    } else if (u_smType == PCF) {
        visibility = shadowMapsPCF();
    } else if (u_smType == EXPONENTIAL_SM) {
        visibility = exponentialShadowMaps();
    }

    outColor.rgb = (colorDiff * rhoDiff + colorSpec * rhoSpec) * visibility;
    outColor.a   = 1.0;
}
