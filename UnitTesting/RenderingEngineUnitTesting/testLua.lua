light = {
    pos = { 0.0, 2.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, 
    col = {0.9686, 0.8156, 0.2117, 1.0},
    intensity = 80, 
    specularPower = 0.0, 
    directional = true
}

ambient = 
    {col = { 1.0, 1.0, 1.0, 1.0}, 
    intensity = 0.05
}

cameraPosition = {
    pos =   { 1.0, 8.5, 17.0},
    dir =   { 0.0, 0.0, -1.0},
    up =    { 0.0, 0.9,  0.1},
    right = { 0.8, 0.2,  0.0}
}

lights = {
    {
        pos = { 0.0, 2.0, 0.0}, dir = {0.0, 0.0, -1.0}, up = {0.0, 1.0, 0.0}, 
        col = {0.9686, 0.8156, 0.2117, 1.0},
        intensity = 80, 
        specularPower = 0.0, 
        directional = true
    },
    {
        pos = { 0.0, -2.0, 0.0}, dir = {0.0, 0.0, 1.0}, up = {0.0,-1.0, 0.0}, 
        col = {0.8156, 0.2117, 0.9686, 0.7},
        intensity = 100, 
        specularPower = 32.0, 
        directional = false
    }
}