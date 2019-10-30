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

    struct Fog {
        float maxDist;
        float minDist;
        vec4 color;
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
    
    uniform vec3 cameraPosition;
    uniform Light sceneLight;
    uniform Fog sceneFog;

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

        distance = length(cameraPosition - vectorIn.vposition);
        float fogFactor = (sceneFog.maxDist - distance) / (sceneFog.maxDist - sceneFog.minDist); 
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        frag_colour = mix(sceneFog.color, vec4(diffuse, sceneLight.color.w), fogFactor);
    }
]]

models = {}
models[1] = {file = "../3DModels/monkey.fbx", pos = { 1.0,  0.0, 2.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file = "../3DModels/monkey.fbx", pos = { 1.5,  0.0, 4.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[3] = {file = "../3DModels/monkey.fbx", pos = { 2.0,  0.0, 6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[4] = {file = "../3DModels/monkey.fbx", pos = { 2.5,  0.0, 8.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 4.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

cameraPosition = {
    pos   = {0.181, -0.723, 18.367},
    dir   = {0.128, 0.100, -0.987},
    up    = {-0.013, 0.995, 0.099},
    right = {0.992, -0.000, 0.129},
    angle = {-82.586, 5.740}
}

fog = {
    maxDist = 15.0,
    minDist =  2.0,
    color = {118 / 255, 227 / 255, 17 / 255, 1.0}
}