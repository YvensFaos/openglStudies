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
    } vectorOut;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
        vectorOut.vuv = uv;

        gl_Position = projection * view * model * vec4(vertex, 1.0);
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

    uniform int numberPointLights;
    uniform int numberDirectionLights;

    uniform Light pointLights[10];
    uniform Light directionalLights[10];
    
    out vec4 frag_colour;

    vec4 calculatePointLight(int index, const vec3 norm) {
        vec3 directionV = pointLights[index].position - vectorIn.vposition;
        float distance = length(directionV);
        float attenuationIntensity = pointLights[index].intensity / maximumIntensity;
        lightConstant = 1.0f   / attenuationIntensity;
        lightLinear = 0.09f    / attenuationIntensity;
        lightQuadratic = 0.032f/ attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        vec3 lightDir = normalize(directionV);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse =  attenuation * diff * pointLights[index].color.rgb;
        return vec4(diffuse, 1.0);
    }

    vec4 calculateDirectionalLight(int index, const vec3 norm) {
        float distance = length(directionalLights[index].position - vectorIn.vposition);
        float attenuationIntensity = directionalLights[index].intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, directionalLights[index].direction), 0.0);
        vec3 diffuse =  attenuation * diff * directionalLights[index].color.rgb;
        return vec4(diffuse, 1.0);
    }

    void main()
    {          
        vec3 norm = normalize(vectorIn.vnormal);

        vec4 directionalLightResultant = vec4(0.0);
        int clampedNumberOfDirectional = min(numberDirectionLights, 10);
        for(int i = 0; i < clampedNumberOfDirectional; i++) 
        {
            directionalLightResultant += calculateDirectionalLight(i, norm);
        }

        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        for(int i = 0; i < clampedNumberOfPoint; i++) 
        {
            pointLightResultant += calculatePointLight(i, norm);
        }

        vec4 resultantLight = directionalLightResultant + pointLightResultant;
        resultantLight.a = 1.0;

        frag_colour = resultantLight;
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

cameraPosition = {
    pos =   {-1.961, -0.053, 8.617},
    dir =   {0.414, -0.021, -0.910},
    up =    {0.009, 1.000, -0.019 },
    right = {0.910, -0.000, 0.414 }}

models = {}
models[1] = {file = "../3DModels/monkey.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, 0.0, 0.0}}

posInitial = 5.0
lightObject = {file = "../3DModels/tsphere.fbx", pos = {0.0,  0.0, posInitial}, sca = {0.1, 0.1, 0.1}, rot = {0.0,  0.0, 0.0}}

directionalIntensity = 80;

lights = {
    {pos = { 0.0, 1.0, 3.0}, dir = {0.0, -1.0, 0.0}, up = {0.0, 1.0, 0.0}, 
    col = {227 / 255, 118 / 255, 252 / 255, 1.0}, 
    intensity = directionalIntensity, 
    specularPower = 256.0, 
    directional = true},

    {pos = { 0.0, -5.0, 3.0}, dir = {0.0, 1.0, 0.0}, up = {0.0, 1.0, 0.0}, 
    col = {100 / 255, 10 / 255, 56 / 255, 1.0}, 
    intensity = directionalIntensity, 
    specularPower = 256.0, 
    directional = true},

    {pos = { 4.0, 0.0, 3.0}, dir = {-1.0, 0.0, 0.0}, up = {0.0, 1.0, 0.0}, 
    col = {255 / 255, 0 / 255, 0 / 255, 1.0}, 
    intensity = directionalIntensity, 
    specularPower = 256.0, 
    directional = true},

    {pos = {-4.0, 0.0, 3.0}, dir = { 1.0, 0.0, 0.0}, up = {0.0, 1.0, 0.0}, 
    col = {0 / 255, 10 / 255, 195 / 255, 1.0}, 
    intensity = directionalIntensity, 
    specularPower = 256.0, 
    directional = true},

    {pos = {0.0, 7.0, 0.0}, dir = {0.0, 0.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 0 / 255, 0 / 255, 1.0}, 
    intensity = 300, 
    specularPower = 256.0, 
    directional = false},

    {pos = {0.0, 0.0, posInitial}, dir = {0.0, 0.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 0 / 255, 1.0}, 
    intensity = 90, 
    specularPower = 256.0, 
    directional = false}
}

accumulator = 0
function updateLight(deltaTime, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    accumulator = accumulator + deltaTime
    nposx = posx
    nposy = posy
    nposz = posInitial + 0.35 + 1.5 * math.sin(math.rad(accumulator) * 100.0)
    return intensity, cola, colb, colg, colr, upz, upy, upx, dirz, diry, dirx, nposz, nposy, nposx
end

debug = false