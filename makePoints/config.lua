vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;

    out vectorOut {
        vec4 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main()
    {
        vectorOut.vposition = viewProjection * model * vec4(vertex, 1.0);
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
        vectorOut.vuv = uv;

        gl_Position = vectorOut.vposition;
    }  
]]

pointGeometricShader = [[
    #version 400
    layout (triangles) in;
    layout (points, max_vertices = 128) out;

    in vectorOut {
        vec4 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorIn[];

    out vectorFOut {
        vec4 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorFOut;

    uniform int density;

    void main()
    {
        vec3 dir1 = (vec3(vectorIn[1].vposition - vectorIn[0].vposition));
        float hl1 = length(dir1);
        dir1 = normalize(dir1);

        vectorFOut.vposition = vectorIn[0].vposition;
        vectorFOut.vnormal = vectorIn[0].vnormal;
        vectorFOut.vuv = vectorIn[0].vuv;

        gl_Position = vectorIn[0].vposition;
        EmitVertex();

        gl_Position = vectorIn[1].vposition;
        EmitVertex();   

        float step = hl1 / density;
        float stepP = step;
        while(stepP < hl1) {
            gl_Position = vectorIn[0].vposition + vec4(stepP * dir1, 0);
            EmitVertex();
            stepP += step;
        }
        gl_Position = vectorIn[1].vposition;
        EmitVertex();

        EndPrimitive();
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

    in vectorFOut {
        vec4 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorFOut;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    out vec4 frag_colour;

    void main()
    {          
        vec3 norm = normalize(vectorFOut.vnormal);
        float distance = length(sceneLight.position - vec3(vectorFOut.vposition));
        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;
        frag_colour = vec4(diffuse, sceneLight.color.w) + vec4(1.0, 0.2, 0.2, 0.5);
    }
]]

models = {}
models[1] = {file = "../3DModels/monkey.fbx", pos = { 0.0,  0.0, -2.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 4.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

density = 30