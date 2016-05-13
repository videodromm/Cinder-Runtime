/*
 Cinder-Cereal
 Copyright (c) 2016, Simon Geilfus, All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"
#include "cinder/Vector.h"

#include "cinder/Area.h"
#include "cinder/Camera.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/Filesystem.h"

namespace cereal {

// glm vector serialization
template<class Archive> void serialize( Archive & archive, ci::vec2 &v ) { archive( v.x, v.y ); }
template<class Archive> void serialize( Archive & archive, ci::vec3 &v ) { archive( v.x, v.y, v.z ); }
template<class Archive> void serialize( Archive & archive, ci::vec4 &v ) { archive( v.x, v.y, v.z, v.w ); }
template<class Archive> void serialize( Archive & archive, ci::ivec2 &v ) { archive( v.x, v.y ); }
template<class Archive> void serialize( Archive & archive, ci::ivec3 &v ) { archive( v.x, v.y, v.z ); }
template<class Archive> void serialize( Archive & archive, ci::ivec4 &v ) { archive( v.x, v.y, v.z, v.w ); }
template<class Archive> void serialize( Archive & archive, ci::uvec2 &v ) { archive( v.x, v.y ); }
template<class Archive> void serialize( Archive & archive, ci::uvec3 &v ) { archive( v.x, v.y, v.z ); }
template<class Archive> void serialize( Archive & archive, ci::uvec4 &v ) { archive( v.x, v.y, v.z, v.w ); }
template<class Archive> void serialize( Archive & archive, ci::dvec2 &v ) { archive( v.x, v.y ); }
template<class Archive> void serialize( Archive & archive, ci::dvec3 &v ) { archive( v.x, v.y, v.z ); }
template<class Archive> void serialize( Archive & archive, ci::dvec4 &v ) { archive( v.x, v.y, v.z, v.w ); }

// glm matrices serialization
template<class Archive> void serialize( Archive & archive, ci::mat2 &m ) { archive( m[0], m[1], m[2], m[3] ); }
template<class Archive> void serialize( Archive & archive, ci::dmat2 &m ) { archive( m[0], m[1], m[2], m[3] ); }
template<class Archive> void serialize( Archive & archive, ci::mat3 &m ) { archive( m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8] ); }
template<class Archive> void serialize( Archive & archive, ci::mat4 &m ) { archive( m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15] ); }
template<class Archive> void serialize( Archive & archive, ci::dmat4 &m ) { archive( m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15] ); }

// glm quaternions serialization
template<class Archive> void serialize( Archive & archive, ci::quat &q ) { archive( q.x, q.y, q.z, q.w ); }
template<class Archive> void serialize( Archive & archive, ci::dquat &q ) { archive( q.x, q.y, q.z, q.w ); }

// Area
template<class Archive>
void save( Archive & archive, const ci::Area &area )
{
	archive( area.getX1(), area.getY1(), area.getX2(), area.getY2() );
}
template<class Archive>
void load( Archive & archive, ci::Area &area )
{
	int32_t x1, y1, x2, y2;
	archive( x1, y1, x2, y2 );
	area.set( x1, y1, x2, y2 );
}

// Color
template<class Archive> void serialize( Archive & archive, ci::Color &c ) { archive( c.r, c.g, c.b ); }
template<class Archive> void serialize( Archive & archive, ci::Color8u &c ) { archive( c.r, c.g, c.b ); }
template<class Archive> void serialize( Archive & archive, ci::ColorA &c ) { archive( c.r, c.g, c.b, c.a ); }
template<class Archive> void serialize( Archive & archive, ci::ColorA8u &c ) { archive( c.r, c.g, c.b, c.a ); }

// Rect
template<class Archive>
void save( Archive & archive, const ci::Rectf &rect )
{
	archive( rect.getX1(), rect.getY1(), rect.getX2(), rect.getY2() );
}
template<class Archive>
void load( Archive & archive, ci::Rectf &rect )
{
	float x1, y1, x2, y2;
	archive( x1, y1, x2, y2 );
	rect.set( x1, y1, x2, y2 );
}
template<class Archive>
void save( Archive & archive, const ci::Rectd &rect )
{
	archive( rect.getX1(), rect.getY1(), rect.getX2(), rect.getY2() );
}
template<class Archive>
void load( Archive & archive, ci::Rectd &rect )
{
	double x1, y1, x2, y2;
	archive( x1, y1, x2, y2 );
	rect.set( x1, y1, x2, y2 );
}

// Camera
template<class Archive>
void save( Archive & archive, const ci::Camera &cam )
{
	archive( cam.getEyePoint(), cam.getOrientation(), cam.getFov(), cam.getAspectRatio(), cam.getNearClip(), cam.getFarClip(), cam.getPivotDistance() );
}
template<class Archive>
void load( Archive & archive, ci::Camera &cam )
{
	ci::vec3 eye;
	ci::quat orientation;
	float verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist;
	archive( eye, orientation, verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist );
	cam.setEyePoint( eye );
	cam.setOrientation( orientation );
	cam.setFov( verticalFovDegrees );
	cam.setAspectRatio( aspectRatio );
	cam.setNearClip( nearPlane );
	cam.setFarClip( farPlane );
	cam.setPivotDistance( pivotDist );
}
// CameraPersp
template<class Archive>
void save( Archive & archive, const ci::CameraPersp &cam )
{
	archive( cam.getEyePoint(), cam.getOrientation(), cam.getFov(), cam.getAspectRatio(), cam.getNearClip(), cam.getFarClip(), cam.getPivotDistance(), cam.getLensShift() );
}
template<class Archive>
void load( Archive & archive, ci::CameraPersp &cam )
{
	ci::vec3 eye;
	ci::vec2 lensShift;
	ci::quat orientation;
	float verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist;
	archive( eye, orientation, verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist, lensShift );
	cam.setEyePoint( eye );
	cam.setOrientation( orientation );
	cam.setPerspective( verticalFovDegrees, aspectRatio, nearPlane, farPlane );
	cam.setPivotDistance( pivotDist );
	cam.setLensShift( lensShift );
}
// CameraOrtho
template<class Archive>
void save( Archive & archive, const ci::CameraStereo &cam )
{
	archive( cam.getEyePoint(), cam.getOrientation(), cam.getFov(), cam.getAspectRatio(), cam.getNearClip(), cam.getFarClip(), cam.getPivotDistance(), cam.getLensShift(), cam.getConvergence(), cam.getEyeSeparation() );
}
template<class Archive>
void load( Archive & archive, ci::CameraStereo &cam )
{
	ci::vec3 eye;
	ci::vec2 lensShift;
	ci::quat orientation;
	float verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist, convergence, eyeSeparation;
	archive( eye, orientation, verticalFovDegrees, aspectRatio, nearPlane, farPlane, pivotDist, lensShift, convergence, eyeSeparation );
	cam.setEyePoint( eye );
	cam.setOrientation( orientation );
	cam.setPerspective( verticalFovDegrees, aspectRatio, nearPlane, farPlane );
	cam.setPivotDistance( pivotDist );
	cam.setLensShift( lensShift );
	cam.setConvergence( convergence );
	cam.setEyeSeparation( eyeSeparation );
}

// fs::path
template<class Archive>
void save( Archive & archive, const ci::fs::path &p )
{
	archive( p.string() );
}
template<class Archive>
void load( Archive & archive, ci::fs::path &p )
{
	std::string pstring;
	archive( pstring );
	p = pstring;
}

}
