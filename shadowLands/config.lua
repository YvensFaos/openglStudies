vertexShader = [[
    #version 410 core
    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;
    
    out vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorOut;

    uniform mat4 model;

    void main()
    {
        vectorOut.vposition = vec3(model * vec4(vertex, 1.0));
        vectorOut.vnormal = mat3(transpose(inverse(model))) * normal;
        vectorOut.vuv = uv;
    }
]]

controlTesselationShader = [[
    #version 410 core

    layout (vertices = 3) out;

    in vectorOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } vectorIn[];

    out controlOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } controlOut[];

    uniform float maxTessLevel = 10.0;

    void main() {
        controlOut[gl_InvocationID].vposition = vectorIn[gl_InvocationID].vposition;
        controlOut[gl_InvocationID].vnormal = vectorIn[gl_InvocationID].vnormal;
        controlOut[gl_InvocationID].vuv = vectorIn[gl_InvocationID].vuv;

        gl_TessLevelOuter[0] = maxTessLevel;
        gl_TessLevelOuter[1] = maxTessLevel;
        gl_TessLevelOuter[2] = maxTessLevel;
        gl_TessLevelInner[0] = maxTessLevel;
    }                                                                     
]]

evaluationTesselationShader = [[
    #version 410 core

    layout(triangles, equal_spacing, ccw) in; 

    in controlOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } controlIn[];

    out evaluationOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vlightSpace;
    } evaluationOut;
    
    uniform mat4 viewProjection;
    uniform mat4 lightViewProjection;

    vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
    {
        return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
    }

    vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
    {
        return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
    }
 
    void main()
    { 
        evaluationOut.vposition = interpolate3D(controlIn[0].vposition, controlIn[1].vposition, controlIn[2].vposition);
        evaluationOut.vuv = interpolate2D(controlIn[0].vuv, controlIn[1].vuv, controlIn[2].vuv);
        evaluationOut.vnormal = interpolate3D(controlIn[0].vnormal, controlIn[1].vnormal, controlIn[2].vnormal);

        evaluationOut.vlightSpace = vec4(evaluationOut.vposition, 1.0); 
        gl_Position = vec4(evaluationOut.vposition, 1.0);
    }
]]

shadowEvaluationTesselationShader = [[
    #version 410 core

    layout(triangles, equal_spacing, ccw) in; 

    in controlOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } controlIn[];

    out evaluationOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } evaluationOut;
    
    vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
    {
        return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
    }

    vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
    {
        return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
    }
 
    void main()
    { 
        evaluationOut.vposition = interpolate3D(controlIn[0].vposition, controlIn[1].vposition, controlIn[2].vposition);
        evaluationOut.vuv = interpolate2D(controlIn[0].vuv, controlIn[1].vuv, controlIn[2].vuv);
        evaluationOut.vnormal = interpolate3D(controlIn[0].vnormal, controlIn[1].vnormal, controlIn[2].vnormal);

        gl_Position = vec4(evaluationOut.vposition, 1.0);
    }
]]

DETAILS = 0.45
ELEVATN = 1.35
TERRAIN = 5.45

