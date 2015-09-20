
uniform vec2 resolution;
uniform float time;
varying vec2 vUv;
vec3 skyColor(in vec3 rd) 
{
    vec3 sundir = normalize(vec3(.0, .1, 1.));
    float yd = min(rd.y, 0.);
    rd.y = max(rd.y, 0.);
    vec3 col = vec3(0.);
    col += vec3(.4, .4 - exp(-rd.y * 20.) * .3, .0) * exp(-rd.y * 9.);
    col += vec3(.3, .5, .6) * (1. - exp(-rd.y * 8.)) * exp(-rd.y * .9);
    col = mix(col * 1.2, vec3(.3), 1. - exp(yd * 100.));
    col += vec3(1.0, .8, .55) * pow(dot(rd, sundir), 15.) * .6;
    col += pow(dot(rd, sundir), 150.0) * .15;
    return col;
}
float checker(vec2 p) 
{
    p = mod(floor(p), 2.0);
    return mod(p.x + p.y, 2.0) < 1.0 ? .25 : 0.1;
}
void main() 
{
    vec2 q = vUv.xy / resolution.xy;
    vec2 v = -1.0 + 2.0 * q;
    v.x *= resolution.x / resolution.y;
    vec3 dir = normalize(vec3(v.x, v.y + .5, 1.5));
    vec3 col = vec3(checker(dir.xz / dir.y * .5 + vec2(0., -time * 2.))) + skyColor(reflect(dir, vec3(0., 1., 0.))) * .3;
    col = mix(col, skyColor(dir), exp(-max(-v.y * 9. - 4.8, 0.)));
    col *= .7 + .3 * pow(q.x * q.y * (1. - q.x) * (1. - q.y) * 16., .1);
    gl_FragColor = vec4(col, 1.);
}
