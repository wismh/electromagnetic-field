<xml>
    <vertex><![CDATA[
        #version 330 core

        // No uModel: the engine uses this shader for instanced particles.
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec2 aUV;
        layout(location = 2) in vec3 aInstancePos;
        layout(location = 3) in float aInstanceRot;
        layout(location = 4) in vec2 aInstanceSize;
        layout(location = 5) in vec4 aInstanceColor;

        out vec2 vUV;
        out vec4 vColor;

        uniform mat4 uView;
        uniform mat4 uProjection;
        uniform vec4 uColor;

        void main() {
            vUV = aUV;
            vColor = aInstanceColor * uColor;
            float c = cos(aInstanceRot);
            float s = sin(aInstanceRot);
            vec2 scaled = aPosition.xy * aInstanceSize;
            vec2 rotated = vec2(scaled.x * c - scaled.y * s, scaled.x * s + scaled.y * c);
            gl_Position = uProjection * uView * vec4(aInstancePos + vec3(rotated, 0.0), 1.0);
        }
    ]]></vertex>

    <fragment><![CDATA[
        #version 330 core

        in vec2 vUV;
        in vec4 vColor;
        out vec4 FragColor;

        // outward > 0.5: dot (out of the page). Otherwise a cross. Tints are chosen so r > b means out.
        float coverage(vec2 uv, float outward) {
            vec2 p = (uv - 0.5) * 2.0;
            float r = length(p);
            float ring = step(0.68, r) * step(r, 0.94);
            float dot_mark = step(r, 0.24);
            float d1 = abs(p.x - p.y) * 0.70710678;
            float d2 = abs(p.x + p.y) * 0.70710678;
            float cross_mark = step(min(d1, d2), 0.11) * step(r, 0.78);
            float glyph = outward > 0.5 ? dot_mark : cross_mark;
            return max(ring, glyph);
        }

        void main() {
            float outward = vColor.r > vColor.b ? 1.0 : 0.0;
            vec2 dx = dFdx(vUV) * 0.25;
            vec2 dy = dFdy(vUV) * 0.25;
            float a = coverage(vUV + dx + dy, outward) + coverage(vUV + dx - dy, outward) +
                      coverage(vUV - dx + dy, outward) + coverage(vUV - dx - dy, outward);
            FragColor = vec4(vColor.rgb, vColor.a * a * 0.25);
        }
    ]]></fragment>
</xml>
