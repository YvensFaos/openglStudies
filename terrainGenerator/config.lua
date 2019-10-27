vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 viewProjection;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    uniform float time;

    const float THRESHOULD = 0.025;

    const float DETAILS = 0.10;
    const float ELEVATN = 0.13;
    const float TERRAIN = 0.65;

    float height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;

        return uvP.x;
    }

    void main()
    {
        float realHeight = height(uv);

        float hL = height(uv - vec2(THRESHOULD, 0.0));
        float hR = height(uv + vec2(THRESHOULD, 0.0));
        float hD = height(uv - vec2(0.0, THRESHOULD));
        float hU = height(uv + vec2(0.0, THRESHOULD));
        
        vec3 hposition = vertex;
        hposition.y = realHeight;
        
        vec3 cnormal = vec3(hL - hR, hD - hU, 0.6);
        cnormal = normalize(cnormal);
        
        vectorOut.vposition = vec3(model * vec4(hposition, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * cnormal;
        vectorOut.vuv = uv;

        vec4 vecOut = vec4(vectorOut.vposition, 1.0);
        gl_Position = viewProjection * vecOut;
    }  
]]

fragmentShader = [[
    #version 400

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorIn;

    uniform sampler2D textureUniform0;

    float factorLightConstant = 1.0f;
    float factorLightLinear = 0.09f;
    float factorLightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform int numberPointLights = 0;
    uniform int numberDirectionLights = 0;

    uniform Light pointLights[10];
    uniform Light directionalLights[10];
    
    out vec4 frag_colour;

    vec4 calculatePointLight(int index, const vec3 norm) {
        vec3 directionV = pointLights[index].position - vectorIn.vposition;
        float distance = length(directionV);
        float attenuationIntensity = pointLights[index].intensity / maximumIntensity;
        float lightConstant = factorLightConstant / attenuationIntensity;
        float lightLinear = factorLightLinear / attenuationIntensity;
        float lightQuadratic = factorLightQuadratic / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        vec3 lightDir = normalize(directionV);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse =  attenuation * diff * pointLights[index].color.rgb;
        return vec4(diffuse, 1.0);
    }

    vec4 calculateDirectionalLight(int index, const vec3 norm) {
        vec3 directionV = directionalLights[index].position - vectorIn.vposition;
        float distance = length(directionV);
        float attenuationIntensity = directionalLights[index].intensity / maximumIntensity;
        float lightConstant = factorLightConstant / attenuationIntensity;
        float lightLinear = factorLightLinear / attenuationIntensity;
        float lightQuadratic = factorLightQuadratic / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(-directionalLights[index].direction, norm), 0.0);
        vec3 diffuse =  attenuation * diff * directionalLights[index].color.rgb;
        return vec4(diffuse, 1.0);
    }

    void main() {          
        vec3 norm = normalize(vectorIn.vnormal);

        vec4 directionalLightResultant = vec4(0.0);
        int clampedNumberOfDirectional = min(numberDirectionLights, 10);
        for(int i = 0; i < clampedNumberOfDirectional; i++) {
            directionalLightResultant += calculateDirectionalLight(i, norm);
        }

        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        for(int i = 0; i < clampedNumberOfPoint; i++) {
            pointLightResultant += calculatePointLight(i, norm);
        }

        vec4 resultantLight = directionalLightResultant + pointLightResultant + texture(textureUniform0, vectorIn.vuv);
        resultantLight.a = 1.0;

        frag_colour = resultantLight;
    }
]]

models = {}
models[1] = {file = "../3DModels/plane100x100.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

light = {pos = {0.0, 0.0, 0.0}, 
         dir = {0.0, -10.0, 0.0}, 
         up = {0.0, 1.0, 0.0}, 
         col = {237.0, 228.0, 62.0, 1.0}, 
         intensity = 0.125, 
         specularPower = 128.0, 
         directional = true }

cameraPosition = {
    pos   = {-0.047,  2.920,  5.758},
    dir   = {-0.000, -0.681, -0.732},
    up    = {-0.000,  0.732, -0.681},
    right = { 1.000,  0.000, -0.000}
}