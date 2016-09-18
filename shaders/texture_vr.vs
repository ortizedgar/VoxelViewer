varying vec3 vTexCoord;

void main()
{    
    vTexCoord = (gl_TextureMatrix[0]*gl_MultiTexCoord0).xyz;
    gl_Position = gl_Vertex;
}

