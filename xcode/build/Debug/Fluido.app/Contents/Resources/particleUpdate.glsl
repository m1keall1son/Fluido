#version 330

in vec4 position;
in vec4 velocity;

out vec4 oPosition;
out vec4 oVelocity;

uniform float uDissapation;
uniform float uDeltaTime;

uniform sampler2D uVelocities;

uniform ivec2 ciWindowSize;

void main(){
    
    oVelocity = velocity;
    oPosition = position;
    
    vec2 size = ciWindowSize;
    
    vec2 tc = position.xy / size;
    
    vec3 fluidVel = texture( uVelocities, tc ).xyz;

    oVelocity.xyz = (fluidVel / uDeltaTime);
    oVelocity.xyz *= uDissapation;// * density;
    oPosition.xyz = position.xyz + oVelocity.xyz;
    
    if( oPosition.y > size.y )oPosition.y = 0.;
    if( oPosition.y < 0. )oPosition.y = size.y;
    if( oPosition.x > size.x ) oPosition.x = 0.;
    if( oPosition.x < 0. ) oPosition.x = size.x;
    
}