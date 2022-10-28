vertexShader = [[
    #version 330 core
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vec3 FragPos;
    out vec2 TextCoords;
    out vec3 Normal;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main()
    {
        vec4 worldPos = model * vec4(vertex, 1.0);
        FragPos = worldPos.xyz;

        Normal = mat3(transpose(inverse(model))) * normal;
        TextCoords = uv;

        gl_Position = viewProjection * worldPos;
    }  
]]

fragmentShader = [[
    #version 330 core
    layout (location = 0) out vec4 gPosition;
    layout (location = 1) out vec4 gNormal;
    layout (location = 2) out vec4 gAlbedoSpec;

    in vec3 FragPos;
    in vec2 TextCoords;
    in vec3 Normal;

    void main()
    {            
        gPosition = vec4(FragPos, 1.0);
        gNormal = vec4(normalize(Normal), 1.0f);
        gAlbedoSpec = vec4(0.0f, 1.0f, 1.0f, 1.0f);
    } 
]]

gFragmentShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform0; // POSITION
    uniform sampler2D textureUniform1; // NORMAL
    uniform sampler2D textureUniform2; // ALBEDO

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 200.0f;

    uniform Light lights[10];

    out vec4 frag_colour;

    vec3 calculateDirectionalLight(Light light, vec3 pos, vec3 norm, vec3 alb) {
        float distance = length(light.position - pos);
        float attenuationIntensity = light.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, light.direction), 0.0);

        return attenuation * diff * light.color.xyz * alb;
    }

    vec3 calculatePointLight(Light light, vec3 pos, vec3 norm, vec3 alb) {
        vec3 directionV = light.position - pos;
        float distance = length(directionV);
        float attenuationIntensity = light.intensity / maximumIntensity;
        lightConstant = 1.0f   / attenuationIntensity;
        lightLinear = 0.09f    / attenuationIntensity;
        lightQuadratic = 0.032f/ attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        vec3 lightDir = normalize(directionV);
        float diff = max(dot(norm, lightDir), 0.0);

        return attenuation * diff * light.color.xyz * alb;
    }

    void main() {
        vec3 pos = vec3(texture(textureUniform0, vuv.st));
        vec3 norm = normalize(vec3(texture(textureUniform1, vuv.st)));
        vec3 alb = vec3(texture(textureUniform2, vuv.st));

        vec3 finalColor = vec3(0,0,0);
        for(int i = 0; i < 10; i++) {
            vec3 diffuse = vec3(0,0,0);
            if(lights[i].directional) {
                diffuse = calculateDirectionalLight(lights[i], pos, norm, alb);
            } else {
                diffuse = calculatePointLight(lights[i], pos, norm, alb);
            }
            
            finalColor = finalColor + diffuse;
        }
        
        frag_colour = vec4(finalColor, 1.0);
    }
]]

kernelsSize = 64
noisesSize = 16

models = {}
models[1] = {file = "../3DModels/scene01.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

lights = {
    {pos = {3.729, -10.005, 5.615}, dir = {-0.253, 0.949, -0.191}, up = {0.758, 0.317, 0.571}, 
    col = {255 / 255, 255 / 255, 255 / 255, 1.0}, 
    intensity = 200,
    specularPower = 256.0, 
    directional = true},

    {pos = {-8.790, -1.438, 10.494}, dir = {0.717, 0.359, -0.598}, up = {-0.275, 0.934, 0.230}, 
    col = {255 / 255, 0 / 255, 255 / 255, 1.0}, 
    intensity = 200, 
    specularPower = 256.0, 
    directional = true},

    {pos = {0.0, 0.0, 1.0}, dir = {0.0, -1.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 255 / 255, 1.0}, 
    intensity = 200, 
    specularPower = 256.0, 
    directional = true},

    {pos = {0.0, 0.0, 1.0}, dir = {0.0, 1.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {125 / 255, 255 / 255, 30 / 255, 1.0}, 
    intensity = 200, 
    specularPower = 256.0, 
    directional = true},

    {pos = {3.891, 0.012, 6.981}, dir = {0.0, 0.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 255 / 255, 1.0}, 
    intensity = 200, 
    specularPower = 256.0, 
    directional = false},

    {pos = {2.0, 2.0, 0.0}, dir = {-0.992, 0.022, 0.123}, up = {0.0, 1.0, 0.0}, 
    col = {227 / 255, 118 / 255, 100 / 255, 1.0}, 
    intensity = lightIntensity,
    specularPower = 256.0, 
    directional = false},
    
    {pos = {0.0, 0.0, 0.0}, dir = {0.0, 0.0, 0.0}, up = {0.0, 0.0, 0.0}, 
    col = {255 / 255, 255 / 255, 255 / 255, 1.0}, 
    intensity = 200, 
    specularPower = 256.0, 
    directional = false},

    {pos = {2.0, 4.0, 0.0}, dir = {-0.513, -0.576, -0.637}, up = {0.0, 0.0, 0.0}, 
    col = {0 / 255, 255 / 255, 0 / 255, 1.0}, 
    intensity = 70, 
    specularPower = 256.0, 
    directional = true},

    {pos = {-10.222, 7.357, 9.484}, dir = {0.825, -0.395, -0.404}, up = {0.354, 0.919, -0.173}, 
    col = {125 / 255, 125 / 255, 125 / 255, 1.0}, 
    intensity = 170, 
    specularPower = 256.0, 
    directional = true},

    {pos = {8.632, 7.064, 10.020}, dir = {-0.637, -0.513, -0.576}, up = {-0.380, 0.859, -0.344}, 
    col = {125 / 255, 125 / 255, 125 / 255, 1.0}, 
    intensity = 170, 
    specularPower = 256.0, 
    directional = true}
}

cameraPosition = {
    pos   = {8.632, 7.064, 10.020},
    dir   = {-0.637, -0.513, -0.576},
    up    = {-0.380, 0.859, -0.344},
    right = {0.670, 0.000, -0.742},
    angle = {-137.895, -30.839}
}