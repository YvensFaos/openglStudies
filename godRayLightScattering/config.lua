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
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz + 0.2*sceneLight.color.xyz;
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

lightFragmentShader = [[
    #version 400

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorIn;

    uniform vec4 lightColor;
    out vec4 frag_colour;

    void main()
    {          
        frag_colour = lightColor;
    }
]]

lightScatteringShader = [[
    #version 400

    in vec2 vuv;

    uniform float exposure;
    uniform float decay;
    uniform float density;
    uniform float weight;
    uniform vec2 lightPositionOnScreen;
    uniform sampler2D textureUniform;

    const int NUM_SAMPLES = 100;
    const float LIGHT_THRESHOLD = 0.0125;

    out vec4 frag_colour;

    void main()
    {	
        // Calculate vector from pixel to light source in screen space.
        vec2 lps = (lightPositionOnScreen.xy + 1) / 2.0;
        vec2 deltaTexCoord = (vuv - lps.xy);
        deltaTexCoord *= 1.0f / NUM_SAMPLES * density;

        // Store initial sample.
        vec4 color = texture(textureUniform, vuv);

        // Set up illumination decay factor.
        float illuminationDecay = 1.0f;

        vec2 texCoord = vuv;

        for (int i = 0; i < NUM_SAMPLES; i++)
        {
            // Step sample location along ray.
            texCoord -= deltaTexCoord;
            
            // Retrieve sample at new location.
            vec4 ssample = texture(textureUniform, texCoord);

            // Apply sample attenuation scale/decay factors.
            ssample *= illuminationDecay * weight;

            // Accumulate combined color.
            color += ssample;

            // Update exponential decay factor.
            illuminationDecay *= decay;
        }
        color *= exposure;
        frag_colour = vec4(color.r, color.g, color.b, 1.0);
    }
]]

mixFragmentShader = [[
    #version 400

    in vec2 vuv;
    out vec4 frag_colour;

    uniform sampler2D textureUniform0;
    uniform sampler2D textureUniform1;

    void main() {
        frag_colour = texture(textureUniform0, vuv) + texture(textureUniform1, vuv);
    }
]]

models = {}
models[1] = {file = "Resources/monkey.fbx", pos = { 0.0,  0.0,  3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file = "Resources/monkey.fbx", pos = { 3.0,  0.0,  1.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 45.0, 0.0}}
models[3] = {file = "Resources/monkey.fbx", pos = {-3.0,  0.0,  1.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,-45.0, 0.0}}
models[4] = {file = "Resources/monkey.fbx", pos = { 6.0,  0.0,-15.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 25.0, 0.0}}
models[5] = {file = "Resources/monkey.fbx", pos = {-6.0,  0.0,-15.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,-25.0, 0.0}}

lightPos = { 0,  0.0, -10.0}
lightObject = {file = "Resources/csphere.fbx", pos = lightPos, sca = {1.4, 1.4, 1.4}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = lightPos, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {1.0, 1.0, 0.9, 1.0}, intensity = lightIntensity, directional = true}

Exposure = 0.0034
Decay = 1.00001
Density = 0.84
Weight = 1.65

LightMovement = 11
LightSpeed = 0.9

function lightMovementFunction(accumulator, x, y, z)
    nx = x + LightMovement * math.sin(LightSpeed * accumulator)
    ny = y
    nz = z
    return nz, ny, nx
end