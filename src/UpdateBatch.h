//
//  UpdateBatch.h
//  ParticleImplementation
//
//  Created by Ryan Bartley on 2/2/15.
//
//

#pragma once

#include <array>
#include <map>
#include <string>

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/TransformFeedbackObj.h"
#include "cinder/gl/BufferTexture.h"
#include "cinder/Log.h"

#include "FluidParticle.h"

namespace lonestar {
	
using VaoPingPong =	std::array<ci::gl::VaoRef, 2>;
using VboPingPong = std::array<ci::gl::VboRef, 2>;
using XfoPingPong = std::array<ci::gl::TransformFeedbackObjRef, 2>;
using BtoPingPong = std::array<ci::gl::BufferTextureRef, 2>;
using ElementVbo  = ci::gl::VboRef;
using SubroutineMap = std::map<std::string, GLuint>;
	
template<typename T>
class UpdateBatch {
public:
	using UpdateBatchRef = std::shared_ptr<UpdateBatch<T>>;
	using SetupFunc = std::function<void( ci::geom::BufferLayout &,
										 std::vector<T> &,
										 std::vector<uint32_t> &,
										 ci::gl::GlslProg::Format &,
										 ci::gl::GlslProg::Format & )>;
	using GlslPrepFunc = std::function<void( ci::gl::GlslProgRef & )>;
	
	static UpdateBatchRef create( uint32_t numUpdateBatch,
								 GlslPrepFunc preUpdate,
								 GlslPrepFunc postUpdate,
								 GlslPrepFunc preDraw,
								 GlslPrepFunc postDraw );
	
	~UpdateBatch() = default;
	
	void update();
	void draw();
	
	void populateBuffers();
	void populateGlsl();
	
	ci::gl::BufferTextureRef& getBufferTexture( bool current = true )
		{ return current ? mBtos[mCurrentBuffer] : mBtos[1-mCurrentBuffer]; }
	const ci::gl::BufferTextureRef& getBufferTexture( bool current = true ) const
		{ return current ? mBtos[mCurrentBuffer] : mBtos[1-mCurrentBuffer]; }
	
	void replaceUpdateGlslProg( const ci::gl::GlslProgRef &glsl ){ mUpdateGlsl = glsl; }
	void replaceRenderGlslProg( const ci::gl::GlslProgRef &glsl ){ mRenderGlsl = glsl; }

	ci::gl::GlslProgRef& getUpdateGlslProg() { return mUpdateGlsl; }
	const ci::gl::GlslProgRef& getUpdateGlslProg() const { return mUpdateGlsl; }
	ci::gl::GlslProgRef& getRenderGlslProg() { return mRenderGlsl; }
	const ci::gl::GlslProgRef& getRenderGlslProgRef() const { return mRenderGlsl; }
	
private:
	UpdateBatch( uint32_t numUpdateBatch,
				GlslPrepFunc preUpdate,
				GlslPrepFunc postUpdate,
				GlslPrepFunc preDraw,
				GlslPrepFunc postDraw );
	
	void initBuffers( const ci::geom::BufferLayout &layout, const std::vector<T> &data, const std::vector<uint32_t> &indices );
	
