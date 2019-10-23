simpleVertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main()
    {
        gl_Position = viewProjection * model * vec4(vertex, 1.0);
    }  
]]

simpleFragmentShader = [[
    #version 400

    uniform vec4 color;

    out vec4 frag_colour;

    void main()
    {          
        frag_colour = color;
    }
]]

models = {}
models[1] = {file = "../3DModels/cube.obj", pos = { 0.0,  0.0, 0.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lightIntensity = 100
light = {pos = { 0.0, 4.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, col = {227 / 255, 118 / 255, 252 / 255, 1.0}, intensity = lightIntensity, specularPower = 256.0, directional = true}

raysPerUnit = 50
pointsPerUnit = 25

rcolorR = 0.0
rcolorG = 0.0
rcolorB = 0.0
rcolorA = 255.0

mcolorR = 0.0
mcolorG = 0.0
mcolorB = 0.0
mcolorA = 0.0

cameraPosition = {
    pos   = {1.122, 0.020, 6.186},
    dir   = {-0.231, 0.032, -0.972},
    up    = {0.008, 0.999, 0.032},
    right = {0.973, 0.000, -0.231}
}