#version 330 core

in vec3 color_vsout;

////////////////////////////////////////////////////////////////////////////////
void main() {

    gl_FragColor = vec4(color_vsout, 1.0);
}

/// @file
