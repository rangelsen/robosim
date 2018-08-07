#version 130

out vec4 FragColor;

in vec2 tex_coord_vsout;
flat in vec4 material_color_vsout;
flat in vec4 color_ambient_vsout;
flat in vec4 color_diffuse_vsout;
flat in vec4 color_specular_vsout;

void main() {    

    gl_FragColor = material_color_vsout * (color_ambient_vsout +
        color_diffuse_vsout + color_specular_vsout);
}

/*
#version 130

in vec2 texcoords_vsout;

/*
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
*/

/*
void main() {

	// gl_FragColor = vec4(0.8, 0.8, 0.8, 1.0);

    // gl_FragColor = texture2D(texture_diffuse1, texcoords_vsout);

	// gl_FragColor = mix(texture2D(texture_specular1, texcoords_vsout),
									// texture2D(texture_diffuse1, texcoords_vsout), 0.8);
	
	// gl_FragColor = mix(diffuse_specular_mix,
					   // texture2D(texture_normal1, texcoords_vsout), 0.1);
}
*/

/// @file

