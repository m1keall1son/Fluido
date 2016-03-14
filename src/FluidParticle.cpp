//
//  NoiseParticle.cpp
//  Cube
//
//  Created by Ryan Bartley on 2/19/15.
//
//

#include "cinder/app/Platform.h"
#include "cinder/Log.h"

#include "FluidParticle.h"

using namespace ci;
using namespace app;
using namespace std;

namespace lonestar {
	
ci::vec3 FluidParticle::sBounding = vec3( 0 );
	
void FluidParticle::initData( geom::BufferLayout &layout,
							  vector<FluidParticle> &data,
							  vector<uint32_t> &indices )
{
	for( auto & particle : data ) {
        particle.position = vec4( randFloat()*getWindowWidth(), randFloat()*getWindowHeight(), 0, 1. );
        particle.velocity = vec4(0);
	}
	layout.append( (geom::Attrib) POSITION_OTHER_INDEX, geom::DataType::FLOAT, 4, sizeof(FluidParticle), 0 );
	layout.append( (geom::Attrib) VELOCITY_INDEX, geom::DataType::FLOAT, 4, sizeof(FluidParticle), (sizeof(vec4) * 1) );
}

void FluidParticle::initGlsl( gl::GlslProg::Format &updateFormat, gl::GlslProg::Format &renderFormat )
{
    updateFormat.feedbackFormat( GL_INTERLEAVED_ATTRIBS );
    updateFormat.feedbackVaryings({
        "oPosition",
        "oVelocity",
    });
    updateFormat.vertex( loadAsset("particleUpdate.glsl") );
    updateFormat.attribLocation( "position",            POSITION_OTHER_INDEX );
    updateFormat.attribLocation( "velocity",			VELOCITY_INDEX );
    updateFormat.preprocess(true);
    
    renderFormat.vertex( loadAsset("particleRender.vert") );
    renderFormat.fragment( loadAsset("particleRender.frag") );
    renderFormat.attribLocation( "position",		POSITION_OTHER_INDEX );
    renderFormat.attribLocation( "velocity",		VELOCITY_INDEX );
    renderFormat.preprocess(true);
	
}

}