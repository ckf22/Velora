#version 450

float x[6] = float[](
  0.0,
  -0.5,
  0.5,
  1,
  0.8,
  1
);

float y[6] = float[](
  -0.5,
  0.5,
  0.5,
  1,
  1,
  0.8
);


void main(){
  gl_Position = vec4(x[gl_VertexIndex], y[gl_VertexIndex], 0.0, 1 );
}