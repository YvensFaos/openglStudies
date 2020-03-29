vertexDebugNormalShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    
    out vectorOut {
        vec4 vposition;
        vec3 vnormal;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main()
    {
        vectorOut.vposition = viewProjection * model * vec4(vertex, 1.0);
        vectorOut.vnormal = normal; //mat3(transpose(inverse(model))) * 

        gl_Position = vectorOut.vposition;
    }
]]

geometryDebugShader = [[
    #version 400
    layout (triangles) in;
    layout (line_strip, max_vertices = 2) out;

    in vectorOut {
        vec4 vposition;
        vec3 vnormal;
    } vectorIn[];

    const float MAGNITUDE = 0.2;

    void main()
    {
        vec3 dir1 = (vec3(vectorIn[1].vposition - vectorIn[0].vposition));
        float hl1 = length(dir1) / 2.0;
        dir1 = normalize(dir1);
        vec3 dir2 = (vec3(vectorIn[2].vposition - vectorIn[0].vposition));
        float hl2 = length(dir2) / 2.0;
        dir2 = normalize(dir2);
        vec3 nor1 = normalize(cross(dir2, dir1));

        vec4 midp = vectorIn[0].vposition;
        if(hl1 > hl2)
        {
            midp = midp + vec4(dir1*hl1, 0);
        }
        else
        {
            midp = midp + vec4(dir2*hl2, 0);
        }
        gl_Position = midp;
        EmitVertex();

        gl_Position = midp + vec4(nor1, 0) * MAGNITUDE;
        EmitVertex();
        EndPrimitive();
    } 
]]

fragmentDebugShader = [[
    #version 400
    uniform vec4 normalColor;
    out vec4 frag_colour;

    void main()
    {
        frag_colour = normalColor;
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
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 7.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 0.0, 9.0}, dir = {0.0, 0.0, 1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

normalColor = {255 / 255, 255 / 255, 10 / 255, 1.0}