geometryShader = [[
    #version 410 core

    layout(triangles) in;
    layout(triangle_strip, max_vertices = 3) out;

    in evaluationOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vlightSpace;
    } evaluationIn[];

    out geometryOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vlightSpace;
    } geometryOut;

    const float DETAILS = ]] .. DETAILS .. [[;
    const float ELEVATN = ]] .. ELEVATN .. [[;
    const float TERRAIN = ]] .. TERRAIN .. [[;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

    uniform float adjustHeight = 2.0;

    uniform mat4 viewProjection;
    uniform mat4 lightViewProjection;

    void main() {
        for(int i = 0; i < 4; i++) {
            vec3 calculatedHeight = height(evaluationIn[i].vuv);
            vec4 finalPosition = gl_in[i].gl_Position + vec4(0.0, calculatedHeight.y - adjustHeight, 0.0, 0.0);

            gl_Position = viewProjection * finalPosition;
            geometryOut.vposition = vec3(gl_Position);
            geometryOut.vnormal = evaluationIn[i].vnormal;
            geometryOut.vuv = evaluationIn[i].vuv;
            geometryOut.vlightSpace = lightViewProjection * finalPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
]]

shadowGeometryShader = [[
    #version 410 core

    layout(triangles) in;
    layout(triangle_strip, max_vertices = 3) out;

    in evaluationOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } evaluationIn[];

    out geometryOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } geometryOut;

    const float DETAILS = ]] .. DETAILS .. [[;
    const float ELEVATN = ]] .. ELEVATN .. [[;
    const float TERRAIN = ]] .. TERRAIN .. [[;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    uniform float adjustHeight = 2.0;
    uniform mat4 viewProjection;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

    void main() {
        for(int i = 0; i < 4; i++) {
            vec3 calculatedHeight = height(evaluationIn[i].vuv);
            vec4 finalPosition = gl_in[i].gl_Position + vec4(0.0, calculatedHeight.y - adjustHeight, 0.0, 0.0);

            gl_Position = viewProjection * finalPosition;
            geometryOut.vposition = vec3(gl_Position);
            geometryOut.vnormal = evaluationIn[i].vnormal;
            geometryOut.vuv = evaluationIn[i].vuv;
            EmitVertex();
        }
        EndPrimitive();
    }
]]

