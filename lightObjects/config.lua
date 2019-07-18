vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;

    out vectorOut {
        vec4 lposition;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        vectorOut.lposition = projection * view * model * vec4(vertex, 1.0);
        gl_Position = vectorOut.lposition;
    }  
]]

completeVertexShader = [[
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
        vectorOut.vuv = uv;

        mat3 normalMatrix = transpose(inverse(mat3(model)));
        vec3 n = normalize(normalMatrix * normal);
        vectorOut.vnormal = n;

        gl_Position = projection * view * model * vec4(vertex, 1.0);
    }  
]]

directionalLightFragmentShader = [[
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
        float diff = max(dot(-directionalLights[index].direction, norm), 0.0);
        float attenuationIntensity = directionalLights[index].intensity / maximumIntensity;
        vec3 diffuse =  attenuationIntensity * diff * directionalLights[index].color.rgb;
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
    uniform vec4 lightColor;

    out vec4 frag_colour;

    void main()
    {          
        frag_colour = lightColor;
    }
]]

geometryShader = [[
    #version 400
       layout (triangles) in;
       layout (line_strip, max_vertices = 10) out;
       
       in vectorOut {
           vec4 lposition;
       } vectorIn[];

       uniform vec3 lightDirection;

       const float MAGNITUDE = 0.2;
       const float ARROW_HEAD_SIZE = 0.4;
       void main()
       {
           vec3 dir1 = (vec3(vectorIn[1].lposition - vectorIn[0].lposition));
           float hl1 = length(dir1) / 2.0;
           dir1 = normalize(dir1);

           vec3 dir2 = (vec3(vectorIn[2].lposition - vectorIn[0].lposition));
           float hl2 = length(dir2) / 2.0;
           dir2 = normalize(dir2);

           vec4 midp = vectorIn[0].lposition + vec4(dir1*hl1 + dir2*hl2, 0.0);
           gl_Position = midp;
           EmitVertex();

           vec3 normalizedLightDirection = normalize(lightDirection);
           vec3 direction = vec3(midp) - normalizedLightDirection;
           direction = normalize(direction);

           vec4 finp = midp + vec4(normalizedLightDirection, 0) * MAGNITUDE;
           gl_Position = finp;
           EmitVertex();

           gl_Position = finp + vec4(cross(vec3(1, 0, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();
           gl_Position = finp + vec4(cross(vec3(-1, 0, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();
           gl_Position = finp;
           EmitVertex();

           gl_Position = finp + vec4(cross(vec3(0, 1, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();
           gl_Position = finp + vec4(cross(vec3(0, -1, 0), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();
           gl_Position = finp;
           EmitVertex();

           gl_Position = finp + vec4(cross(vec3(0, 0, 1), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();
           gl_Position = finp + vec4(cross(vec3(0, 0, -1), direction), 0) * MAGNITUDE * ARROW_HEAD_SIZE;
           EmitVertex();

           EndPrimitive();
       }
]]

cameraPosition = {
    pos   = {-0.002, 0.065, 6.590},
    dir   = {0.040, 0.067, -0.997},
    up    = {-0.003, 0.998, 0.067},
    right = {0.999, -0.000, 0.040}
}

lightModel = "../3DModels/tetrahedron.obj"

posInitial = 5.0
initialDirX = 0.0;

models = {}
models[1] = {file = "../3DModels/nonormalsphere.obj", pos = {0.0,  0.0, 0.0}, sca = {1.0, 1.0, 1.0}, rot = {0.0,  0.0, 0.0}}
models[2] = {file = "../3DModels/nonormalmonkey.obj", pos = {0.0,  1.0, 0.3}, sca = {0.5, 0.5, 0.5}, rot = {0.0,  0.0, 0.0}}
models[3] = {file = "../3DModels/cube.obj", pos = {-1.0,  0.0, 0.0}, sca = {0.3, 0.8, 0.3}, rot = {10.0, 30.0, 0.0}}
models[4] = {file = "../3DModels/cube.obj", pos = { 1.0,  0.0, 0.0}, sca = {0.3, 0.8, 0.3}, rot = {10.0, 30.0, 0.0}}

pointLight = 
    {pos = {0.0, 0.0, posInitial}, dir = {0.0, 0.0, -1.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 0 / 255, 1.0}, 
    intensity = 50, 
    specularPower = 256.0, 
    directional = true}

accumulator = 0
sign = 1
function updateLight(deltaTime, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    accumulator = accumulator + deltaTime
    nposx = posx
    nposy = posy
    nposz = posz
    ndirx = dirx + sign * accumulator * 0.005
    if ndirx >  4 then 
        sign = -1 
        accumulator = 0
    end
    if ndirx < -4 then 
        sign = 1 
        accumulator = 0
    end
    ndiry = diry
    ndirz = dirz
    return intensity, cola, colb, colg, colr, upz, upy, upx, ndirz, ndiry, ndirx, nposz, nposy, nposx
end

debug = false