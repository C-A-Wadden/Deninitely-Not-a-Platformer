uniform sampler2D texture;

void main(){
    vec4 color = texture2D( texture, gl_TexCoord[0].st );
    color.x *= 2;
    gl_FragColor = color;
}
