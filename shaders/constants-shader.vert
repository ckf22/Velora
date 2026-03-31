#version 450

float x[3] = float[](
  0.0,
  -0.5,
  0.5
);

float y[3] = float[](
  -0.5,
  0.5,
  0.5
);


void main(){
  gl_Position = vec4(x[gl_VertexIndex], y[gl_VertexIndex], 0.0, 1 );
}