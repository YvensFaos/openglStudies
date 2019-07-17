vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    layout (location = 3) in vec3 tangent;

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
           vec3 direction = normalizedLightDirection - vec3(midp);
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
    pos =   {-0.022, -0.015, 5.621},
    dir =   {0.135, 0.027, -0.990},
    up =    {-0.004, 1.000, 0.027 },
    right = {0.991, -0.000, 0.135 }}

lightModel = "../3DModels/tetrahedron.obj"

posInitial = 5.0
initialDirX = 0.0;
lightObject = {file = "../3DModels/tsphere.fbx", pos = {0.0,  0.0, posInitial}, sca = {0.1, 0.1, 0.1}, rot = {0.0,  0.0, 0.0}}

pointLight = 
    {pos = {0.0, 0.0, posInitial}, dir = {0.0, -1.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 0 / 255, 1.0}, 
    intensity = 90, 
    specularPower = 256.0, 
    directional = true}

accumulator = 0
function updateLight(deltaTime, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    accumulator = accumulator + deltaTime
    nposx = posx
    nposy = posy
    nposz = posInitial -- + 0.35 + 1.5 * math.sin(math.rad(accumulator) * 50.0)
    ndirx = initialDirX -- + 0.5 * math.sin(math.rad(accumulator) * 20.0)
    ndiry = diry
    ndirz = initialDirX + 0.25 * math.cos(math.rad(accumulator) * 25.0)
    return intensity, cola, colb, colg, colr, upz, upy, upx, ndirz, ndiry, ndirx, nposz, nposy, nposx
end

debug = false