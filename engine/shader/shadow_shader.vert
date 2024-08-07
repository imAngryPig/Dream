// #version 450

// const vec2 OFFSETS[6] = vec2[](
//   vec2(-1.0, -1.0),
//   vec2(-1.0, 1.0),
//   vec2(1.0, -1.0),
//   vec2(1.0, -1.0),
//   vec2(-1.0, 1.0),
//   vec2(1.0, 1.0)
// );

// struct PointLight {
//   vec4 position; // ignore w
//   vec4 color; // w is intensity
// };

// layout(set = 0, binding = 0) uniform GlobalUbo {
//   mat4 projection;
//   mat4 view;
//   mat4 invView;
//   vec4 ambientLightColor; // w is intensity
//   PointLight pointLights[10]; // pls try Specialization constant when creating pipeline for substitude '10'
//   int numLights;
// } ubo;

// layout(push_constant) uniform PushConstants {
//     mat4 modelMatrix; 
//     mat4 normalMatrix;
// } push;


// void main() {
//     gl_Position = vec4(OFFSETS[gl_VertexIndex], 0.0, 1.0);

// }


#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  mat4 lightProjection;
  mat4 lightView;
  mat4 lightInvView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10]; // pls try Specialization constant when creating pipeline for substitude '10'
  int numLights;
} ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

layout(location = 3) out vec2 v_Texcoord;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

// const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));


void main(){
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    // temporary: this is only correct in certain situations!
    // only works correctly if scale is uniform (sx == sy == sz)
    // vec3 normalWorldSpace = normalize(mat3( push.modelMatrix ) * normal);

    // calculating the inverse in a shader can be expensive and should be avoided
    // mat3 normalMatrix = transpose(inverse(mat3( push.modelMatrix )));
    // vec3 normalWorldSpace = normalize(normalMatrix * normal);
    // fragNormalWorld = normalWorldSpace;
    
    fragNormalWorld = normalize(mat3( push.normalMatrix ) * normal);
    
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    v_Texcoord = uv;

}