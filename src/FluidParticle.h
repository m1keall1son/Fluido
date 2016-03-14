//
//  Particle.h
//  ParticlesTest
//
//  Created by Ryan Bartley on 2/3/15.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"
#include "cinder/Timeline.h"

namespace lonestar {
	
const uint32_t POSITION_OTHER_INDEX	= 0;
const uint32_t VELOCITY_INDEX		= 1;

class FluidParticle {
public:
	ci::vec4	position;
	ci::vec4	velocity;
	
	static void initData( ci::geom::BufferLayout &layout,
						  std::vector<FluidParticle> &data,
						  std::vector<uint32_t> &indices );
	
	static void initGlsl( ci::gl::GlslProg::Format &updateFormat,
						  ci::gl::GlslProg::Format &renderFormat );
	
	static ci::vec3 sBounding;
	static std::array<float, 6> sIdPercentages;
};

}