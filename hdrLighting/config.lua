vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    uniform vec3 viewPosition;

    out vectorOut {
        vec3 vposition;
        vec3 vnormal;

        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorOut;

    out mat4 modelMatrix;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        modelMatrix = model;

        vectorOut.vnormal = normal;
        vectorOut.tangentViewPos = viewPosition;
        vectorOut.tangentFragPos = vectorOut.vposition;
    
        gl_Position = projection * view * model * vec4(vertex, 1.0);
    }
]]

multiLightFragmentShader = [[
    #version 400

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        float specularPower;
        bool directional;
    };

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;

        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorIn;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform int numberPointLights;
    uniform Light pointLights[10];
    
    out vec4 frag_colour;

    vec4 calculatePointLight(int index, const vec3 norm, const vec3 viewDir) {
        vec3 directionV = normalize(pointLights[index].position - vectorIn.tangentFragPos);

        float distance = length(pointLights[index].position - vectorIn.vposition);
        float attenuationIntensity = pointLights[index].intensity / maximumIntensity;
        float attenuation = 1.0 / (lightConstant / attenuationIntensity + lightLinear * distance / attenuationIntensity + lightQuadratic * (distance * distance) / attenuationIntensity);
        
        float diff = max(dot(norm, directionV), 0.0);

        vec3 diffuse =  attenuation * diff * pointLights[index].color.rgb;

        vec3 reflectDir = reflect(-directionV, norm);
        float inc = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(inc, pointLights[index].specularPower);
        vec4 specular = attenuation * vec4(vec3(spec), 1.0);
        specular.a = 1.0;

        return vec4(diffuse, 1.0) + specular;
    }

    in mat4 modelMatrix;

    void main()
    {          
        vec3 norm = normalize(transpose(inverse(mat3(modelMatrix))) * vectorIn.vnormal);

        //vec3 norm = normalize(vectorIn.vnormal);

        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        vec3 viewDir = normalize(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        for(int i = 0; i < clampedNumberOfPoint; i++) 
        {
            pointLightResultant += calculatePointLight(i, norm, viewDir);
        }
        vec4 resultantLight = pointLightResultant;
        resultantLight.a = 1.0;

        frag_colour = resultantLight;
    }
]]

hdrShader = [[
    #version 400
    in vec2 vuv;

    uniform sampler2D textureUniform;
    uniform float logAve;
    uniform float exposure;

    vec3 rgb2xyz(vec3 c) {
        float R = ((c.r > 0.04045) ? pow((( c.r + 0.055 ) / 1.055), 2.4) : (c.r / 12.92)) * 100.0;
        float G = ((c.g > 0.04045) ? pow((( c.g + 0.055 ) / 1.055), 2.4) : (c.g / 12.92)) * 100.0;
        float B = ((c.b > 0.04045) ? pow((( c.b + 0.055 ) / 1.055), 2.4) : (c.b / 12.92)) * 100.0;

        float X = R * 0.4124 + G * 0.3576 + B * 0.1805;
        float Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
        float Z = R * 0.0193 + G * 0.1192 + B * 0.9505;

        return vec3(X, Y, Z);
    }

    vec3 xyz2rgb(vec3 c) {
        float X = c.x / 100.0;
        float Y = c.y / 100.0;
        float Z = c.z / 100.0;
        
        float R = X *  3.2406 + Y * -1.5372 + Z * -0.4986;
        float G = X * -0.9689 + Y *  1.8758 + Z *  0.0415;
        float B = X *  0.0557 + Y * -0.2040 + Z *  1.0570;
        
        R = ((R > 0.0031308) ? (1.055 * ( pow( R, 1./2.4 ) ) - 0.055) : (12.92 * R)); 
        G = ((G > 0.0031308) ? (1.055 * ( pow( G, 1./2.4 ) ) - 0.055) : (12.92 * G)); 
        B = ((B > 0.0031308) ? (1.055 * ( pow( B, 1./2.4 ) ) - 0.055) : (12.92 * B)); 
        
        return vec3(R, G, B);
    }

    vec3 xyz2xyY(vec3 xyz) {
        float xyzSum = xyz.x + xyz.y + xyz.z;
        vec3 xyYCol = vec3(0.0);
        if(xyzSum > 0.0) {
            xyYCol = vec3(xyz.x / xyzSum, xyz.y / xyzSum, xyz.y);
        }

        return xyYCol;
    }

    out vec4 frag_colour;

    void main()
    {
        vec3 xyz = rgb2xyz(vec3(texture(textureUniform, vuv)));
        vec3 xyY = xyz2xyY(xyz);

        float L = (exposure * xyY.z) / logAve;
        L = (L * (1 + L / 1.0)) / (1 + L);

        if(xyY.y > 0.0) {
            xyz.x = (L * xyY.x) / (xyY.y);
            xyz.y = L;
            xyz.z = (L * (1 - xyY.x - xyY.y)) / xyY.y;
        }

        frag_colour = vec4(xyz2rgb(xyz), 1.0);
    }
]]

exposure = 0.32

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,   0.0, 0.0}}
models[2] = {file = "../3DModels/nonormalmonkey.obj", pos = { 2.0,  0.0, 1.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  20.0, 0.0}}
models[3] = {file = "../3DModels/nonormalmonkey.obj", pos = {-2.0,  0.0, 1.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0, -20.0, 0.0}}
models[4] = {file = "../3DModels/brick.obj", pos = {0.0,  -2.0, -0.0}, sca = {4.0, 4.0, 4.0}, rot = {-70.0, 0.0, 0.0}}
models[5] = {file = "../3DModels/brick.obj", pos = {0.0, 2.5, -4.0}, sca = {4.0, 4.0, 4.0}, rot = {  0.0, 0.0, 0.0}}
models[6] = {file = "../3DModels/nonormaltsphere.obj", pos = {0.0, 3.0, -2.0}, sca = {1.2, 1.2, 1.2}, rot = { 0.0, 0.0, 0.0}}

lightIntensity = 100
lights = {
    {
        pos = { 0.0, 4.0, 0.0}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {227 / 255, 118 / 255, 252 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 32.0, 
        directional = false
    }
    ,
    {
        pos = { 2.0, -1.5, 2.0}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {118 / 255, 255 / 255, 227 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 256.0, 
        directional = false
    },
    {
        pos = {-2.0, -1.5, 2.0}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {255 / 255, 227 / 255, 118 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 256.0, 
        directional = false
    },
    {
        pos = { 0.0, 0.5, 6.0}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {20 / 255, 215 / 255, 215 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 256.0, 
        directional = false
    }
} 

cameraPosition = {
    pos   = {-2.709, 5.704, 12.595},
    dir   = {0.250, -0.331, -0.910},
    up    = {0.088, 0.944, -0.319},
    right = {0.964, -0.000, 0.265},
    angle = {-74.656, -19.348}
}