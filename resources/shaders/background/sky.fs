#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform float u_time;
uniform float u_height;
uniform float u_time_since_over;

vec3 bottom_low = vec3(1.0, 0.42, 0.592);
vec3 bottom_high = vec3(1.0, 0.569, 0.4);

vec3 top_low = vec3(0.302, 0.651, 1.0);
vec3 top_high = vec3(0.451, 0.153, 0.361);

vec3 top_over = vec3(0.047, 0.047, 0.078);
vec3 bottom_over = vec3(0.243, 0.137, 0.278);

float transition_to_over = 0.25;

void main() {
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.x *= u_resolution.x/u_resolution.y;

    vec3 bottom = mix(mix(bottom_low, bottom_high, u_height), bottom_over, min(transition_to_over * u_time_since_over, 1.0));
    vec3 top = mix(mix(top_low, top_high, u_height), top_over, min(transition_to_over * u_time_since_over, 1.0));

    vec3 color = mix(bottom, top, ((1.0 + 0.5 * sin(st.x + 0.3 * u_time)) + st.y) / 2.0);

    gl_FragColor = vec4(color, 1.0);
}
