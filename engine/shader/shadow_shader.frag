// #version 450

// layout (location = 0) out vec4 outColor;

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

// layout(set = 1, binding = 0) uniform sampler2D shadowMap;

// layout(push_constant) uniform PushConstants {
//     mat4 modelMatrix; 
//     mat4 normalMatrix;
// } push;


// void main() {
//   vec2 fragCoord = gl_FragCoord.xy;
//   outColor = texture(shadowMap, (fragCoord / vec2(1280, 1060)) );
//   outColor = outColor.bgra;
//   // outColor = vec4((fragCoord / vec2(1280, 1060)).y, 0, 0, 1.0);

// }

#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 v_Texcoord;

layout(location = 0) out vec4 outColor;

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

layout(set = 1, binding = 0) uniform sampler2D map_Ka;
layout(set = 1, binding = 1) uniform sampler2D map_Kd;
layout(set = 1, binding = 2) uniform sampler2D map_Ks;
layout(set = 1, binding = 3) uniform sampler2D map_Ke;

layout(set = 2, binding = 0) uniform materialCoef {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 emission;
  float shininess;
  float ior;
  float dissolve;
  int illum;
  int Ka_exist;
  int Kd_exist;
  int Ks_exist;
  int Ke_exist;
} mcoef;

layout(set = 3, binding = 0) uniform sampler2D shadowMap;

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix; 
    mat4 normalMatrix;
} push;

vec4 floatToVec4(float f) {
  return vec4(f, f, f, 1.0);
}

vec4 depthToColor(float depth) {
  return vec4(vec3(pow(depth, 32)), 1.0);
}

void main(){
    vec2 texelsize = 1.0 / vec2(1280,1060);
    vec4 lightSpacePos = ubo.lightProjection * ubo.lightView * vec4(fragPosWorld, 1.0);
    lightSpacePos.xyz /= lightSpacePos.w;
    lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5; // [-1, 1] -> [0, 1]
    float nearestDepth = texture(shadowMap, lightSpacePos.xy).bgra.x;
    lightSpacePos.z = pow(lightSpacePos.z, 32);

    vec3 diffuseLight = (ubo.ambientLightColor.xyz * ubo.ambientLightColor.w) * mcoef.ambient.xyz;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    for (int i = 0; i < ubo.numLights; i++) {
      PointLight light = ubo.pointLights[i];
      vec3 directionToLight = light.position.xyz - fragPosWorld;
      float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
      directionToLight = normalize(directionToLight);

      float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
      vec3 intensity = light.color.xyz * light.color.w * attenuation;

      diffuseLight += intensity * cosAngIncidence;

      // specular lighting
      vec3 halfAngle = normalize(directionToLight + viewDirection);
      float blinnTerm = dot(surfaceNormal, halfAngle);
      blinnTerm = clamp(blinnTerm, 0, 1);
      blinnTerm = pow(blinnTerm, 256.0); // higher values -> sharper highlight
      specularLight += intensity * blinnTerm;
    }
    
    // outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);

    vec3 diffuseColor;
    vec3 specularColor;
    if(mcoef.Kd_exist == 0) diffuseColor = fragColor;
    else diffuseColor = texture(map_Kd, v_Texcoord).xyz;
    if(mcoef.Ks_exist == 0) specularColor = fragColor;
    else specularColor = texture(map_Ks, v_Texcoord).xyz;

    outColor = vec4(diffuseLight * diffuseColor * mcoef.diffuse.xyz + specularLight * specularColor * mcoef.specular.xyz, 1.0);
    outColor.xyz = clamp(outColor.xyz, 0.0, 1.0);



    if(lightSpacePos.x > -0.0001 && lightSpacePos.x < 1.0001 && lightSpacePos.y > -0.0001 && lightSpacePos.y < 1.0001
    ) {
      int sampleCount = 4;
      float percentage = 0.0;
      float count = (2 * sampleCount + 1) * (2 * sampleCount + 1);
      float shadowCount = 0.0;
      for (int i = -sampleCount; i <= sampleCount; i++) 
        for (int j = -sampleCount; j <= sampleCount; j++) {
          vec2 offset = vec2(i,j) * texelsize;
          nearestDepth = texture(shadowMap, lightSpacePos.xy + offset).bgra.x;
          if(lightSpacePos.z - nearestDepth > 0.0075){
            shadowCount += 1.0;
            // outColor = floatToVec4(0.1) * outColor;
          }
        }
      percentage = shadowCount / count;
      outColor = floatToVec4(1 - percentage) * outColor;
      return ;
    }

    // outColor.xyz = pow(outColor.xyz, vec3(1.0/2.2));

    // vec4 fragCamera = ubo.projection * ubo.view * vec4(fragPosWorld, 1.0);
    // fragCamera /= fragCamera.w;
    // outColor = vec4(vec3(pow(gl_FragCoord.z, 32)), 1.0);

    // outColor = vec4(vec3(nearestDepth), 1.0);
}