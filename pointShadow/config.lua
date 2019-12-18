vertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    layout (location = 3) in vec3 tangent;
    
    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        float specularPower;
        bool directional;
    };

    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec3 vtangent;

        vec3 tangentLightPos;
        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorOut;

    uniform mat4 model;
    uniform mat4 viewProjection;
    uniform vec3 viewPosition;
    uniform Light sceneLight;

    out vec3 vPosition;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vuv = uv;

        mat3 normalMatrix = transpose(inverse(mat3(model)));
        vectorOut.vtangent = normalize(normalMatrix * tangent);
        vectorOut.vnormal = normalize(normalMatrix * normal);

        vectorOut.tangentLightPos = sceneLight.position;
        vectorOut.tangentViewPos  = viewPosition;
        vectorOut.tangentFragPos  = vectorOut.vposition;
        
        vPosition = viewPosition;
        gl_Position = viewProjection * vec4(vectorOut.vposition, 1.0);
    }  
]]

fragmentShader = [[
    #version 400

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        float specularPower;
        bool directional;
    };

    struct AmbientLight {
        vec4 color;
        float intensity;
    };

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec3 vtangent;

        vec3 tangentLightPos;
        vec3 tangentViewPos;
        vec3 tangentFragPos;
    } vectorIn;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 50.0f;

    vec3 gridSamplingDisk[20] = vec3[]
    (
        vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
        vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
        vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
        vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
    );

    uniform samplerCube depthMap;
    uniform AmbientLight sceneAmbientLight;
    uniform Light sceneLight;
    uniform float farPlane;

    in vec3 vPosition;

    out vec4 frag_colour;

    void main()
    {          
        vec3 normal = normalize(vectorIn.vnormal);
        float distance = length(sceneLight.position - vectorIn.vposition);

        //Attenuation calculation
        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        float flightConstant  = lightConstant  / attenuationIntensity;
        float flightLinear    = lightLinear    / attenuationIntensity;
        float flightQuadratic = lightQuadratic / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

        //Shadow calculation
        vec3 fragToLight = vectorIn.vposition - sceneLight.position; 
        float currentDepth = length(fragToLight); 
        float shadow = 0.0;
        float bias = 0.15;
        int samples = 20;
        float viewDistance = length(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        float diskRadius = (1.0 + (viewDistance / farPlane)) / 55.0;
        float closestDepth = 0.0f;
        for(int i = 0; i < samples; ++i)
        {
            closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
            closestDepth *= farPlane;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
        shadow /= float(samples);

        vec3 lightDirection = normalize(vectorIn.tangentLightPos - vectorIn.tangentFragPos);

        //Diffuse calculation
        float diff = max(dot(lightDirection, normal), 0.0);
        vec3 diffuse = vec4(attenuation * diff * sceneLight.color).rgb;
        vec4 diffuseColor = vec4(diffuse, sceneLight.color.w);

        //Specular calculation
        vec3 viewDir = normalize(vectorIn.tangentViewPos - vectorIn.tangentFragPos);
        vec3 reflectDir = reflect(-lightDirection, normal);
        float inc = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(inc, sceneLight.specularPower);
        vec3 specular = attenuation * vec3(spec);
        vec4 specularColor = vec4(specular.rgb, 1.0);
        
        //Ambient calculation
        vec4 ambientColor = vec4(sceneAmbientLight.intensity * sceneAmbientLight.color.rgb, 1.0);

        vec4 finalColor = ambientColor + (1.0 - shadow) * (diffuseColor + specularColor);
        finalColor.a = 1.0;

        frag_colour = finalColor;
    }
]]

ashadowVertexShader = [[
    #version 400
    layout (location = 0) in vec3 vertex;
    
    uniform mat4 model;
    
    void main()
    {
        gl_Position = model * vec4(vertex, 1.0);
    }  
]]

ashadowGeometryShader = [[
    #version 400
    layout (triangles) in;
    layout (triangle_strip, max_vertices=18) out;

    uniform mat4 shadowMatrices[6];

    out vec4 fragPos; // FragPos from GS (output per emitvertex)

    void main()
    {
        for(int face = 0; face < 6; ++face)
        {
            gl_Layer = face; // built-in variable that specifies to which face we render.
            for(int i = 0; i < 3; ++i) // for each triangle's vertices
            {
                fragPos = gl_in[i].gl_Position;
                gl_Position = shadowMatrices[face] * fragPos;
                EmitVertex();
            }    
            EndPrimitive();
        }
    }  
]]

