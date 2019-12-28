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
    uniform mat4 view;
    uniform mat4 projection;
    uniform mat4 projected;

    out vec4 projectedCoords;

    void main()
    {
        vec4 position = vec4(vertex,1.0);

        vectorOut.vposition = vec3(model * position);
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
        vectorOut.vuv = uv;
        vec4 vecOut = vec4(vectorOut.vposition, 1.0);

        projectedCoords = projected * vec4(vectorOut.vposition, 1.0);

        gl_Position = projection * view * vecOut;
    }  
]]

fragmentShader = [[
    #version 400

    uniform sampler2D projectedTexture;

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

    in vec4 projectedCoords;

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

        vec4 projTexColor = vec4(0.0); 
        if( projectedCoords.z > 0.0 ) {
            projTexColor = textureProj(projectedTexture,projectedCoords);
        }

        vec4 finalColor = vec4(diffuse, sceneLight.color.w) + vec4(vec3(0.5 * projTexColor.rgb), 1.0);
        finalColor.a = 1.0;

        frag_colour = finalColor;
    }
]]

textureDirectory = "../3DModels"
texturePath = "toy_box_normal.png"

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 2.0}, sca = {1.0, 1.0, 1.0}, rot = {-20.0,  0.0, 0.0}}
models[2] = {file = "../3DModels/plane100x100.obj", pos = { 0.0, -1.0, 0.0}, sca = {5.0, 1.0, 5.0}, rot = {15.0, 0.0, 0.0}}
models[3] = {file = "../3DModels/plane100x100.obj", pos = { 0.0, -2.0, 0.0}, sca = {10.0, 1.0, 10.0}, rot = {5.0, 0.0, 0.0}}

lightIntensity = 200
light = {
    pos = { 0.0, 5.0, 0.0}, 
    dir = {0.0, 1.0, 0.0}, 
    up = {0.0, 0.0, 1.0}, 
    col = {200 / 255, 200 / 255, 200 / 255, 1.0}, 
    intensity = lightIntensity, 
    specularPower = 256.0, 
    directional = true
}

projPos = { 0.0, 3.0, -1.0}
projAt  = { 0.0, 0.0, 0.0}
projUp  = { 0.0, 0.0, 1.0}
projSca = { 0.5, 0.5, 0.5}
projTra = { 0.5, 0.5, 0.5}
    
cameraPosition = {
    pos   = {4.381, 6.902, 13.455},
    dir   = {-0.315, -0.633, -0.707},
    up    = {-0.258, 0.774, -0.578},
    right = {0.913, 0.000, -0.407},
    angle = {-114.028, -39.289}
}