vertexShader = [[
    #version 330 core
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vec3 FragPos;
    out vec2 TextCoords;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main()
    {
        vec4 worldPos = model * vec4(vertex, 1.0);
        FragPos = worldPos.xyz;

        Normal = mat3(transpose(inverse(model))) * normal;
        TextCoords = uv;

        gl_Position = viewProjection * worldPos;
    }  
]]

fragmentShader = [[
    #version 330 core
    layout (location = 0) out vec4 gPosition;
    layout (location = 1) out vec4 gNormal;
    layout (location = 2) out vec4 gAlbedoSpec;

    in vec3 FragPos;
    in vec2 TextCoords;
    in vec3 Normal;

    void main()
    {            
        gPosition = vec4(FragPos, 1.0);
        gNormal = vec4(normalize(Normal), 1.0f);
        gAlbedoSpec = vec4(0.0f, 1.0f, 1.0f, 1.0f);
    } 
]]

gFragmentShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform0; // POSITION
    uniform sampler2D textureUniform1; // NORMAL
    uniform sampler2D textureUniform2; // ALBEDO

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;

    out vec4 frag_colour;

    void main() {
        vec3 norm = normalize(vec3(texture(textureUniform1, vuv.st)));
        float distance = length(sceneLight.position - vec3(texture(textureUniform0, vuv.st)));
        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz * vec3(texture(textureUniform2, vuv.st));
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 4.0, 0.0}, dir = {0.0, 0.0, 1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

cameraPosition = {
    pos   = {0.000, 0.000, 6.669},
    dir   = {-0.000, 0.000, -1.000},
    up    = {0.000, 1.000, 0.000},
    right = {1.000, 0.000, -0.000},
    angle = {-90.000, 0.000}
}