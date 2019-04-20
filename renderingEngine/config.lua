skyboxVertexShader = [[
    #version 400
    layout (location = 0) in vec3 aPos;

    uniform mat4 vpMatrix;

    out vec3 texCoord;

    void main()
    {
        texCoord = aPos;
        vec4 pos = vpMatrix * vec4(aPos, 1.0);
        gl_Position = pos.xyww;
    }  
]]

skyboxFragmentShader = [[
    #version 400
    in vec3 texCoord;

    uniform samplerCube skybox;

    out vec4 frag_colour;

    void main()
    {   
        vec4 color = texture(skybox, texCoord);
        frag_colour = color;
    }
]]

vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vlightSpace;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 viewProjection;
    uniform mat4 lightViewProjection;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
        vectorOut.vuv = uv;
        vec4 vecOut = vec4(vectorOut.vposition, 1.0);
        vectorOut.vlightSpace = lightViewProjection * vecOut; 

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
        vec4 vlightSpace;
    } vectorIn;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    uniform sampler2D shadowMap;

    out vec4 frag_colour;

    float calculateShadows(vec4 vlightSpace) {
        vec3 projCoords = vlightSpace.xyz / vlightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float currentDepth = projCoords.z;
        float bias = max(0.05 * (1.0 - dot(vectorIn.vnormal, sceneLight.direction)), 0.05);
        
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;

        if(projCoords.z > 1.0) {
            shadow = 0.0;
        }
        
        return shadow;
    }

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
        float shadow = calculateShadows(vectorIn.vlightSpace);
        vec3 diffuse =  (1.0 - shadow) * attenuation * diff * sceneLight.color.xyz;
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

shadowVertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    
    uniform mat4 model;
    uniform mat4 lightViewProjection;
    
    void main()
    {
        gl_Position = lightViewProjection * model * vec4(vertex, 1.0);
    }  
]]

shadowFragmentShader = [[
    #version 400

    out vec4 frag_colour;

    void main()
    {
        frag_colour = vec4(vec3(gl_FragCoord.w), 1.0);
    }  
]]

debugQuadVertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec2 uv;

    out vec2 vuv;

    void main()
    {
        vuv = uv;
        gl_Position = vec4(vertex, 1.0);
    }
]]

debugQuadFragShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform;

    out vec4 frag_colour;

    void main()
    {             
        float depthValue = texture(textureUniform, vuv).r;
        frag_colour = vec4(vec3(depthValue), 1.0);
    }
]]

models = {}
models[1] = {file = "monkey.fbx", pos = { 4.0,  0.0, 0.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file = "monkey.fbx", pos = { 0.0,  0.0, 1.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[3] = {file = "monkey.fbx", pos = {-2.0,  0.4, 0.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 30.0, 0.0}}
models[4] = {file = "monkey.fbx", pos = { 1.0,  2.5, 0.8}, sca = {1.1, 1.1, 1.1}, rot = {90.0,  0.0, 0.0}}

plane = {file = "plane.fbx",  pos = { 0.0, 0.5, -4.0}, sca = {8.0, 1.0, 8.0}, rot = {20.0,  0.0, 0.0}}
lightIntensity = 80
light = {pos = { 0.0, 2.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {247 / 255, 208 / 255, 54 / 255, 1.0}, intensity = lightIntensity, directional = true}
nearPlane = -4.0
farPlane = 16.0
projectionDimension = 11.25

function updateLight(deltaTime, accumulator, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    angle = math.rad(accumulator * 0.04)
    cos = math.cos(angle)
    sin = math.sin(angle)
    pcos = math.cos(accumulator)
    psin = math.sin(accumulator)
    safey =  cos * posy + sin * posz
    safez = -sin * posy + cos * posz
    posy = safey
    posz = safez
    safey =  cos * diry + sin * dirz
    safez = -sin * diry + cos * dirz
    diry = safey
    dirz = safez
    safey =  cos * upy + sin * upz
    safez = -sin * upy + cos * upz
    upy = safey
    upz = safez
    return intensity, cola, colb, colg, colr, upz, upy, upx, dirz, diry, dirx, posz, posy, posx
end