#pragma once

#ifndef GLTOOLS_H
#define GLTOOLS_H

#include <iostream>
#include <string>
#include <exception>
#include <sstream>
#include <map>

#include <GL/glew.h>
#include <GL/glu.h>

namespace cg
{
/* Method 1: Function called after OpenGL command sequence. */
/*
 Error output to stream. To be called after OpenGL command sequence!
 */
inline void glErrorVerbose (std::ostream& os, unsigned linenum, bool stop=false) {
  GLenum errCode  = glGetError();
  if    (errCode != GL_NO_ERROR) {
     os << "[" << linenum << "] " << gluErrorString(errCode) << std::endl;
     if (stop) {
        exit(-1);
     }
  }
}

/* Method 2: GLException out of macro GLCODE. */
/* 
 GLException
 */
class GLException : public std::runtime_error
{
public:
	GLException(GLenum code, unsigned linenum) 
		: std::runtime_error(createString(code, linenum)),
		  m_code(code), m_line(linenum) {
	}

	static void throwGLException(unsigned linenum) {
		GLenum errCode = glGetError();
		if (errCode != GL_NO_ERROR) {
			throw new GLException(errCode, linenum); 
		}
	}
	static std::string createString (GLenum code, unsigned linenum) {
		if (code != GL_NO_ERROR) {
			std::ostringstream sstr;
			sstr << "[" << linenum << "] " << gluErrorString(code);
			return sstr.str();
		}
		return std::string();
	}

	GLenum   getErrorCode  () const {
		return m_code;
	}
	unsigned getLineNumber () const {
		return m_line;
	}

private:
	GLenum   m_code;
	unsigned m_line;
};
#define GLCODE(call) call;                 \
  cg::GLException::throwGLException(__LINE__)

  
};

#endif