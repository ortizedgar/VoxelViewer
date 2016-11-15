uniform sampler3D s_texture0;
uniform vec3 iLookFrom;
uniform vec3 iViewDir;
uniform vec3 iDx;
uniform vec3 iDy;
uniform float voxel_step0;
uniform float voxel_step;
uniform int game_status;
uniform int filter;
uniform float time;
varying vec3 vTexCoord;
const float cant_total = 15.0;

// Transfer function
vec3 transfer(float I)
{
    vec3 clr;
    float t0 = 0.3;
    float t1 = 0.7;
    if(I<t0)
    {
        clr = vec3(0.0 , 0.0 , I/t0);
    }
    else
    {
        if(I<t1)
        {
            clr = vec3(0.0 , (I-t0)/(t1-t0) , 0.0);
        }
        else
        {
            clr = vec3((I-t1)/(1-t1),0.0 , 0.0);
        }
    }

    return mix(clr , vec3(I,I,I) , 0.5);
}


vec3 tex3d(vec3 pos)
{
    vec3 S = vec3(0,0,0);
    pos += vec3(128.0,128.0,128.0);
    float k = 1.0/256.0;
    vec4 tx = texture3D(s_texture0,pos.xzy*k);
    return filter!=0 ? transfer(tx.r) : tx.rgb + S;
}

void main()
{
    vec2 uv = vTexCoord.xy*0.75;

    // computo la direccion del rayo
    // D = N + Dy*y + Dx*x;

    vec3 rd = normalize(iViewDir + iDy*uv.y + iDx*uv.x);
    vec3 ro = iLookFrom + rd*voxel_step0;
    vec3 S = vec3(0.0,0.0,0.0);
    float k = 1.0;

    // Ray marching
    for (int i = 0; i < cant_total; i++)
    {
        S += tex3d(ro)*k;
        ro += rd*voxel_step;
    }
    S /= cant_total;
	if(S.r<0.20)
		discard;
	float K = 2.;
    gl_FragColor.r = 50./255.*S.x*K;
    gl_FragColor.g = 160./255.*S.y*K;
    gl_FragColor.b = 210./255.*S.z*K;
	gl_FragColor.rgb *= gl_FragColor.rgb;
	gl_FragColor.a = 1.0;
	
}