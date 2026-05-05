<xml>
    <vertex><![CDATA[
        #version 330 core

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec2 aUV;

        out vec2 vUV;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        void main() {
            vUV = aUV;
            gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
        }
    ]]></vertex>

    <fragment><![CDATA[
        #version 330 core

        // The quad spans kQuadRadii core radii from centre to edge (must match Game's kChargeQuadRadii).
        const float kQuadRadii = 3.0;

        out vec4 FragColor;

        in vec2 vUV;

        uniform vec4 uColor;
        // x = sign of q (+1 / -1), y = fixed, z = hovered
        uniform vec4 uStyle;

        float band(float d, float half_width) {
            float aa = fwidth(d);
            return 1.0 - smoothstep(half_width - aa, half_width + aa, d);
        }

        void main() {
            // p is measured in core radii: the core disc is |p| < 1.
            vec2 p = (vUV - 0.5) * 2.0 * kQuadRadii;
            float r = length(p);
            bool positive = uStyle.x > 0.0;
            bool fixed_charge = uStyle.y > 0.5;
            bool hovered = uStyle.z > 0.5;

            float core = band(r, 1.0);
            float glow = exp(-1.8 * max(r - 1.0, 0.0)) * (1.0 - smoothstep(2.2, kQuadRadii, r));
            glow *= hovered ? 0.85 : 0.5;

            vec3 color = mix(uColor.rgb, vec3(1.0), 0.3 * core);
            float alpha = max(core, glow);

            // Sign glyph inside the core: a bar, plus a vertical bar for positive charges.
            vec2 a = abs(p);
            float horizontal = band(max(a.x - 0.5, 0.0) + max(a.y - 0.12, 0.0), 0.0001);
            float vertical = positive ? band(max(a.y - 0.5, 0.0) + max(a.x - 0.12, 0.0), 0.0001) : 0.0;
            float glyph = max(horizontal, vertical) * core;
            color = mix(color, vec3(0.04, 0.04, 0.08), glyph);

            // Fixed charges get a pin ring just outside the core.
            if (fixed_charge) {
                float ring = band(abs(r - 1.3), 0.06);
                color = mix(color, vec3(0.95), ring);
                alpha = max(alpha, ring);
            }

            FragColor = vec4(color, alpha * uColor.a);
        }
    ]]></fragment>
</xml>
