varying vec3 vTexCoord;

void main()
{
    vTexCoord = gl_MultiTexCoord0.xyz;
    gl_Position = gl_Vertex;
}