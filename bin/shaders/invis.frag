// https://en.sfml-dev.org/forums/index.php?topic=24250.0

uniform sampler2D texture;

void main(){
    vec4 color = texture2D( texture, gl_TexCoord[0].st );
    color.w *= 0.25;
    gl_FragColor = color;
}