fragmentShader = [[
    #version 410 core

    struct AmbientLight {
        vec4 color;
        float intensity;
    };

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
        float specularPower;
    };

    in geometryOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
        vec4 vlightSpace;
    } geometryIn;

    uniform sampler2D textureUniform0;
    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;
    uniform sampler2D shadowMap;

    const float THRESHOULD = 0.05;
    const float SLOPE_FACTOR = 10.0;
    const float DETAILS = ]] .. DETAILS .. [[;
    const float ELEVATN = ]] .. ELEVATN .. [[;
    const float TERRAIN = ]] .. TERRAIN .. [[;

    float factorLightConstant = 1.0f;
    float factorLightLinear = 0.09f;
    float factorLightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform int numberPointLights = 0;
    uniform int numberDirectionLights = 0;

    uniform vec3 eyePosition;

    uniform Light pointLights[10];
    uniform Light directionalLights[10];
    uniform AmbientLight sceneAmbientLight;

    out vec4 frag_colour;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

    vec4 calculatePointLight(int index, const vec3 norm) {
        vec3 directionV = pointLights[index].position - geometryIn.vposition;
        float distance = length(directionV);
        float attenuationIntensity = pointLights[index].intensity / maximumIntensity;
        float lightConstant = factorLightConstant / attenuationIntensity;
        float lightLinear = factorLightLinear / attenuationIntensity;
        float lightQuadratic = factorLightQuadratic / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        vec3 lightDir = normalize(directionV);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse =  attenuation * diff * pointLights[index].color.rgb;
        vec3 viewDir = normalize(eyePosition - geometryIn.vposition);
        vec3 reflectDir = reflect(-directionV, norm);
        float inc = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(inc, pointLights[index].specularPower);
        vec4 specular = attenuation * vec4(vec3(spec), 1.0);
        specular.a = 1.0;

        return vec4(diffuse.rgb + specular.rgb, 1.0);
    }

    vec4 calculateDirectionalLight(int index, const vec3 norm) {
        vec3 directionV = directionalLights[index].position - geometryIn.vposition;
        float distance = length(directionV);
        float attenuationIntensity = directionalLights[index].intensity / maximumIntensity;
        float lightConstant = factorLightConstant / attenuationIntensity;
        float lightLinear = factorLightLinear / attenuationIntensity;
        float lightQuadratic = factorLightQuadratic / attenuationIntensity;
        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(-directionalLights[index].direction, norm), 0.0);
        vec3 diffuse =  attenuation * diff * directionalLights[index].color.rgb;
        vec3 viewDir = normalize(eyePosition - geometryIn.vposition);
        vec3 reflectDir = reflect(-directionV, norm);
        float inc = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(inc, directionalLights[index].specularPower);
        vec4 specular = attenuation * vec4(vec3(spec), 1.0);
        specular.a = 1.0;

        return vec4(diffuse.rgb + specular.rgb, 1.0);
    }

    float calculateShadows(vec4 vlightSpace, vec3 normal, vec3 ldirection) {
        vec3 projCoords = vlightSpace.xyz / vlightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;
        float currentDepth = projCoords.z;
        float bias = max(0.05 * (1.0 - dot(normal, ldirection)), 0.05);
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        float pcfDepth = 0.0;
        for(int x = -2; x <= 2; ++x) {
            for(int y = -2; y <= 2; ++y) {
                pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }    
        }
        shadow /= 9.0;
        if(projCoords.z > 1.0) {
            shadow = 0.0;
        }
        
        return shadow;
    }

    void main() {   
        vec3 co = height(geometryIn.vuv);
        vec3 hL = height(geometryIn.vuv - vec2(THRESHOULD, 0.0));
        vec3 hR = height(geometryIn.vuv + vec2(THRESHOULD, 0.0));
        vec3 hD = height(geometryIn.vuv - vec2(0.0, THRESHOULD));
        vec3 hU = height(geometryIn.vuv + vec2(0.0, THRESHOULD));
        
        vec3 cL = normalize(vec3(vec2(1.0, 0.0), (hR - hL).x * SLOPE_FACTOR));
        vec3 cD = normalize(vec3(vec2(0.0, 1.0), (hU - hD).x * SLOPE_FACTOR));
        vec3 cnormal = cross(cL, cD).xzy;

        vec4 directionalLightResultant = vec4(0.0);
        int clampedNumberOfDirectional = min(numberDirectionLights, 10);
        for(int i = 0; i < clampedNumberOfDirectional; ++i) {
            directionalLightResultant += calculateDirectionalLight(i, cnormal);
        }
        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        for(int i = 0; i < clampedNumberOfPoint; ++i) {
            pointLightResultant += calculatePointLight(i, cnormal);
        }

        float shadow = calculateShadows(geometryIn.vlightSpace, cnormal, directionalLights[0].direction);

        vec4 ambient = sceneAmbientLight.intensity * sceneAmbientLight.color * texture(textureUniform0, geometryIn.vuv);
        ambient.a = 0.0;

        vec4 resultantLight = ambient + (1.0 - shadow) * (directionalLightResultant * texture(textureUniform0, geometryIn.vuv));// + pointLightResultant * texture(textureUniform0, geometryIn.vuv));
        resultantLight.a = 1.0;

        if(gl_FrontFacing) {
            frag_colour = resultantLight;
        } else {
            frag_colour = vec4(1.00, 0.05, 0.05, 1.0);
        }
    }
]]

wireGeometryShader = [[
    #version 410 core

    layout(triangles) in;
    layout(line_strip, max_vertices = 4) out;

    in evaluationOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } evaluationIn[];

    out geometryOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } geometryOut;

    const float DETAILS = ]] .. DETAILS .. [[;
    const float ELEVATN = ]] .. ELEVATN .. [[;
    const float TERRAIN = ]] .. TERRAIN .. [[;

    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

    uniform float adjustHeight = 2.0;
    uniform mat4 viewProjection;

    vec3 height(vec2 position) {
        vec4 value1 = DETAILS * texture(textureUniform1, position);
        vec4 value2 = ELEVATN * texture(textureUniform2, position);
        vec4 value3 = TERRAIN * texture(textureUniform3, position);
        vec4 uvP = value1 + value2 + value3;
        vec3 h = vec3(uvP);
        return h;
    }

    void main() {
        for(int i = 0; i < 4; i++) {
            vec3 calculatedHeight = height(evaluationIn[i % 3].vuv);
            vec4 finalPosition = gl_in[i % 3].gl_Position + vec4(0.0, calculatedHeight.y - adjustHeight, 0.0, 0.0);
            
            gl_Position = viewProjection * finalPosition;
            geometryOut.vposition = vec3(gl_Position);
            geometryOut.vnormal = evaluationIn[i % 3].vnormal;
            geometryOut.vuv = evaluationIn[i % 3].vuv;
            EmitVertex();
        }
        EndPrimitive();
    }
]]