ashadowFragmentShader = [[
    #version 400
    in vec4 fragPos;

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };
    
    uniform Light sceneLight;
    uniform float farPlane;

    void main()
    {
        // get distance between fragment and light source
        float lightDistance = length(fragPos.xyz - sceneLight.position);
        
        // map to [0:1] range by dividing by far_plane
        lightDistance = lightDistance / farPlane;
        
        // write this as modified depth
        gl_FragDepth = lightDistance;
    }  
]]

depthCubemapFragmentShader = [[
    #version 400
    in vec2 vuv;

    uniform samplerCube textureUniform;
    uniform float width;
    uniform float height;

    out vec4 frag_colour;

    void main()
    {       
        float value = 0.0;
        vec3 tint = vec3(0.0);
        if(gl_FragCoord.x < width / 3.0) {
            if(gl_FragCoord.y < height / 2.0) {  value = texture(textureUniform, vec3(0.0, vuv * 3.0)).r; tint = 0.2 * vec3(1.0, 0.0, 0.0);
            } else {                             value = texture(textureUniform, vec3(1.0, vuv * 3.0)).r; tint = 0.4 * vec3(1.0, 0.0, 0.0); }
        } else if(gl_FragCoord.x < (2.0 * width) / 3.0) {
            if(gl_FragCoord.y < height / 2.0) { value = texture(textureUniform, vec3(vuv * 3.0, 0.0)).r; tint = 0.2 * vec3(0.0, 1.0, 0.0);
            } else {                            value = texture(textureUniform, vec3(vuv * 3.0, 1.0)).r; tint = 0.4 * vec3(0.0, 1.0, 0.0); }
        } else {
            if(gl_FragCoord.y < height / 2.0) { value = texture(textureUniform, vec3(vuv.x * 3.0, 0.0, vuv.y * 3.0)).r; tint = 0.2 * vec3(0.0, 0.0, 1.0);
            } else {                            value = texture(textureUniform, vec3(vuv.x * 3.0, 1.0, vuv.y * 3.0)).r; tint = 0.4 * vec3(0.0, 0.0, 1.0); }
        }

        frag_colour = vec4(vec3(value) + tint, 1.0);
    }
]]

models = {}
index = 0

function incrementIndex()
    index = index + 1
    return index
end

models[incrementIndex()] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0, 0.0,  2.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[incrementIndex()] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0, 0.0, -2.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[incrementIndex()] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0, 2.0, -4.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}
models[incrementIndex()] = {file = "../3DModels/plane100x100.obj", pos = { 0.0, -3.0, 0.0}, sca = {5.0, 1.0, 5.0}, rot = {15.0, 0.0, 0.0}}
models[incrementIndex()] = {file = "../3DModels/plane100x100.obj", pos = {0.0, 3.1, -4.95}, sca = {5.0, 1.0, 5.0}, rot = {90.0, 0.0, 0.0}}
models[incrementIndex()] = {file = "../3DModels/plane100x100.obj", pos = {-5.0, 1.1, 0}, sca = {5.0, 1.0, 7.0}, rot = {90.0, 0.0, -90.0}}

shadowFov = 90.0
shadowWidth =  1024
shadowHeight = 1024
shadowNear = 0.1
shadowFar = 25.0

initialMask = 255

ambient = {
    col = {255 / 255, 255 / 255, 255 / 255, 1.0}, 
    intensity = 0.1
}

lightIntensity = 100
lights = {
    {
        pos = {-0.5, 0.0, -0.5}, 
        dir = {0.0, 0.0, 1.0}, 
        up = {0.0, 1.0, 0.0}, 
        col = {0.0 / 255, 118 / 255, 252 / 255, 1.0}, 
        intensity = lightIntensity, 
        specularPower = 16.0, 
        directional = false
    }
}

cameraPosition = {
    pos   = {11.540, 3.409, 8.260},
    dir   = {-0.792, -0.174, -0.585},
    up    = {-0.140, 0.985, -0.104},
    right = {0.594, 0.000, -0.805},
    angle = {-143.571, -10.049}
}

-- Update Functions 

sign = 1
function updateLight(px, py, pz, dx, dy, dz, ux, uy, uz, cr, cg, cb, ca, intensity, specular, delta, accumulator)
    ppx = px + sign * delta * 1.3
    if ppx > 4 then
        sign = -1
    end
    if ppx < -4 then
        sign = 1
    end
    ppy = py + math.sin(sign * delta * 0.2)
    return ppx, ppy, pz, dx, dy, dz, ux, uy, uz, cr, cg, cb, ca, intensity, specular
end