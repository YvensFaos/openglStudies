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

    uniform mat4 viewProjection;
    uniform mat4 model;

    uniform vec3 lightPosition;
    uniform vec3 viewPosition;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vuv = uv;

        mat3 normalMatrix = transpose(inverse(mat3(model)));
        vec3 T = normalize(normalMatrix * tangent);
        vec3 N = normalize(normalMatrix * normal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);

        mat3 TBN = transpose(mat3(T, B, N));
        vectorOut.tangentLightPos = TBN * lightPosition;
        vectorOut.tangentViewPos = TBN * viewPosition;
        vectorOut.tangentFragPos = TBN * vectorOut.vposition;
        
        vectorOut.vnormal = N;
        vectorOut.vtangent = tangent;

        gl_Position = viewProjection * vec4(vectorOut.vposition, 1.0);
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

fragmentShader = [[
    #version 400

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

    out vec4 frag_colour;

    void main()
    {
        //Calculate normal vector
        vec3 normal = texture(texture_normal1, vectorIn.vuv).rgb;
        normal = normalize(normal * 2.0 - 1.0);

        //Calculate diffuse color
        vec4 diffuse = texture(texture_diffuse1, vectorIn.vuv);

        //Calculate ambientTerm
        vec4 ambientTerm = sceneAmbientLight.intensity * sceneAmbientLight.color * diffuse;
        ambientTerm.a = 1.0;

        //Calculate diffuseTerm
        vec3 lightDirection = normalize(vectorIn.tangentLightPos - vectorIn.tangentFragPos);
        float diff = max(dot(lightDirection, normal), 0.0);

        //Calculate attenuation
        float distance = length(sceneLight.position - vectorIn.tangentFragPos);
        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        float lightConstant = 1.0f    / attenuationIntensity;
        float lightLinear = 0.09f     / attenuationIntensity;
        float lightQuadratic = 0.032f / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

        vec4 diffuseTerm = attenuation * diff * sceneLight.color * diffuse;
        diffuseTerm.a = 1.0;

        //Calculate specularTerm
        vec3 viewDirection = normalize(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        vec3 reflectDirection = reflect(-lightDirection, normal);
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        float spec = pow(max(dot(normal, halfwayDirection), 0.0), sceneLight.specularPower);

        vec4 specularTerm = vec4(0.2) * spec;
        specularTerm.a = 1.0;

        vec4 result = specularTerm; //ambientTerm; // + diffuseTerm;// + specularTerm;
        result.a = 1.0;
        frag_colour = result;
    }
]]

--[[
    

        
]]-- 

models = {}
models[1] = {file = "../3DModels/monkey.obj",  pos =  { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[2] = {file =  "../3DModels/tsphere.obj", pos =  {-2.5,  0.0, 3.0}, sca = {1.0, 1.0, 1.0},   rot = { 0.0,  10.0, 0.0}}
models[3] = {file =  "../3DModels/tsphere.obj",  pos =  { 2.5,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, -10.0, 0.0}}
models[4] = {file = "../3DModels/monkey.obj",  pos =  {-2.5, 0.0,-6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 45.0, 0.0}}
models[5] = {file = "../3DModels/monkey.obj",  pos =  { 2.5, 0.0,-6.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,-45.0, 0.0}}
models[6] = {file = "../3DModels/monkey.obj",  pos =  { 0.0,  0.0,-7.5}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightObject = {file = "../3DModels/tsphere.fbx",  pos =  { 0.0,  0.0, 0.0}, sca = {0.125, 0.125, 0.125}, rot = { 0.0,  0.0, 0.0}}

ambient = {col = {255 / 255, 255 / 255, 255 / 255, 1.0}, intensity = 0.2}

cameraPosition = {pos =  { -3.3,  0.2, 2.11}, up = {0.0, 1.0, 0.0}, dir = { 0.36, -0.046, -0.93}, right = { 0.93,  0.0, 0.35}}

lightIntensity = 200
initialY = -1.0
light = {pos = { 0.0, initialY, -6.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {50 / 255, 50 / 255, 227 / 255, 1.0}, intensity = lightIntensity, specularPower = 1.0, directional = true}

accu = 0
function moveY(value, deltaTime)
    accu = accu + deltaTime * 0.275
    return initialY + 4 * math.sin(accu)
end