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

    out vec3 Normal;
    out vec3 Position;
    
    uniform mat4 model;
    uniform mat4 viewProjection;
    
    void main()
    {
        Normal = mat3(transpose(inverse(model))) * normal;
        Position = vec3(model * vec4(vertex, 1.0));
        gl_Position = viewProjection * model * vec4(vertex, 1.0);
    }  
]]

fragmentShader = [[
    #version 400

    in vec3 Normal;
    in vec3 Position;

    uniform float refractiveIndex;
    uniform vec3 cameraPos;
    uniform samplerCube skybox;

    out vec4 frag_colour;

    void main()
    {             
        float ratio = 1.00 / refractiveIndex;
        vec3 I = normalize(Position - cameraPos);
        vec3 R = refract(I, normalize(Normal), ratio);
        frag_colour = vec4(texture(skybox, R).rgb, 1.0);
    }
]]

air = 1.0
water = 1.33
ice = 1.309
glass = 1.52
diamond = 2.42

refractiveIndex = air
model = "monkey.fbx"