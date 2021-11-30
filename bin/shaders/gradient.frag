//https://github.com/SFML/SFML/wiki/Source:-Radial-Gradient-Shader

uniform vec4 color;
uniform float expand;
uniform vec2 center;
uniform float radius;
uniform float windowHeight;
void main(void)
{
    vec4 oColor = vec4(255, 255, 255, 0);
    vec2 centerFromSfml = vec2(center.x, windowHeight - center.y);
    vec2 p = (gl_FragCoord.xy - centerFromSfml) / radius;
	float r = sqrt(dot(p, p));
	if (r < 1.0)
	{
		gl_FragColor = mix(color, oColor, (r - expand) / (1 - expand));
	}
	else
	{
		gl_FragColor = oColor;
	}
};