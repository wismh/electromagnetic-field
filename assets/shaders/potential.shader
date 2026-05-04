<xml>
    <vertex>
        #version 330 core

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec2 aUV;

        out vec2 vWorld;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        void main() {
            vec4 world = uModel * vec4(aPosition, 1.0);
            vWorld = world.xy;
            gl_Position = uProjection * uView * world;
        }
    </vertex>

    <fragment><![CDATA[
        #version 330 core

        const int kMaxCharges = 32;

        out vec4 FragColor;

        in vec2 vWorld;

        // xy = position, z = q. Only the first uParams.x entries are read.
        uniform vec4 uCharges[kMaxCharges];
        // x = charge count, y = k, z = softening
        uniform vec4 uParams;

        void main() {
            int count = int(uParams.x);
            float eps2 = uParams.z * uParams.z;
            float phi = 0.0;
            for (int i = 0; i < kMaxCharges; ++i) {
                if (i >= count) {
                    break;
                }
                vec2 d = vWorld - uCharges[i].xy;
                phi += uParams.y * uCharges[i].z / sqrt(dot(d, d) + eps2);
            }

            float t = tanh(0.6 * phi);
            vec3 background = vec3(0.02, 0.02, 0.04);
            vec3 positive = vec3(1.0, 0.35, 0.15);
            vec3 negative = vec3(0.15, 0.45, 1.0);
            vec3 color = mix(background, t > 0.0 ? positive : negative, abs(t));

            // Equipotential lines every 0.25 of phi, anti-aliased by the screen-space derivative.
            float level = phi * 4.0;
            float dist = abs(fract(level + 0.5) - 0.5) / max(fwidth(level), 1e-4);
            float line = 1.0 - clamp(dist, 0.0, 1.0);
            color += 0.25 * line * vec3(1.0);

            FragColor = vec4(color, 1.0);
        }
    ]]></fragment>
</xml>