	VaoPingPong mVaos;
	VboPingPong mVbos;
	XfoPingPong	mXfos;
	BtoPingPong	mBtos;
	ElementVbo		mElements;
	GlslPrepFunc	mInitUpdate, mInitRender,
					mPreUpdateFunc, mPostUpdateFunc,
					mPreDrawFunc, mPostDrawFunc;
	ci::gl::GlslProgRef	mUpdateGlsl, mRenderGlsl;
	uint32_t			mNumUpdateBatch;
	uint32_t			mNumIndices		= 0;
	uint32_t			mCurrentBuffer	= 1;
	GLenum				mPrimitiveType	= GL_POINTS;
};

template<typename T> using UpdateBatchRef = std::shared_ptr<UpdateBatch<T>>;

template<typename T>
UpdateBatchRef<T> UpdateBatch<T>::create( uint32_t numUpdateBatch,
										 GlslPrepFunc preUpdate,
										 GlslPrepFunc postUpdate,
										 GlslPrepFunc preDraw,
										 GlslPrepFunc postDraw )
{
	return std::shared_ptr<UpdateBatch<T>>( new UpdateBatch<T>( numUpdateBatch, preUpdate, postUpdate, preDraw, postDraw ) );
}
	
template<typename T>
UpdateBatch<T>::UpdateBatch( uint32_t numUpdateBatch,
							GlslPrepFunc preUpdate,
							GlslPrepFunc postUpdate,
							GlslPrepFunc preDraw,
							GlslPrepFunc postDraw )
: mNumUpdateBatch( numUpdateBatch ), mPreUpdateFunc( preUpdate ),
	mPostUpdateFunc( postUpdate ), mPreDrawFunc( preDraw ),
	mPostDrawFunc( postDraw )
{
	CI_ASSERT_MSG( sizeof(T) % 16 == 0, "UpdateBatch expects a 16 byte aligned object" );
	
	populateBuffers();
	populateGlsl();
}
	
template<typename T>
void UpdateBatch<T>::populateBuffers()
{
	ci::geom::BufferLayout layout;
	std::vector<T> bufferData( mNumUpdateBatch );
	std::vector<uint32_t> indices;
	T::initData( layout, bufferData, indices );
	
	initBuffers( layout, bufferData, indices );
}
	
template<typename T>
void UpdateBatch<T>::populateGlsl()
{
	ci::gl::GlslProg::Format updateFormat, renderFormat;
	
	T::initGlsl( updateFormat, renderFormat );
	
	try {
		mUpdateGlsl = ci::gl::GlslProg::create( updateFormat );
		mRenderGlsl = ci::gl::GlslProg::create( renderFormat );
	}
	catch( const ci::gl::GlslProgCompileExc &ex ) {
		CI_LOG_E(ex.what());
		throw ex;
	}
	catch( const ci::gl::GlslProgLinkExc &ex ) {
		CI_LOG_E(ex.what());
		throw ex;
	}
	catch( const ci::gl::GlslProgExc &ex ) {
		CI_LOG_E(ex.what());
		throw ex;
	}
	catch( const ci::app::AssetLoadExc &ex ) {
		CI_LOG_E(ex.what());
		throw ex;
	}
}

template<typename T>
void UpdateBatch<T>::initBuffers( const ci::geom::BufferLayout &layout, const std::vector<T> &data, const std::vector<uint32_t> &indices )
{
	
	mVbos[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), GL_STATIC_DRAW );
	mVbos[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), GL_STATIC_DRAW );
	
	if( ! indices.empty() ) {
		mElements = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW );
	}
	
	for( int i = 0; i < 2; i++ ) {
		mVaos[i] = ci::gl::Vao::create();
		ci::gl::ScopedVao scopeVao( mVaos[i] );
		ci::gl::ScopedBuffer scopeVbo( mVbos[i] );
		
		auto infos = layout.getAttribs();
		
		for( int j = 0; j < infos.size(); j++ ) {
			ci::gl::enableVertexAttribArray( j );
			auto & info = infos[j];
			if( info.getDataType() == ci::geom::DataType::FLOAT ) {
				ci::gl::vertexAttribPointer( j,
											info.getDims(),
											GL_FLOAT,
											GL_FALSE,
											info.getStride(),
											(GLvoid*) info.getOffset() );
			}
			else {
				ci::gl::vertexAttribIPointer( j,
											 info.getDims(),
											 GL_INT,
											 info.getStride(),
											 (GLvoid*) info.getOffset() );
			}
		}
		mXfos[i] = ci::gl::TransformFeedbackObj::create();
		mXfos[i]->bind();
		ci::gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[i] );
		mXfos[i]->unbind();
		
		mBtos[i] = ci::gl::BufferTexture::create( mVbos[i], GL_RGBA32F );
	}
}
	
template<typename T>
void UpdateBatch<T>::update()
{
	ci::gl::ScopedGlslProg scopeGlsl( mUpdateGlsl );

	if( mPreUpdateFunc )
		mPreUpdateFunc( mUpdateGlsl );
	
	// This equation just reliably swaps all concerned buffers
	mCurrentBuffer = 1 - mCurrentBuffer;
	
	// We use this vao for input to the Glsl, while using the opposite
	// for the TransformFeedbackObj.
	ci::gl::ScopedVao		vaoScope( mVaos[mCurrentBuffer] );
	// Because we're not using a fragment shader, we need to
	// stop the rasterizer. This will make sure that OpenGL won't
	// move to the rasterization stage.
	ci::gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
	
	// Opposite TransformFeedbackObj to catch the calculated values
	// In the opposite buffer
	mXfos[1-mCurrentBuffer]->bind();
	
	ci::gl::setDefaultShaderVars();
	
	// We begin Transform Feedback, using the same primitive that
	// we're "drawing". Using points for the particle system.
	ci::gl::beginTransformFeedback( mPrimitiveType );
	
	if( mElements ) {
		ci::gl::ScopedBuffer scopeBuffer( mElements );
		ci::gl::drawElements( mPrimitiveType, mNumIndices, GL_UNSIGNED_INT, 0 );
	}
	else
		ci::gl::drawArrays( mPrimitiveType, 0, mNumUpdateBatch );
	
	ci::gl::endTransformFeedback();
	
	if( mPostUpdateFunc )
		mPostUpdateFunc( mUpdateGlsl );
}
	
template<typename T>
void UpdateBatch<T>::draw()
{
	ci::gl::ScopedGlslProg scopeGlsl( mRenderGlsl );
	
	if( mPreDrawFunc )
		mPreDrawFunc( mRenderGlsl );
	
	ci::gl::ScopedVao			vaoScope( mVaos[1-mCurrentBuffer] );
	
	// handles ciModelViewProjection and ciElapsedTime
	ci::gl::setDefaultShaderVars();
	
	if( mElements ) {
		ci::gl::ScopedBuffer scopeBuffer( mElements );
		ci::gl::drawElements( mPrimitiveType, mNumIndices, GL_UNSIGNED_INT, 0 );
	}
	else
		ci::gl::drawArrays( mPrimitiveType, 0, mNumUpdateBatch );
	
	
	if( mPostDrawFunc )
		mPostDrawFunc( mRenderGlsl );
}
	
template<typename T>
using UpdateBatchRef = std::shared_ptr<UpdateBatch<T>>;

	
}