wireFragmentShader = [[
    #version 410 core

    uniform vec4 wireColor;

    out vec4 frag_colour;

    void main() {   
        frag_colour = wireColor;
    }
]]

shadowFragmentShader = [[
    #version 410

    out vec4 frag_colour;

    void main()
    {
        gl_FragDepth = gl_FragCoord.z;
    }  
]]

debugQuadFragShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform;

    out vec4 frag_colour;

    void main()
    {             
        float depthValue = texture(textureUniform, vuv).r;
        frag_colour = vec4(vec3(depthValue), 1.0);
        gl_FragDepth = gl_FragCoord.z;
    }
]]

combineQuadFragShader = [[
    #version 400

    in vec2 vuv;

    uniform sampler2D textureUniform0;
    uniform sampler2D textureUniform1;

    out vec4 frag_colour;

    void main()
    {             
        float depthValue1 = texture(textureUniform0, vuv).r;
        float depthValue2 = texture(textureUniform1, vuv).r;

        if(depthValue1 == 1.0 && depthValue2 == 1.0) {
            frag_colour = vec4(1.0);    
            gl_FragDepth = 1.0;
        } else {
            frag_colour = depthValue1 < depthValue2 ? vec4(vec3(depthValue1), 1.0) : vec4(vec3(0.0), 1.0);
            gl_FragDepth = depthValue1 < depthValue2 ? depthValue1 : 0.0;
        }
    }
]]


models = {}
models[1] = {file = "../3DModels/plane10x10b.obj", pos = {0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = {0.0,  0.0, 0.0}}

terrainTexture = "wildtextures-stone_pebbles_seamless_texture_512.jpg"

maxTessLevel = 6.0;
maxTessLevelShadow = 6.0

wireColor = {0.0, 0.0, 1.0, 1.0}

terrainSize = 5
elevationSize = 16
detailsSize = 32

nearPlane = 0.5
farPlane = 19.5
projectionDimension = 8.75

shadowWidth =  1024;
shadowHeight = 1024;

bwidth =  2000;
bheight = 2000;

adjustHeight = 1.75

seed = 6

ambient = {col = {255 / 255, 255 / 255, 255 / 255, 1.0}, intensity = 0.20}

light = {
    pos   = {0.000, 9.568, 9.530},
    dir   = {0.000, -0.815, -0.577},
    up    = {1.000, 0.000, 0.000},
    col = {237.0, 230.0, 90.0, 1.0}, 
    intensity = 10.0, 
    specularPower = 0.5, 
    directional = true 
}

cameraPosition = {
    pos   = {-18.501, 20.739, 2.725},
    dir   = {0.679, -0.733, -0.047},
    up    = {0.731, 0.680, -0.051},
    right = {0.070, -0.000, 0.998},
    angle = {-3.988, -47.139}
}

idir = light["dir"]
ipos = light["pos"]
mpos = models[1]["pos"]
mpos = models[1]["pos"]
accumulator = 0
function updateLight(deltaTime, posx, posy, posz, dirx, diry, dirz, upx, upy, upz, colr, colg, colb, cola, intensity)
    accumulator = accumulator + -50.0 * deltaTime
    if accumulator > 360 then
        accumulator = 0
    end
    angle = math.rad(accumulator)
    cos = math.cos(angle)
    sin = math.sin(angle)
    sposy = ipos[2] - mpos[2]
    sposz = ipos[3] - mpos[3]
    ssposy = cos * sposy + sin * sposz
    ssposz =-sin * sposy + cos * sposz
    posy = ssposy + mpos[2]
    posz = ssposz + mpos[3]
    diry =  cos * idir[2] + sin * idir[3]
    dirz = -sin * idir[2] + cos * idir[3]
    return intensity, cola, colb, colg, colr, upz, upy, upx, dirz, diry, dirx, posz, posy, posx
end