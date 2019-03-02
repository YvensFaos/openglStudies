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
        frag_colour = vec4(color.rgb, 0.5);
    }
]]

colorShader = [[
    #version 400

    uniform vec4 color;
    out vec4 frag_colour;
    void main() {
        frag_colour = color;
    }
]]

castShader = [[
    #version 400

    in vec2 texCoord;
    out vec4 frag_colour;

    uniform float offsetSpace = 20.0;
    uniform sampler2D texture_diffuse1;
    uniform sampler2D texture_diffuse2;

    void main() {
        float offset = 1.0 / offsetSpace;

        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right    
        );

        float kernel[9] = float[](
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        );

        vec4 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = texture(texture_diffuse1, texCoord.st + offsets[i]);
        }
        vec4 col = vec4(0.0);
        for(int i = 0; i < 9; i++)
        {
            col += sampleTex[i] * kernel[i];
        }
        frag_colour = col + texture(texture_diffuse2, texCoord);
    }
]]

shadowFragmentShader = [[
    #version 400

    in vec2 texCoord;
    out vec4 frag_colour;

    uniform sampler2D texture_diffuse1;
    uniform sampler2D texture_specular1;

    void main() {
        vec4 tex = texture(texture_diffuse1, texCoord);
        float alpha = tex.x < 0.3 ? 0.0 : 1.0;
        frag_colour = vec4(tex.x, 0.0, 0.0, alpha);
    }
]]

monkey = "monkey.fbx"
plane = "plane.fbx"