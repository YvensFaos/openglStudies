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

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
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

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    out vec4 frag_colour;

    void main()
    {          
        vec3 norm = normalize(vectorIn.vnormal);
        float distance = length(sceneLight.position - vectorIn.vposition);

        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;

        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

gaussianHeader = [[
    #version 400
    in vec2 vuv;

    const float PixOffset[5] = float[](0.0,1.0,2.0,3.0,4.0);
    const float factor = 0.00125;
    
    uniform sampler2D textureUniform;
    uniform float gaussianWeight[5];

    out vec4 frag_colour;
]]

gaussianBlur1 = gaussianHeader .. [[
    void main() {
        vec4 xPass = texture(textureUniform, vuv) * gaussianWeight[0];
        for(int i = 1; i < 5; i++) {
            xPass += texture(textureUniform, vuv + vec2(factor *  PixOffset[i], 0)) * gaussianWeight[i];
            xPass += texture(textureUniform, vuv + vec2(factor * -PixOffset[i], 0)) * gaussianWeight[i];
        }
        vec4 col = xPass;
        col.a = 1.0;
        frag_colour = col;
    }
]]

gaussianBlur2 = gaussianHeader .. [[
    void main() {
        vec4 yPass = texture(textureUniform, vuv) * gaussianWeight[0];
        for(int i = 1; i < 5; i++) {
            yPass += texture(textureUniform, vuv + vec2(0, factor *  PixOffset[i])) * gaussianWeight[i];
            yPass += texture(textureUniform, vuv + vec2(0, factor * -PixOffset[i])) * gaussianWeight[i];
        }
        vec4 col = yPass;
        col.a = 1.0;
        frag_colour = col;
    }
]]

models = {}
models[1] = {file = "../3DModels/monkey.fbx", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 4.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

cameraPosition = {
    pos   = {-0.909, -0.323, 9.711},
    dir   = {0.155, 0.021, -0.988},
    up    = {-0.003, 1.000, 0.021},
    right = {0.988, -0.000, 0.155},
    angle = {-81.089, 1.200}
}

sigma = 4.0

showSkybox = true