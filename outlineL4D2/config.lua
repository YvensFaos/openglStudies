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
        frag_colour = vec4(diffuse, 1.0);
    }
]]

singleColorFragment = [[
    #version 400

    out vec4 frag_colour;

    void main()
    {          
        frag_colour = vec4(242.0 / 255.0, 85.0 / 255.0, 237.0 / 255.0, 1.0);
    }
]]

blurFragmentShader = [[
    #version 400

    in vec2 vuv;
    uniform sampler2D textureUniform;
    out vec4 frag_colour;

    void main() {
        float offset = 0.001;

        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right
        );

        float kernel[9] = float[](
            4.0,  8.0, 4.0,
            8.0, 16.0, 8.0,
            4.0,  8.0, 4.0  
        );

        vec4 sampleTex[9];
        vec4 col = vec4(0.0);
        for(int i = 0; i < 9; i++)
        {
            col += texture(textureUniform, vuv.st + offsets[i]) * kernel[i];
        }
        col /= 40.0;
        frag_colour = col;
    }
]]

edgeFragmentShader = [[
    #version 400

    in vec2 vuv;
    uniform sampler2D textureUniform;
    out vec4 frag_colour;

    void main() {
        float offset = 0.001;

        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right
        );

        float kernel[9] = float[](
            1.0,  1.0, 1.0,
            1.0, -8.0, 1.0,
            1.0,  1.0, 1.0  
        );

        vec4 sampleTex[9];
        vec4 col = vec4(0.0);
        for(int i = 0; i < 9; i++)
        {
            col += texture(textureUniform, vuv.st + offsets[i]) * kernel[i];
        }
        col.a = 1.0;
        frag_colour = col;
    }
]]

dilationFragmentShader = [[
    #version 400

    in vec2 vuv;
    uniform sampler2D textureUniform;
    out vec4 frag_colour;

    void main() {
        float offset = 0.001;

        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset),
            vec2( 0.0f,    offset),
            vec2( offset,  offset),
            vec2(-offset,  0.0f),  
            vec2( 0.0f,    0.0f),  
            vec2( offset,  0.0f),  
            vec2(-offset, -offset),
            vec2( 0.0f,   -offset),
            vec2( offset, -offset) 
        );

        float kernel[9] = float[](
            1.0,  1.0, 1.0,
            1.0,  1.0, 1.0,
            1.0,  1.0, 1.0  
        );

        vec4 sampleTex[9];
        vec4 col = vec4(0.0);
        for(int i = 0; i < 9; i++)
        {
            col += texture(textureUniform, vuv.st + offsets[i]) * kernel[i];
        }
        col.a = 1.0;
        frag_colour = col;
    }
]]

auraOnlyFragment = [[
    #version 400

    in vec2 vuv;
    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    out vec4 frag_colour;

    void main() {
        vec3 result = vec3(texture(textureUniform2, vuv.st) - texture(textureUniform1, vuv.st));
        result = vec3(242.0 / 255.0, 85.0 / 255.0, 237.0 / 255.0) * result;
        frag_colour =  vec4(result, 1.0);
    }
]]

stackUpFrames = [[
    #version 400

    in vec2 vuv;
    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    out vec4 frag_colour;

    void main() {
        frag_colour =  texture(textureUniform1, vuv.st) + texture(textureUniform2, vuv.st);
    }
]]

models = {}
models[1] = {file = "Resources/monkey.fbx", pos = { 0.0,  0.0, 6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file = "Resources/monkey.fbx", pos = { 2.0,  0.0, 4.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[3] = {file = "Resources/monkey.fbx", pos = {-2.0,  0.0, 4.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 0.0, 5.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {0.4, 0.8, 0.9, 1.0}, intensity = lightIntensity, specularPower = 128.0, directional = true}