<xml>
    <vertex><![CDATA[
        #version 330 core

        // Instanced: the engine feeds per-instance attributes and, because there is no uModel
        // uniform, picks this shader instead of its built-in particle shader.
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

        // Triangular arrow head pointing along local +x (marks direction on field lines).
        float coverage(vec2 uv) {
            float y = abs(uv.y - 0.5) * 2.0;
            return step(y, 1.0 - uv.x);
        }

        void main() {
            vec2 dx = dFdx(vUV) * 0.25;
            vec2 dy = dFdy(vUV) * 0.25;
            float a = coverage(vUV + dx + dy) + coverage(vUV + dx - dy) +
                      coverage(vUV - dx + dy) + coverage(vUV - dx - dy);
            FragColor = vec4(vColor.rgb, vColor.a * a * 0.25);
        }
    ]]></fragment>
</xml>
