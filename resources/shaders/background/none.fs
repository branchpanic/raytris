#ifdef GL_ES
precision mediump float;
#endif

uniform float u_time;
uniform vec2 u_resolution;

uniform float u_height;
uniform float u_game_over_time;
uniform int u_block_size;

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
