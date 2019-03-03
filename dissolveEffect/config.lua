vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;

    uniform mat4 mvpMatrix;

    out vec2 texCoord;

    void main() {
        texCoord = uv;
        gl_Position = mvpMatrix * vec4(vertex, 1.0);
    }
]]

dissolveFragmentShader = [[
    #version 400

    uniform vec3 baseColor;
    uniform float threshold;
    uniform float thickness;
    in vec2 texCoord;
    out vec4 frag_colour;

    uniform sampler2D texture_diffuse1;

    void main() {
        vec4 tex = texture(texture_diffuse1, texCoord);

        vec4 alphaC = tex.x < threshold + thickness ? vec4(0.0) : vec4(1.0 - threshold, 0.0, 0.0, 1.0);
        vec4 stepC = tex.x <= threshold + thickness && tex.x >= threshold  ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0);
        
        frag_colour = alphaC + stepC;
    }
]]

model = "monkey.fbx"
thickness = 0.05

function updateThreshold(threshold, deltaTime)
    timer = math.asin(threshold) + deltaTime * 0.125
    newThreshold = math.sin(timer)
    if newThreshold > 0.9990 then
        newThreshold = 0
    end
    return newThreshold
end