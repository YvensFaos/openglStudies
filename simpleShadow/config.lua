vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vpositionInLightSpace;
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
        vectorOut.vpositionInLightSpace = lightViewProjection * vecOut; 

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
        vec4 vpositionInLightSpace;
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
        float diff = max(dot(-sceneLight.direction, norm), 0.0);
        //vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;
        float shadow = calculateShadows(vectorIn.vpositionInLightSpace);
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

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 0.0}, sca = {1.3, 1.3, 1.3}, rot = { 0.0,  0.0, 0.0}}
models[3] = {file = "../3DModels/plane1x1.obj", pos = { 0.0,  0.0, -3.0}, sca = {5.0, 1.0, 5.0}, rot = {70.0,  0.0, 0.0}}

shadowWidth = 1024
shadowHeight = 1024
nearPlane = -4.0
farPlane = 12.0
projectionDimension = 8.0

lightIntensity = 120
light = {
    pos = { 0.0, 1.0, 7.0}, 
    dir = {0.0, 0.0, -1.0}, 
    up = {0.0, 1.0, 0.0}, 
    col = {50 / 255, 219 / 255, 59 / 255, 1.0}, 
    intensity = lightIntensity, 
    specularPower = 128.0, 
directional = true}

cameraPosition = {
    pos   = {9.075, 0.176, 10.399},
    dir   = {-0.682, -0.044, -0.730},
    up    = {-0.030, 0.999, -0.032},
    right = {0.731, 0.000, -0.682},
    angle = {-133.014, -2.540}
}