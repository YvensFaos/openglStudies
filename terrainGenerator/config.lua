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

    const float DETAILS = 0.10;
    const float ELEVATN = 0.13;
    const float TERRAIN = 0.65;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

    void main()
    {
        vec3 realHeight = height(uv);
        vec3 hposition = vertex;
        hposition.y = realHeight.y;

        vectorOut.vposition = vec3(model * vec4(hposition, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
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
    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    const float THRESHOULD = 0.05;
    const float SLOPE_FACTOR = 10.0;
    const float DETAILS = 0.10;
    const float ELEVATN = 0.13;
    const float TERRAIN = 0.65;

    float factorLightConstant = 1.0f;
    float factorLightLinear = 0.09f;
    float factorLightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform int showOnlyNormals = 0;
    uniform int numberPointLights = 0;
    uniform int numberDirectionLights = 0;

    uniform Light pointLights[10];
    uniform Light directionalLights[10];

    uniform mat4 model;

    out vec4 frag_colour;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

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
        vec3 co = height(vectorIn.vuv);
        vec3 hL = height(vectorIn.vuv - vec2(THRESHOULD, 0.0));
        vec3 hR = height(vectorIn.vuv + vec2(THRESHOULD, 0.0));
        vec3 hD = height(vectorIn.vuv - vec2(0.0, THRESHOULD));
        vec3 hU = height(vectorIn.vuv + vec2(0.0, THRESHOULD));
        
        vec3 cL = normalize(vec3(vec2(1.0, 0.0), (hR - hL).x * SLOPE_FACTOR));
        vec3 cD = normalize(vec3(vec2(0.0, 1.0), (hU - hD).x * SLOPE_FACTOR));
        vec3 cnormal = cross(cL, cD).xzy;

        cnormal = mat3(transpose(inverse(model))) * cnormal;
        cnormal = normalize(cnormal);

        vec4 directionalLightResultant = vec4(0.0);
        int clampedNumberOfDirectional = min(numberDirectionLights, 10);
        for(int i = 0; i < clampedNumberOfDirectional; i++) {
            directionalLightResultant += calculateDirectionalLight(i, cnormal);
        }
        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        for(int i = 0; i < clampedNumberOfPoint; i++) {
            pointLightResultant += calculatePointLight(i, cnormal);
        }
        vec4 resultantLight = directionalLightResultant + pointLightResultant + texture(textureUniform0, vectorIn.vuv);
        resultantLight.a = 1.0;

        if(gl_FrontFacing) {
            if(showOnlyNormals == 1) {
                frag_colour = vec4(cnormal, 1.0);
            } else {
                frag_colour = resultantLight;
            }
        } else {
            frag_colour = vec4(0.05, 0.05, 0.05, 1.0);
        }
    }
]]

models = {}
models[1] = {file = "../3DModels/plane100x100.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 0.0, 0.0}}

light = {pos = {0.0, 1.5, 3.0}, 
         dir = {-1.0, 0.0, 0.0}, 
         up = {0.0, 1.0, 0.0}, 
         col = {253.0, 228.0, 62.0, 1.0}, 
         intensity = 0.25, 
         specularPower = 128.0, 
         directional = true }

cameraPosition = {
    -- pos   = {2.296, 1.766, 4.429},
    -- dir   = {-0.797, -0.422, -0.432},
    -- up    = {-0.371, 0.906, -0.201},
    -- right = {0.476, 0.000, -0.879}
    -- pos   = {-0.164, 2.866, -0.505},
    -- dir   = {0.024, -0.422, 0.906},
    -- up    = {0.011, 0.907, 0.422},
    -- right = {-1.000, 0.000, 0.026}
    pos   = {0.152, 2.636, 7.039},
    dir   = {-0.024, -0.422, -0.906},
    up    = {-0.011, 0.907, -0.422},
    right = {1.000, 0.000, -0.026}
}