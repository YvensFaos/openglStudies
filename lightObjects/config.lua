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
        vectorOut.lposition = vec4(model * vec4(vertex, 1.0));
        gl_Position = projection * view * model * vec4(vertex, 1.0);
    }  
]]

lightFragmentShader = [[
    #version 400
    uniform vec4 lightColor;
    uniform vec3 lightDirection;

    out vec4 frag_colour;

    void main()
    {          
        frag_colour = vec4(lightDirection, 1.0);
    }
]]

geometryShader = [[
    #version 400
       layout (triangles) in;
       layout (line_strip, max_vertices = 2) out;
       
       in vectorOut {
           vec4 lposition;
       } vectorIn[];

       const float MAGNITUDE = 0.4;
       void main()
       {
           vec3 dir1 = (vec3(vectorIn[1].lposition - vectorIn[0].lposition));
           float hl1 = length(dir1) / 2.0;
           dir1 = normalize(dir1);
           vec3 dir2 = (vec3(vectorIn[2].lposition - vectorIn[0].lposition));
           float hl2 = length(dir2) / 2.0;
           dir2 = normalize(dir2);
           vec3 nor1 = normalize(cross(dir2, dir1));
           vec4 midp = vectorIn[0].lposition;

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
           vec4 finp = midp + vec4(0, 0, -1, 0) * MAGNITUDE;
           gl_Position = finp;
           EmitVertex();

           //vec3 direction = lightDirection - vec3(midp);
           //direction = normalize(direction);
           //gl_Position = finp;
           //EmitVertex();
           //gl_Position = finp + vec4(cross(vec3(MAGNITUDE, 0, 0), direction), 0) * MAGNITUDE;
           //EndPrimitive();
           //gl_Position = finp;
           //EmitVertex();
           //gl_Position = finp + vec4(cross(direction, vec3(MAGNITUDE, 0, 0)), 0) * MAGNITUDE;
           //EndPrimitive();
           //gl_Position = finp;
           //EmitVertex();
           //gl_Position = finp + vec4(cross(vec3(0, MAGNITUDE, 0), direction), 0) * MAGNITUDE;
           //EndPrimitive();
           //gl_Position = finp;
           //EmitVertex();
           //gl_Position = finp + vec4(cross(vec3(0, MAGNITUDE, 0), direction), 0) * MAGNITUDE;
           //EndPrimitive();
       }
]]

cameraPosition = {
    pos =   {-1.961, -0.053, 8.617},
    dir =   {0.414, -0.021, -0.910},
    up =    {0.009, 1.000, -0.019 },
    right = {0.910, -0.000, 0.414 }}


lightModel = "../3DModels/cube.obj"

posInitial = 5.0
lightObject = {file = "../3DModels/tsphere.fbx", pos = {0.0,  0.0, posInitial}, sca = {0.1, 0.1, 0.1}, rot = {0.0,  0.0, 0.0}}

pointLight = 
    {pos = {0.0, 0.0, posInitial}, dir = {0.0, 0.0, -1.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 0 / 255, 1.0}, 
    intensity = 90, 
    specularPower = 256.0, 
    directional = true}

accumulator = 0
function updateLight(deltaTime, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    accumulator = accumulator + deltaTime
    nposx = posx
    nposy = posy
    nposz = posInitial + 0.35 + 1.5 * math.sin(math.rad(accumulator) * 100.0)
    return intensity, cola, colb, colg, colr, upz, upy, upx, dirz, diry, dirx, nposz, nposy, nposx
end

debug = false