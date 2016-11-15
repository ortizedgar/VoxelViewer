uniform sampler3D s_texture0;
uniform vec3 pos;
uniform vec3 iViewDir;
varying vec3 vTexCoord;

// Transfer function
vec4 transfer(float I)
{
    vec4 clr;
    float s = 0.75;
    if(I<0.3)
        clr = vec4(s , s , 1.0 , 1.0);
    else
    if(I<0.7)
        clr = vec4(s , 1.0 , s,1.0);
    else
        clr = vec4(1.0 , s,s,1.0) ;

    return clr * I * 1.5;
}

void main()
{

    vec3 q = (vTexCoord-0.5)* 256;
    float s = dot(q-pos,iViewDir);
    float dist = length(pos + iViewDir*s - q);
    if( s>=0 && s<75 && dist <5)
    {
        gl_FragColor = vec4(0.5,0.5,1,1);
    }
    else
    {
        dist = length(pos-q);
        if( dist<10)
            gl_FragColor = vec4(0.5,0.5,1,1-dist/10);
        else
        {
            vec3 clr  = texture3D(s_texture0, vTexCoord).rgb;
            float k = clr.g;		// intensidad
			if(k<0.1)
				discard;

			if(clr.r>0.7 && clr.g+clr.b<0.5)
			{
				// anomalia
				gl_FragColor.rgb = clr;
				gl_FragColor.a = 0.35;
				
			}
			else
			{	
				// tejido normal
				if(k>0.9)
					gl_FragColor.a = 0.07;
				else
					gl_FragColor.a = k*k* 0.2;
					
				gl_FragColor.r = 45.0/255.0 * k * 1.5;
				gl_FragColor.g = 229.0/255.0 * k* 1.5;
				gl_FragColor.b = 237.0/255.0 * k* 1.5;
				
			}
				
				
        }
    }
}