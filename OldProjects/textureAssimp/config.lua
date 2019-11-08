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

fragmentShader = [[
    #version 400

    in vec2 texCoord;
    out vec4 frag_colour;

    uniform sampler2D texture_diffuse1;
    uniform sampler2D texture_specular1;

    void main() {
        vec4 diffuse = texture(texture_diffuse1, texCoord);
        vec4 specular = texture(texture_specular1, texCoord);
        vec4 color = diffuse + specular;
        frag_colour = vec4(color.rgb, 1.0);
    }
]]

model = "monkey.obj"