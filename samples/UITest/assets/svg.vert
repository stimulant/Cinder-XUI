varying vec4 vColor;

void main()
{
	vColor = gl_Color;
	gl_Position = ftransform();
}