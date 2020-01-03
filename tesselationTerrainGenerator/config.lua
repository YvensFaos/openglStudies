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
    } evaluationOut;
    
    uniform mat4 viewProjection;

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

        gl_Position = viewProjection * vec4(evaluationOut.vposition, 1.0);
    }
]]

DETAILS = 0.10
ELEVATN = 0.13
TERRAIN = 0.65

geometryShader = [[
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
            vec4 finalPosition = gl_in[i].gl_Position + vec4(0.0, calculatedHeight.y, 0.0, 0.0);
            
            gl_Position = finalPosition;
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

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    in geometryOut {
        vec3 vposition;
        vec3 vnormal;
        vec2 vuv;
    } geometryIn;

    uniform sampler2D textureUniform0;
    uniform sampler2D textureUniform1;
    uniform sampler2D textureUniform2;
    uniform sampler2D textureUniform3;

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

    uniform Light pointLights[10];
    uniform Light directionalLights[10];

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
        return vec4(diffuse, 1.0);
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
        return vec4(diffuse, 1.0);
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
        for(int i = 0; i < clampedNumberOfDirectional; i++) {
            directionalLightResultant += calculateDirectionalLight(i, cnormal);
        }
        vec4 pointLightResultant = vec4(0.0);
        int clampedNumberOfPoint = min(numberPointLights, 10);
        for(int i = 0; i < clampedNumberOfPoint; i++) {
            pointLightResultant += calculatePointLight(i, cnormal);
        }
        vec4 resultantLight = directionalLightResultant + pointLightResultant + texture(textureUniform0, geometryIn.vuv);
        resultantLight.a = 1.0;

        if(gl_FrontFacing) {
            frag_colour = resultantLight;
        } else {
            frag_colour = vec4(0.05, 0.05, 0.05, 1.0);
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
            vec4 finalPosition = gl_in[i % 3].gl_Position + vec4(0.0, calculatedHeight.y, 0.0, 0.0);
            
            gl_Position = finalPosition;

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

models = {}
models[1] = {file = "../3DModels/plane10x10.obj", pos = { 0.0,  0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = { 0.0,  0.0, 0.0}}

maxTessLevel = 1.0;

wireColor = {0.0, 0.0, 1.0, 1.0}

terrainSize = 4
elevationSize = 16
detailsSize = 32

light = {pos = {0.0, 1.5, 3.0}, 
         dir = {0.0, -1.0, 0.0}, 
         up = {0.0, 1.0, 0.0}, 
         col = {127.0, 195.0, 235.0, 1.0}, 
         intensity = 0.25, 
         specularPower = 128.0, 
         directional = true }

cameraPosition = {
    pos   = {1.925, 1.126, 4.845},
    dir   = {-0.691, -0.293, -0.660},
    up    = {-0.212, 0.956, -0.202},
    right = {0.691, 0.000, -0.723},
    angle = {-136.312, -17.038}
}