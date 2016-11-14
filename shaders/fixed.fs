uniform sampler2D s_texture0;
varying vec3 vTexCoord;

void main()
{
	gl_FragColor = texture(s_texture0, vTexCoord);
}