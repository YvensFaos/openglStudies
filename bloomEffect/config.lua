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

hdrGammaCorrection = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform;

    uniform float exposure = 0.32;
    uniform float gamma = 2.2;

    out vec4 frag_colour;

    void main() {
        vec3 hdrColor = texture(textureUniform, vuv).rgb;
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));

        frag_colour = vec4(mapped, 1.0);
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
    uniform float gamma = 2.2;
    
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

        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        vec3 viewDir = normalize(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        for(int i = 0; i < clampedNumberOfPoint; i++) 
        {
            pointLightResultant += calculatePointLight(i, norm, viewDir);
        }
        vec4 resultantLight = pointLightResultant;
        resultantLight.a = 1.0;

        resultantLight.rgb = pow(resultantLight.rgb, vec3(1.0/gamma));

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

        vec3 original = xyz2rgb(xyz);
        float luminance = 0.2126 * original.r + 0.7152 * original.g + 0.0722 * original.b;

        if(luminance < 300.02) {
            frag_colour = vec4(original, 1.0);
        } else {
            frag_colour = vec4(0.0);
        }
    }
]]

thresholdShader = [[
    #version 400
    in vec2 vuv;

    uniform sampler2D textureUniform;
    uniform float threshold;
    vec3 rgb2xyz(vec3 c) {
        float R = ((c.r > 0.04045) ? pow((( c.r + 0.055 ) / 1.055), 2.4) : (c.r / 12.92)) * 100.0;
        float G = ((c.g > 0.04045) ? pow((( c.g + 0.055 ) / 1.055), 2.4) : (c.g / 12.92)) * 100.0;
        float B = ((c.b > 0.04045) ? pow((( c.b + 0.055 ) / 1.055), 2.4) : (c.b / 12.92)) * 100.0;
        float X = R * 0.4124 + G * 0.3576 + B * 0.1805;
        float Y = R * 0.2126 + G * 0.7152 + B * 0.0722;
        float Z = R * 0.0193 + G * 0.1192 + B * 0.9505;
        return vec3(X, Y, Z);
    }

    out vec4 frag_colour;

    void main()
    {
        vec4 original = texture(textureUniform, vuv);
        float luminance = 0.2126 * original.r + 0.7152 * original.g + 0.0722 * original.b;
        frag_colour = luminance * original;
    }
]]

gaussianWeight = 16

gaussianHeader = [[
    #version 400
    in vec2 vuv;

    const int GAUSSIAN_WEIGHT = ]] .. gaussianWeight .. [[;
    const float factor = 1.0f;
    
    uniform sampler2D textureUniform;
    uniform float gaussianWeight[ ]] .. gaussianWeight .. [[ ];

    out vec4 frag_colour;

    void main() {
        vec2 step = vec2(1.0f) / textureSize(textureUniform, 0); //vec2(1.0f / 600.0f, 1.0f / 800.0f);//
]]

gaussianBlur1 = gaussianHeader .. [[
        vec3 xPass = texture(textureUniform, vuv).rgb * gaussianWeight[0];

        for(int i = 1; i < GAUSSIAN_WEIGHT; i++) {
            xPass += texture(textureUniform, vuv + vec2(factor * +i * step.x, 0.0f)).rgb * gaussianWeight[i];
            xPass += texture(textureUniform, vuv - vec2(factor * +i * step.x, 0.0f)).rgb * gaussianWeight[i];
        }
        frag_colour = vec4(xPass, 1.0);
    }
]]

gaussianBlur2 = gaussianHeader .. [[
        vec3 yPass = texture(textureUniform, vuv).rgb * gaussianWeight[0];
        for(int i = 1; i < GAUSSIAN_WEIGHT; i++) {
            yPass += texture(textureUniform, vuv + vec2(0.0f, factor * +i * step.y)).rgb * gaussianWeight[i];
            yPass += texture(textureUniform, vuv - vec2(0.0f, factor * +i * step.y)).rgb * gaussianWeight[i];
        }
        frag_colour = vec4(yPass, 1.0);
    }
]]

combineShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform float bloomFactor = 1.0f;

    out vec4 frag_colour;

    void main() {
        const float gamma = 2.2;
        const float exposure = -0.54;

        vec3 hdrColor = texture(textureUniform2, vuv).rgb;
        vec3 bloomColor = texture(textureUniform1, vuv).rgb;
        frag_colour = vec4(hdrColor + bloomFactor * bloomColor, 1.0);
    }
]]

bloomFactor = 0.9

initialMask = 255
gaussianBlurPasses = 4

downsample = 0.5

normalGamma = 0.7
normalExposure = -0.65

highGamma = 0.2
highExposure = -0.65

sigma = 16.0

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
        pos = { 0.0, 0.5, 4.0}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {20 / 255, 215 / 255, 215 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 256.0, 
        directional = false
    }
} 

cameraPosition = {
    pos   = {4.575, -0.512, 12.081},
    dir   = {-0.395, 0.098, -0.914},
    up    = {0.039, 0.995, 0.090},
    right = {0.918, 0.000, -0.396},
    angle = {-113.354, 5.633}
    -- pos   = {2.481, 1.915, 10.361},
    -- dir   = {-0.348, -0.207, -0.915},
    -- up    = {-0.073, 0.978, -0.193},
    -- right = {0.935, 0.000, -0.355},
    -- angle = {-110.806, -11.928}
    -- pos   = {-10.590, 3.997, 21.858},
    -- dir   = {0.636, 0.065, -0.769},
    -- up    = {-0.041, 0.998, 0.050},
    -- right = {0.771, -0.000, 0.637},
    -- angle = {-50.439, 3.734}
    -- pos   = {0.763, 0.829, 6.221},
    -- dir   = {-0.235, -0.211, -0.949},
    -- up    = {-0.051, 0.977, -0.205},
    -- right = {0.971, 0.000, -0.240},
    -- angle = {-103.913, -12.181}
    -- pos   = {2.655, 1.979, 12.746},
    -- dir   = {-0.178, -0.124, -0.976},
    -- up    = {-0.022, 0.992, -0.122},
    -- right = {0.984, 0.000, -0.179},
    -- angle = {-100.311, -7.140}
    -- pos   = {-7.316, 2.398, 12.478},
    -- dir   = {0.498, -0.152, -0.854},
    -- up    = {0.077, 0.988, -0.131},
    -- right = {0.864, -0.000, 0.504},
    -- angle = {-59.752, -8.754}
    -- pos   = {-13.097, 9.210, 9.666},
    -- dir   = {0.729, -0.464, -0.503},
    -- up    = {0.382, 0.886, -0.263},
    -- right = {0.567, -0.000, 0.823},
    -- angle = {-34.569, -27.662}
    -- pos   = {1.340, 6.233, 14.145},
    -- dir   = {-0.057, -0.347, -0.936},
    -- up    = {-0.021, 0.938, -0.346},
    -- right = {0.998, 0.000, -0.061},
    -- angle = {-93.501, -20.277}
    -- pos   = {-2.709, 5.704, 12.595},
    -- dir   = {0.250, -0.331, -0.910},
    -- up    = {0.088, 0.944, -0.319},
    -- right = {0.964, -0.000, 0.265},
    -- angle = {-74.656, -19.348}
}