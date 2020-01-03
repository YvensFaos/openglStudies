normalVertexShader = [[
    #version 410 core

    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;

    out vec3 vposition;
    out vec3 vnormal;
    out vec2 vuv;

    uniform mat4 model;
    uniform mat4 viewProjection;

    void main() {
        vposition = vec3(model * vec4(vertex, 1.0));
        vnormal = mat3(transpose(inverse(model))) * normal;
        vuv = uv;

        gl_Position = viewProjection * vec4(vposition, 1.0);
    }
]]

normalGeometryShader = [[
    #version 410 core

    layout(triangles) in;
    layout(line_strip, max_vertices = 4) out;

    in vec3 vposition[];
    in vec3 vnormal[];
    in vec2 vuv[];

    out vec3 gposition;
    out vec3 gnormal;
    out vec2 guv;

    void main() {
        for(int i = 0; i < 4; i++) {
            gl_Position = gl_in[i % 3].gl_Position;
            gposition = vec3(gl_Position);
            gnormal = vnormal[i % 3];
            guv = vuv[i % 3];
            EmitVertex();
        }
        EndPrimitive();
    }
]]

normalFragmentShader = [[
    #version 410 core

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    in vec3 gposition;
    in vec3 gnormal;
    in vec2 guv;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    out vec4 frag_colour;

    void main()
    {          
        vec3 norm = normalize(gnormal);
        float distance = length(sceneLight.position - gposition);

        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;

        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;
        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

vertexShader = [[
    #version 410 core

    layout (location = 0) in vec3 vertex;
    layout (location = 1) in vec3 normal;
    layout (location = 2) in vec2 uv;

    out vec3 vposition;
    out vec3 vnormal;
    out vec2 vuv;

    uniform mat4 model;

    void main() {
        vposition = vec3(model * vec4(vertex, 1.0));
        vnormal = mat3(transpose(inverse(model))) * normal;
        vuv = uv;
    }
]]

controlTesselationShader = [[
    #version 410 core

    layout (vertices = 3) out;

    in vec3 vposition[];
    in vec3 vnormal[];
    in vec2 vuv[];

    out vec3 tsposition[];
    out vec3 tsnormal[];
    out vec2 tsuv[];
    out float ttessLevel[];

    uniform vec3 eyeWorldPos;

    float getTesselationLevel(float Distance0, float Distance1)
    {
        float minDistance = 1.0;
        float maxDistance = 10.0;
        float d = (Distance0 + Distance1) / 2.0;

        float level = (1.0 - abs(minDistance - max(minDistance, min(d, (minDistance + maxDistance)))) / maxDistance) * 10.0;
        return level;
    }
    
    void main() {
        tsposition[gl_InvocationID] = vposition[gl_InvocationID];
        tsnormal[gl_InvocationID] = vnormal[gl_InvocationID];
        tsuv[gl_InvocationID] = vuv[gl_InvocationID];

        float eyeToVertexDistance0 = distance(eyeWorldPos, tsposition[0]);
        float eyeToVertexDistance1 = distance(eyeWorldPos, tsposition[1]);
        float eyeToVertexDistance2 = distance(eyeWorldPos, tsposition[2]);

        gl_TessLevelOuter[0] = max(0.1, getTesselationLevel(eyeToVertexDistance1, eyeToVertexDistance2));
        gl_TessLevelOuter[1] = max(0.1, getTesselationLevel(eyeToVertexDistance2, eyeToVertexDistance0));
        gl_TessLevelOuter[2] = max(0.1, getTesselationLevel(eyeToVertexDistance0, eyeToVertexDistance1));
        gl_TessLevelInner[0] = max(0.1, gl_TessLevelOuter[2]);

        ttessLevel[gl_InvocationID] = gl_TessLevelOuter[0] / 10.0;
    }                                                                     
]]

evaluationTesselationShader = [[
    #version 410 core

    layout(triangles, equal_spacing, ccw) in; 

    in vec3 tsposition[];
    in vec3 tsnormal[];
    in vec2 tsuv[];
    in float ttessLevel[];

    out vec3 fsposition;
    out vec3 fsnormal;
    out vec2 fsuv;
    out float fstessLevel;
    
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
        fsposition = interpolate3D(tsposition[0], tsposition[1], tsposition[2]);
        fsuv = interpolate2D(tsuv[0], tsuv[1], tsuv[2]);
        fsnormal = interpolate3D(tsnormal[0], tsnormal[1], tsnormal[2]);
        fstessLevel = (ttessLevel[0] + ttessLevel[1] + ttessLevel[2]) / 3.0;

        gl_Position = viewProjection * vec4(fsposition, 1.0);
    }
]]

geometryShader = [[
    #version 410 core

    layout(triangles) in;
    layout(line_strip, max_vertices = 4) out;

    in vec3 fsposition[];
    in vec3 fsnormal[];
    in vec2 fsuv[];
    in float fstessLevel[];

    out vec3 gposition;
    out vec3 gnormal;
    out vec2 guv;
    out float gtessLevel;

    void main() {
        for(int i = 0; i < 4; i++) {
            gl_Position = gl_in[i % 3].gl_Position;
            gposition = vec3(gl_Position);
            gnormal = fsnormal[i % 3];
            guv = fsuv[i % 3];
            gtessLevel = fstessLevel[i % 3];
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

    in vec3 gposition;
    in vec3 gnormal;
    in vec2 guv;
    in float gtessLevel;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    out vec4 frag_colour;

    void main()
    {          
        vec3 norm = normalize(gnormal);
        float distance = length(sceneLight.position - gposition);

        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;

        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;

        frag_colour = vec4(diffuse, sceneLight.color.w) + vec4(0.0, gtessLevel, 0.0, 0.0);
    }
]]

fragmentShader_withoutGeometryShader = [[
    #version 410 core

    struct Light {
        vec3 position;
        vec3 direction;
        vec4 color;
        float intensity;
        bool directional;
    };

    in vec3 fsposition;
    in vec3 fsnormal;
    in vec2 fsuv;

    float lightConstant = 1.0f;
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;
    float maximumIntensity = 100.0f;

    uniform Light sceneLight;
    out vec4 frag_colour;

    void main()
    {          
        vec3 norm = normalize(fsnormal);
        float distance = length(sceneLight.position - fsposition);

        float attenuationIntensity = sceneLight.intensity / maximumIntensity;
        lightConstant = 1.0f    / attenuationIntensity;
        lightLinear = 0.09f     / attenuationIntensity;
        lightQuadratic = 0.032f / attenuationIntensity;

        float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));
        float diff = max(dot(norm, sceneLight.direction), 0.0);
        vec3 diffuse =  attenuation * diff * sceneLight.color.xyz;

        frag_colour = vec4(diffuse, sceneLight.color.w);
    }
]]

models = {}
models[1] = {file = "../3DModels/nonormalmonkey.obj", pos = { 0.0, 0.0, 3.0}, sca = {1.0, 1.0, 1.0}, rot = {0.0, 0.0, 0.0}}

lightIntensity = 100
light = {
    pos = { 0.0, 0.0, 5.0}, 
    dir = {0.0, 0.0, 1.0}, 
    up = {0.0, 1.0, 0.0}, 
    col = {227 / 255, 118 / 255, 252 / 255, 1.0}, 
    intensity = lightIntensity, 
    specularPower = 256.0, 
    directional = true
}

cameraPosition = {
    pos   = {0.840, 0.484, 6.248},
    dir   = {-0.286, -0.123, -0.950},
    up    = {-0.035, 0.992, -0.118},
    right = {0.957, 0.000, -0.289},
    angle = {-106.773, -7.053}
}