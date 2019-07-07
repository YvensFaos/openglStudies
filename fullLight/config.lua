vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    layout (location = 3) in vec3 tangent;

    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec3 vtangent;

        vec3 tangentLightPos;
        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorOut;

    uniform mat4 viewMatrix;
    uniform mat4 viewProjection;
    uniform mat4 model;
    uniform vec3 lightPosition;
    uniform vec3 viewPosition;

    out vec3 vPosition;
    out mat3 normalMatrix;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vuv = uv;
        
        normalMatrix = transpose(inverse(mat3(model)));
        vec3 t = normalize(normalMatrix * tangent);
        vec3 n = normalize(normalMatrix * normal);
        t = normalize(t - dot(t,n) * n);
        vec3 b = cross(n, t);

        mat3 TBN = transpose(mat3(t, b, n));

        vectorOut.vnormal = n;
        vectorOut.vtangent = t;

        vectorOut.tangentLightPos = TBN * lightPosition;
        vectorOut.tangentViewPos  = TBN * viewPosition;
        vectorOut.tangentFragPos  = TBN * vectorOut.vposition;
        
        vPosition = viewPosition;

        gl_Position = viewProjection * vec4(vectorOut.vposition, 1.0);
    }
]]

fragmentShader = [[
    #version 400
    precision highp float;

    struct AmbientLight {
        vec4 color;
        float intensity;
    };

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        float specularPower;
        bool directional;
    };

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec3 vtangent;

        vec3 tangentLightPos;
        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorIn;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform AmbientLight sceneAmbientLight;
    uniform Light sceneLight;

    uniform sampler2D texture_diffuse1;
    uniform sampler2D texture_normal1;

    in mat3 normalMatrix;
    in vec3 vPosition;

    out vec4 frag_colour;

    void main()
    {
        vec3 normal = texture(texture_normal1, vectorIn.vuv).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        vec4 diffuseText = texture(texture_diffuse1, vectorIn.vuv);

        vec4 ambient = sceneAmbientLight.intensity * sceneAmbientLight.color * diffuseText;
        ambient.a = 1.0;

        vec3 lightDirection = normalize(vectorIn.tangentLightPos - vectorIn.tangentFragPos);
        float diff = max(dot(lightDirection, normal), 0.0);

        float distance = length(sceneLight.position - vectorIn.vposition);
        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;

        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

        vec4 diffuse = attenuation * diff * sceneLight.color * diffuseText;
        diffuse.a = 1.0;

        vec3 viewDir = normalize(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        vec3 reflectDir = reflect(-lightDirection, normal);
        
        float inc = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(inc, sceneLight.specularPower);
        vec4 specular = attenuation * vec4(vec3(spec), 1.0);
        specular.a = 1.0;

        frag_colour = ambient + diffuse + specular;
    }
]]

lightFragmentShader = [[
    #version 400

    uniform vec4 lightColor;

    out vec4 frag_colour;

    void main()
    {          
        frag_colour = lightColor;
    }
]]

models = {}
models[1] = {file = "../3DModels/monkey.obj",  pos =  { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file =  "../3DModels/tsphere.obj", pos =  {-2.5,  0.0, 3.0}, sca = {1.0, 1.0, 1.0},   rot = { 0.0,  10.0, 0.0}}
models[3] = {file =  "../3DModels/tsphere.obj",  pos =  { 2.5,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, -10.0, 0.0}}
models[4] = {file = "../3DModels/monkey.obj",  pos =  {-2.5, 0.0,-6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 45.0, 0.0}}
models[5] = {file = "../3DModels/monkey.fbx",  pos =  { 2.5, 0.0,-6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,-45.0, 0.0}}
models[6] = {file = "../3DModels/cube.obj",  pos =  { 0.0,  0.0,-7.5}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightObject = {file = "../3DModels/tsphere.fbx", pos = {0.0,  0.0, 0.0}, sca = {0.125, 0.125, 0.125}, rot = { 0.0,  0.0, 0.0}}

ambient = {col = {255 / 255, 255 / 255, 255 / 255, 1.0}, intensity = 0.05}
cameraPosition = {
    pos =   { 1.835, 8.501, 17.85}, 
    dir =   {-0.053, -0.457, -0.888}, 
    up =    {-0.027, 0.890, -0.456}, 
    right = { 0.998,  0.0, -0.059}}

lightIntensity = 200
initialX = 0.0
initialY = 0.0
initialZ = 0.0
light = {pos = {initialX, initialY, initialZ}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {250 / 255, 150 / 255, 227 / 205, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

accuX = 0
accuY = 0
accuZ = 0

function move(x, y, z, deltaTime)
    accuX = accuX + deltaTime * 0.275
    accuY = accuY + deltaTime * 4.275
    accuZ = accuZ + deltaTime * 0.400
    return z, initialY + 4 * math.sin(accuY), initialX + 6 * math.sin(accuX)
end

debug = false