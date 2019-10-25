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

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    uniform float time;

    const float THRESHOULD = 0.025;

    const float DETAILS = 0.10;
    const float ELEVATN = 0.13;
    const float TERRAIN = 0.65;

    float height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;

        return uvP.x;
    }

    void main()
    {
        float realHeight = height(uv);

        float hL = height(uv - vec2(THRESHOULD, 0.0));
        float hR = height(uv + vec2(THRESHOULD, 0.0));
        float hD = height(uv - vec2(0.0, THRESHOULD));
        float hU = height(uv + vec2(0.0, THRESHOULD));
        
        vec3 hposition = vertex;
        hposition.y = realHeight;
        
        vec3 cnormal = vec3(hL - hR, hD - hU, 0.6);
        cnormal = normalize(cnormal);
        
        vectorOut.vposition = vec3(model * vec4(hposition, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * cnormal;
        vectorOut.vuv = uv;

        vec4 vecOut = vec4(vectorOut.vposition, 1.0);
        gl_Position = viewProjection * vecOut;
    }  
]]

fragmentShader = [[
    #version 400

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorIn;

    out vec4 frag_colour;

    void main()
    {          
        vec4 color = vec4(vectorIn.vnormal, 0.0); // + vec4(vectorIn.vuv, 0.0, 0.0);
        color.a = 1.0;
        frag_colour = color;
    }
]]

models = {}
models[1] = {file = "../3DModels/plane100x100.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

cameraPosition = {
    pos   = {-0.047, 2.920, 5.758},
    dir   = {-0.000, -0.681, -0.732},
    up    = {-0.000, 0.732, -0.681},
    right = {1.000, 0.000, -0.000}
}